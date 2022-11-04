// Copyright 2021 LIV Inc. - MIT License
#include "LivCaptureGlobalClipPlanePostProcess.h"

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

ULivCaptureGlobalClipPlanePostProcess::ULivCaptureGlobalClipPlanePostProcess(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SceneCaptureComponent(nullptr)
	, PostProcessedBackgroundRenderTarget(nullptr)
	, PostProcessedForegroundRenderTarget(nullptr)
	, ForegroundInverseOpacityRenderTarget(nullptr)
{
}

void ULivCaptureGlobalClipPlanePostProcess::OnActivated()
{
	Super::OnActivated();

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);

	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(this, "LivSceneCaptureComponent");
	SceneCaptureComponent->RegisterComponentWithWorld(GetWorld());
	SceneCaptureComponent->AttachToComponent(GetAttachParent(), AttachmentRules);
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->bAlwaysPersistRenderingState = true;
	SceneCaptureComponent->SetRelativeLocation(FVector::ZeroVector);
	SceneCaptureComponent->SetRelativeRotation(FRotator::ZeroRotator);
	SceneCaptureComponent->SetRelativeScale3D(FVector::OneVector);

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;
}

void ULivCaptureGlobalClipPlanePostProcess::OnDeactivated()
{
	Super::OnDeactivated();

	// scene capture component deliberately not destroyed here as we 
	// would lose the parameters set by the developer
}

void ULivCaptureGlobalClipPlanePostProcess::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background / Full Scene
	PostProcessedBackgroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"PostProcessedSceneRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Full scene depth
	PostProcessedForegroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"FullSceneDepthRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground depth (global clipping plane enabled)
	ForegroundInverseOpacityRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundSceneDepthRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Output foreground masked 8bpc
	ForegroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundMaskedRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);
}

void ULivCaptureGlobalClipPlanePostProcess::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->TextureTarget = nullptr;
	}

	if (PostProcessedBackgroundRenderTarget)
	{
		PostProcessedBackgroundRenderTarget->ReleaseResource();
		PostProcessedBackgroundRenderTarget = nullptr;
	}

	if (PostProcessedForegroundRenderTarget)
	{
		PostProcessedForegroundRenderTarget->ReleaseResource();
		PostProcessedForegroundRenderTarget = nullptr;
	}

	if (ForegroundInverseOpacityRenderTarget)
	{
		ForegroundInverseOpacityRenderTarget->ReleaseResource();
		ForegroundInverseOpacityRenderTarget = nullptr;
	}

	if (ForegroundOutputRenderTarget)
	{
		ForegroundOutputRenderTarget->ReleaseResource();
		ForegroundOutputRenderTarget = nullptr;
	}
}

void ULivCaptureGlobalClipPlanePostProcess::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	UWorld* World = GetWorld();

	UpdateLivInputFrame(this);
	
	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(SceneCaptureComponent);
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);
	Context.ApplyHideLists(SceneCaptureComponent);

	// Capture full scene with post processing (RGB)
	TextureTarget = PostProcessedBackgroundRenderTarget;
	CaptureSource = SCS_FinalColorLDR;
	bEnableClipPlane = false;
	PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	CaptureScene();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto ClipPlaneTransform = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto ClipPlanePosition = VROriginTransform.TransformPosition(ClipPlaneTransform.TransformPosition(FVector::ZeroVector));
	const auto ClipPlaneForward = VROriginTransform.TransformVector(ClipPlaneTransform.TransformVector(FVector::ForwardVector));

	// Capture foreground scene with post processing (RGB)
	ClipPlaneBase = ClipPlanePosition;
	ClipPlaneNormal = ClipPlaneForward;
	bEnableClipPlane = true;
	TextureTarget = PostProcessedForegroundRenderTarget;
	CaptureSource = SCS_FinalColorLDR;
	CaptureScene();

	// Capture foreground scene, no post processing for it's alpha channel 
	SceneCaptureComponent->ClipPlaneBase = ClipPlanePosition;
	SceneCaptureComponent->ClipPlaneNormal = ClipPlaneForward;
	SceneCaptureComponent->bEnableClipPlane = true;
	SceneCaptureComponent->TextureTarget = ForegroundInverseOpacityRenderTarget;
	SceneCaptureComponent->CaptureSource = SCS_SceneColorHDR; // deliberately not LDR
	SceneCaptureComponent->CaptureScene();

	const ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	// @TODO:	we only need RTs for captured textures, intermediate/processed textures
	//			can be allocate in RDG (though it will make the debug window less useful)
	FTextureResource* InputColorResource = PostProcessedForegroundRenderTarget->Resource;
	FTextureResource* InputAlphaResource = ForegroundInverseOpacityRenderTarget->Resource;
	FTextureResource* OutputResource = ForegroundOutputRenderTarget->Resource;
	FTextureResource* BackgroundResource = PostProcessedBackgroundRenderTarget->Resource;

	ENQUEUE_RENDER_COMMAND(LivRDGCaptureGlobalClipPlanePostProcess)(
		[FeatureLevel, InputColorResource, InputAlphaResource, OutputResource, BackgroundResource](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			{
				RDG_EVENT_SCOPE(GraphBuilder, "Liv Capture Global Clip Plane PostProcess");

				const auto GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

				const FRDGTextureRef OutputTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
					GraphBuilder,
					OutputResource,
					TEXT("LivCombinedAlpha"));

				{
					RDG_EVENT_SCOPE(GraphBuilder, "Liv RDG Combine Alpha");

					const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
					const TShaderMapRef<FLivRDGCombineAlphaPS> PixelShader(GlobalShaderMap);

					FLivRDGCombineAlphaPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGCombineAlphaPS::FParameters>();
					Parameters->InputColorTexture = InputColorResource->TextureRHI;
					Parameters->InputColorSampler = TStaticSamplerState<>::GetRHI();
					Parameters->InputAlphaTexture = InputAlphaResource->TextureRHI;
					Parameters->InputAlphaSampler = TStaticSamplerState<>::GetRHI();
					Parameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::EClear);

					const FScreenPassTextureViewport ScreenPassTextureViewport(OutputTexture);
					const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

					FLivRenderPass::AddLivPass(
						GraphBuilder,
						RDG_EVENT_NAME("Liv RDG Combine Alpha Pass"),
						ScreenPassTextureViewport,
						PipelineState,
						PixelShader,
						Parameters
					);
				}

				{
					RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

					FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();
					Parameters->ForegroundTexture = OutputTexture;
					Parameters->BackgroundTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
						GraphBuilder,
						BackgroundResource,
						TEXT("LivBackground")
					);

					FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
				}
			}

			GraphBuilder.Execute();
		});

#endif
}
