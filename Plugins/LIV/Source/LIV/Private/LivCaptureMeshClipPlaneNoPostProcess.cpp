// Copyright 2021 LIV Inc. - MIT License
#include "LivCaptureMeshClipPlaneNoPostProcess.h"

#include "BasePassRendering.h"
#include "LivBlueprintFunctionLibrary.h"
#include "LivCaptureContext.h"
#include "LivConversions.h"
#include "LivClipPlane.h"
#include "LivCaptureContext.h"
#include "LivPluginSettings.h"
#include "LivRenderPass.h"
#include "LivShaders.h"
#include "PixelShaderUtils.h"
#include "SceneFilterRendering.h"
#include "ScreenPass.h"

ULivCaptureMeshClipPlaneNoPostProcess::ULivCaptureMeshClipPlaneNoPostProcess(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CameraClipPlane(nullptr)
	, BackgroundRenderTarget(nullptr)
	, ForegroundRenderTarget(nullptr)
	, BackgroundOutputRenderTarget(nullptr)
	, ForegroundOutputRenderTarget(nullptr)
{
}

void ULivCaptureMeshClipPlaneNoPostProcess::OnActivated()
{
	Super::OnActivated();

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = false;

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	
	// Create the clip plane mesh
	CameraClipPlane = NewObject<ULivClipPlane>(this, "LivCameraClipPlane");
	CameraClipPlane->RegisterComponentWithWorld(GetWorld());
	CameraClipPlane->AttachToComponent(this, AttachmentRules);
	CameraClipPlane->SetHiddenInGame(true);

	// Create the floor clip plane mesh
	FloorClipPlane = NewObject<ULivClipPlane>(this, "LivFloorClipPlane");
	FloorClipPlane->RegisterComponentWithWorld(GetWorld());
	FloorClipPlane->AttachToComponent(this, AttachmentRules);
	FloorClipPlane->SetHiddenInGame(true);
}

void ULivCaptureMeshClipPlaneNoPostProcess::OnDeactivated()
{
	Super::OnDeactivated();

	// scene capture component deliberately not destroyed here as we 
	// would lose the parameters set by the developer

	CameraClipPlane->DestroyComponent();
	CameraClipPlane = nullptr;

	FloorClipPlane->DestroyComponent();
	FloorClipPlane = nullptr;
}

void ULivCaptureMeshClipPlaneNoPostProcess::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background (RGB + Depth)
	BackgroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA16f
	);

	// Foreground (Depth)
	ForegroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundRenderTarget",
		ETextureRenderTargetFormat::RTF_R16f
	);

	// Background output (8bpc)
	BackgroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground masked
	ForegroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);
}

void ULivCaptureMeshClipPlaneNoPostProcess::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	TextureTarget = nullptr;

	if (BackgroundRenderTarget)
	{
		BackgroundRenderTarget->ReleaseResource();
		BackgroundRenderTarget = nullptr;
	}

	if (ForegroundRenderTarget)
	{
		ForegroundRenderTarget->ReleaseResource();
		ForegroundRenderTarget = nullptr;
	}

	if (BackgroundOutputRenderTarget)
	{
		BackgroundOutputRenderTarget->ReleaseResource();
		BackgroundOutputRenderTarget = nullptr;
	}

	if (ForegroundOutputRenderTarget)
	{
		ForegroundOutputRenderTarget->ReleaseResource();
		ForegroundOutputRenderTarget = nullptr;
	}
}

void ULivCaptureMeshClipPlaneNoPostProcess::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	UWorld* World = GetWorld();

	// make sure clip plane is off for all captures
	bEnableClipPlane = false;

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);

	// Capture Background
	TextureTarget = BackgroundRenderTarget;
	CaptureSource = SCS_SceneColorSceneDepth;
	CaptureScene();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();
	
	const auto CameraClipPlaneMatrix = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto CameraClipPlanePosition = VROriginTransform.TransformPosition(CameraClipPlaneMatrix.TransformPosition(FVector::ZeroVector));
	const auto CameraClipPlaneForward = VROriginTransform.TransformVector(CameraClipPlaneMatrix.TransformVector(FVector::ForwardVector));
	const auto CameraClipPlaneRotation = CameraClipPlaneForward.Rotation();
	const auto CameraClipPlaneScale = VROriginTransform.GetScale3D() * CameraClipPlaneMatrix.GetScaleVector();

	// Transform camera clip plane mesh
	CameraClipPlane->SetWorldLocationAndRotation(CameraClipPlanePosition, CameraClipPlaneRotation);
	CameraClipPlane->SetWorldScale3D(CameraClipPlaneScale);
	CameraClipPlane->SetHiddenInGame(false);

	if (InputFrame.bFloorClipPlaneEnabled)
	{
		// Transform floor clip plane mesh
		const auto FloorClipPlaneMatrix = InputFrame.FloorClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->GroundPlane.transform);
		const auto FloorClipPlanePosition = VROriginTransform.TransformPosition(FloorClipPlaneMatrix.TransformPosition(FVector::ZeroVector));
		const auto FloorClipPlaneForward = VROriginTransform.TransformVector(FloorClipPlaneMatrix.TransformVector(FVector::ForwardVector));
		const auto FloorClipPlaneRotation = FloorClipPlaneForward.Rotation();
		const auto FloorClipPlaneScale = VROriginTransform.GetScale3D() * FloorClipPlaneMatrix.GetScaleVector();


		FloorClipPlane->SetWorldLocationAndRotation(FloorClipPlanePosition, FloorClipPlaneRotation);
		FloorClipPlane->SetWorldScale3D(FloorClipPlaneScale);
		FloorClipPlane->SetHiddenInGame(false);
	}

	// Capture Foreground
	TextureTarget = ForegroundRenderTarget;
	CaptureSource = SCS_SceneDepth;
	CaptureScene();
	
	// hide clip planes
	CameraClipPlane->SetHiddenInGame(true);
	FloorClipPlane->SetHiddenInGame(true);

	// @TODO: combine the following two shader calls into one if possible

	const ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	FTextureResource* ForegroundResource = ForegroundRenderTarget->Resource;
	FTextureResource* BackgroundResource = BackgroundRenderTarget->Resource;
	FTextureResource* ForegroundOutputResource = ForegroundOutputRenderTarget->Resource;
	FTextureResource* BackgroundOutputResource = BackgroundOutputRenderTarget->Resource;

	ENQUEUE_RENDER_COMMAND(LivRDGCaptureMeshClipPlaneNoPostProcess)(
		[FeatureLevel, ForegroundResource, BackgroundResource, ForegroundOutputResource, BackgroundOutputResource](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			{
				RDG_EVENT_SCOPE(GraphBuilder, "Liv Capture Mesh Clip Plane No Post Process");

				const auto GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

				const FRDGTextureRef OutputForegroundTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
					GraphBuilder,
					ForegroundOutputResource,
					TEXT("Foreground Output")
				);

				const FRDGTextureRef OutputBackgroundTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
					GraphBuilder,
					BackgroundOutputResource,
					TEXT("Background Output")
				);

				{
					RDG_EVENT_SCOPE(GraphBuilder, "Liv Foreground Segmentation and Copy");

					const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
					const TShaderMapRef<FLivRDGForegroundSegmentationAndCopyPS> PixelShader(GlobalShaderMap);

					FLivRDGForegroundSegmentationAndCopyPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGForegroundSegmentationAndCopyPS::FParameters>();
					Parameters->InputForegroundTexture = ForegroundResource->TextureRHI;
					Parameters->InputForegroundSampler = TStaticSamplerState<>::GetRHI();
					Parameters->InputBackgroundTexture = BackgroundResource->TextureRHI;
					Parameters->InputBackgroundSampler = TStaticSamplerState<>::GetRHI();

					Parameters->RenderTargets[0] = FRenderTargetBinding(OutputForegroundTexture, ERenderTargetLoadAction::EClear,0);
					Parameters->RenderTargets[1] = FRenderTargetBinding(OutputBackgroundTexture, ERenderTargetLoadAction::EClear, 0);

					const FScreenPassTextureViewport ScreenPassTextureViewport(OutputForegroundTexture);
					const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

					FLivRenderPass::AddLivPass(
						GraphBuilder,
						RDG_EVENT_NAME("Liv RDG Foreground Segmentation and Copy Pass"),
						ScreenPassTextureViewport,
						PipelineState,
						PixelShader,
						Parameters
					);
				}

				{
					RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

					FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();
					Parameters->ForegroundTexture = OutputForegroundTexture;
					Parameters->BackgroundTexture = OutputBackgroundTexture;

					FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
				}
			}

			GraphBuilder.Execute();
		});

#endif
}
