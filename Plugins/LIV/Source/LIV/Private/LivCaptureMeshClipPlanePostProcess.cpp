// Copyright 2021 LIV Inc. - MIT License
#include "LivCaptureMeshClipPlanePostProcess.h"

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

ULivCaptureMeshClipPlanePostProcess::ULivCaptureMeshClipPlanePostProcess(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SceneCaptureComponent(nullptr)
	, CameraClipPlane(nullptr)
	, PostProcessedSceneRenderTarget(nullptr)
	, BackgroundDepthRenderTarget(nullptr)
	, ForegroundDepthRenderTarget(nullptr)
	, BackgroundOutputRenderTarget(nullptr)
	, ForegroundOutputRenderTarget(nullptr)
{
}

void ULivCaptureMeshClipPlanePostProcess::OnActivated()
{
	Super::OnActivated();

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);

	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(this, "LivSceneCaptureComponent");
	SceneCaptureComponent->RegisterComponentWithWorld(GetWorld());
	SceneCaptureComponent->AttachToComponent(GetAttachParent(), AttachmentRules);
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->bAlwaysPersistRenderingState = false;
	SceneCaptureComponent->SetRelativeLocation(FVector::ZeroVector);
	SceneCaptureComponent->SetRelativeRotation(FRotator::ZeroRotator);
	SceneCaptureComponent->SetRelativeScale3D(FVector::OneVector);

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;

	// Create the camera clip plane mesh
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

void ULivCaptureMeshClipPlanePostProcess::OnDeactivated()
{
	Super::OnDeactivated();

	// scene capture components deliberately not destroyed here as we 
	// would lose the parameters set by the developer

	CameraClipPlane->DestroyComponent();
	CameraClipPlane = nullptr;

	FloorClipPlane->DestroyComponent();
	FloorClipPlane = nullptr;
}

void ULivCaptureMeshClipPlanePostProcess::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background / Full Scene
	PostProcessedSceneRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"PostProcessedSceneRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Full scene depth
	BackgroundDepthRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundDepthRenderTarget",
		ETextureRenderTargetFormat::RTF_R16f
	);

	// Foreground depth (global clipping plane enabled)
	ForegroundDepthRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundDepthRenderTarget",
		ETextureRenderTargetFormat::RTF_R16f
	);

	// Output background 8bpc
	BackgroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Output foreground masked 8bpc
	ForegroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);
}

void ULivCaptureMeshClipPlanePostProcess::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->TextureTarget = nullptr;
	}

	TextureTarget = nullptr;

	if (PostProcessedSceneRenderTarget)
	{
		PostProcessedSceneRenderTarget->ReleaseResource();
		PostProcessedSceneRenderTarget = nullptr;
	}

	if (BackgroundDepthRenderTarget)
	{
		BackgroundDepthRenderTarget->ReleaseResource();
		BackgroundDepthRenderTarget = nullptr;
	}

	if (ForegroundDepthRenderTarget)
	{
		ForegroundDepthRenderTarget->ReleaseResource();
		ForegroundDepthRenderTarget = nullptr;
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

void ULivCaptureMeshClipPlanePostProcess::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	UWorld* World = GetWorld();

	// make sure clip plane is off for all captures
	SceneCaptureComponent->bEnableClipPlane = false;

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(SceneCaptureComponent);
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);
	Context.ApplyHideLists(SceneCaptureComponent);

	// Capture full scene with post processing (RGB)
	TextureTarget = PostProcessedSceneRenderTarget;
	CaptureSource = SCS_FinalColorLDR;
	PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	CaptureScene();

	// Capture full scene depth (Depth)
	SceneCaptureComponent->TextureTarget = BackgroundDepthRenderTarget;
	SceneCaptureComponent->CaptureSource = SCS_SceneDepth;
	SceneCaptureComponent->CaptureScene();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto CameraClipPlaneMatrix = InputFrame.CameraClipPlaneMatrix; // Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
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

	// Capture Foreground Depth
	SceneCaptureComponent->TextureTarget = ForegroundDepthRenderTarget;
	SceneCaptureComponent->CaptureSource = SCS_SceneDepth;
	SceneCaptureComponent->CaptureScene();

	// hide clip planes
	CameraClipPlane->SetHiddenInGame(true);
	FloorClipPlane->SetHiddenInGame(true);
	
	const ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	FTextureResource* BackgroundResource = PostProcessedSceneRenderTarget->Resource;
	FTextureResource* BackgroundDepthResource = BackgroundDepthRenderTarget->Resource;
	FTextureResource* ForegroundDepthResource = ForegroundDepthRenderTarget->Resource;
	FTextureResource* ForegroundOutputResource = ForegroundOutputRenderTarget->Resource;
	FTextureResource* BackgroundOutputResource = BackgroundOutputRenderTarget->Resource;

	ENQUEUE_RENDER_COMMAND(LivRDGCaptureMeshClipPlanePostProcess)(
		[FeatureLevel, BackgroundResource, BackgroundDepthResource, ForegroundDepthResource, ForegroundOutputResource, BackgroundOutputResource](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			{
				RDG_EVENT_SCOPE(GraphBuilder, "Liv Capture Mesh Clip Plane Post Process");

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
					RDG_EVENT_SCOPE(GraphBuilder, "Liv Foreground Segmentation PP and Copy");

					const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
					const TShaderMapRef<FLivRDGForegroundSegmentationPPAndCopyPS> PixelShader(GlobalShaderMap);

					FLivRDGForegroundSegmentationPPAndCopyPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGForegroundSegmentationPPAndCopyPS::FParameters>();
					Parameters->InputBackgroundTexture = BackgroundResource->TextureRHI;
					Parameters->InputBackgroundSampler = TStaticSamplerState<>::GetRHI();
					Parameters->InputBackgroundDepthTexture = BackgroundDepthResource->TextureRHI;
					Parameters->InputBackgroundDepthSampler = TStaticSamplerState<>::GetRHI();
					Parameters->InputForegroundDepthTexture = ForegroundDepthResource->TextureRHI;
					Parameters->InputForegroundDepthSampler = TStaticSamplerState<>::GetRHI();

					Parameters->RenderTargets[0] = FRenderTargetBinding(OutputForegroundTexture, ERenderTargetLoadAction::EClear, 0);
					Parameters->RenderTargets[1] = FRenderTargetBinding(OutputBackgroundTexture, ERenderTargetLoadAction::EClear, 0);

					const FScreenPassTextureViewport ScreenPassTextureViewport(OutputForegroundTexture);
					const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

					FLivRenderPass::AddLivPass(
						GraphBuilder,
						RDG_EVENT_NAME("Liv RDG Foreground Segmentation PP and Copy Pass"),
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
