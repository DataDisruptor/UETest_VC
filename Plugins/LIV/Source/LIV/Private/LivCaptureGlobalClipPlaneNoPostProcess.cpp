// Copyright 2021 LIV Inc. - MIT License
#include "LivCaptureGlobalClipPlaneNoPostProcess.h"
#include "LivBlueprintFunctionLibrary.h"
#include "LivCaptureContext.h"
#include "LivConversions.h"
#include "LivClipPlane.h"
#include "LivCaptureContext.h"
#include "LivRenderPass.h"
#include "LivShaders.h"
#include "PixelShaderUtils.h"
#include "RenderGraphBuilder.h"
#include "SceneFilterRendering.h"
#include "ScreenPass.h"

ULivCaptureGlobalClipPlaneNoPostProcess::ULivCaptureGlobalClipPlaneNoPostProcess(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BackgroundRenderTarget(nullptr)
	, ForegroundRenderTarget(nullptr)
	, ForegroundMaskedRenderTarget(nullptr)
{
}

void ULivCaptureGlobalClipPlaneNoPostProcess::OnActivated()
{
	Super::OnActivated();

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;
}

void ULivCaptureGlobalClipPlaneNoPostProcess::OnDeactivated()
{
	Super::OnDeactivated();

	// scene capture component deliberately not destroyed here as we 
	// would lose the parameters set by the developer
}

void ULivCaptureGlobalClipPlaneNoPostProcess::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background
	BackgroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground
	ForegroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground masked
	ForegroundMaskedRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundMaskedRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);
}

void ULivCaptureGlobalClipPlaneNoPostProcess::ReleaseRenderTargets()
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

	if (ForegroundMaskedRenderTarget)
	{
		ForegroundMaskedRenderTarget->ReleaseResource();
		ForegroundMaskedRenderTarget = nullptr;
	}
}

void ULivCaptureGlobalClipPlaneNoPostProcess::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	UWorld* World = GetWorld();

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);

	// Capture Background
	TextureTarget = BackgroundRenderTarget;
	CaptureSource = SCS_SceneColorHDRNoAlpha;
	bEnableClipPlane = false;
	CaptureScene();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto ClipPlaneTransform = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto ClipPlanePosition = VROriginTransform.TransformPosition(ClipPlaneTransform.TransformPosition(FVector::ZeroVector));
	const auto ClipPlaneForward = VROriginTransform.TransformVector(ClipPlaneTransform.TransformVector(FVector::ForwardVector));

	FTextureResource* InputResource = ForegroundRenderTarget->Resource;
	FTextureResource* OutputResource = ForegroundMaskedRenderTarget->Resource;

	// Capture Foreground
	ClipPlaneBase = ClipPlanePosition;
	ClipPlaneNormal = ClipPlaneForward;
	TextureTarget = ForegroundRenderTarget;
	CaptureSource = SCS_SceneColorHDR;
	bEnableClipPlane = true;
	CaptureScene();

	const ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	// @TODO:	we only need RTs for captured textures, intermediate/processed textures
	//			can be allocate in RDG (though it will make the debug window less useful)

	FTextureResource* BackgroundResource = BackgroundRenderTarget->Resource;
	
	ENQUEUE_RENDER_COMMAND(LivRDGCaptureGlobalClipPlaneNoPostProcess)(
	[FeatureLevel, InputResource, OutputResource, BackgroundResource](FRHICommandListImmediate& RHICmdList)
	{
		FRDGBuilder GraphBuilder(RHICmdList);

		{
			RDG_EVENT_SCOPE(GraphBuilder, "Liv Capture Global Clip Plane No PostProcess");

			const auto GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

			const FRDGTextureRef OutputTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
				GraphBuilder,
				OutputResource,
				TEXT("LivInvertedAlpha")
			);

			{
				RDG_EVENT_SCOPE(GraphBuilder, "Liv RDG Invert Alpha");

				const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
				const TShaderMapRef<FLivRDGInvertAlphaPS> PixelShader(GlobalShaderMap);

				FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

				FLivRDGInvertAlphaPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGInvertAlphaPS::FParameters>();
				Parameters->InputTexture = InputResource->TextureRHI;
				Parameters->InputSampler = BilinearClampSampler;
				Parameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::EClear);

				const FScreenPassTextureViewport ScreenPassTextureViewport(OutputTexture);
				const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

				FLivRenderPass::AddLivPass(
					GraphBuilder,
					RDG_EVENT_NAME("Liv RDG Invert Alpha Pass"),
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
