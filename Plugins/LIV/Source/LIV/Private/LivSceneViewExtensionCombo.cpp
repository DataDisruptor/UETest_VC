// Copyright 2021 LIV Inc. - MIT License


#include "LivSceneViewExtensionCombo.h"

#include "ClipPlaneMeshPassProcessor.h"
#include "LivConversions.h"
#include "LivCustomClipPlane.h"
#include "LivPluginSettings.h"
#include "LivRenderPass.h"
#include "LivShaders.h"
#include "MeshPassProcessor.inl"
#include "PostProcessing.h"
#include "PostProcessMaterial.h"
#include "SceneView.h"

static const TCHAR* GLivForegroundName = TEXT("LivForeground");
static const TCHAR* GLivBackgroundName = TEXT("LivBackground");

FLivSceneViewExtensionCombo::FLivSceneViewExtensionCombo(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient)
	: FLivSceneViewExtensionBase(AutoRegister, AssociatedViewportClient)
{
}

FLivSceneViewExtensionCombo::~FLivSceneViewExtensionCombo()
{
}

void FLivSceneViewExtensionCombo::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	FLivSceneViewExtensionBase::SetupViewFamily(InViewFamily);

	const ULivPluginSettings* PluginSettings = GetDefault<ULivPluginSettings>();
	bTransparency = PluginSettings->bTransparency;
	bBackgroundOnly = PluginSettings->bBackgroundOnly;
	checkf(!(bTransparency && bBackgroundOnly), TEXT("Invalid settings, cannot have background only and foreground transparency enabled at the same time."));
}


void FLivSceneViewExtensionCombo::SubscribeToPostProcessingPass(
	EPostProcessingPass Pass,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks,
	bool bIsPassEnabled)
{
	if(Pass == EPostProcessingPass::FXAA)
	{
		checkf(bIsPassEnabled, TEXT("The LIV Scene View Extension relies on the FXAA pass being enabled."));

		if(bBackgroundOnly)
		{
			InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionCombo::PostProcessPassAfterFXAABackgroundOnly_RenderThread));
		}
		else if(bTransparency)
		{
			InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionCombo::PostProcessPassAfterFXAATransparency_RenderThread));
		}
		else
		{
			InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionCombo::PostProcessPassAfterFXAA_RenderThread));
		}
	}
}

bool FLivSceneViewExtensionCombo::IsReadyForSubmit() const
{
	return ForegroundRenderTarget2D.IsValid() && BackgroundRenderTarget2D.IsValid()
		&& ForegroundFrameNumber == BackgroundFrameNumber;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
FScreenPassTexture FLivSceneViewExtensionCombo::PostProcessPassAfterFXAA_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
#if PLATFORM_WINDOWS
	//check(View.bIsSceneCapture);

	if (IsBackgroundCapture(*View.Family))
	{
		BackgroundFrameNumber = View.Family->FrameNumber;
	}
	else
	{
		const FRDGTextureDesc SceneColorDesc = FRDGTextureDesc::Create2D(View.Family->RenderTarget->GetSizeXY(), EPixelFormat::PF_B8G8R8A8, FClearValueBinding::Black, TexCreate_RenderTargetable);
		const FRDGTextureRef LivForegroundTexture = GraphBuilder.CreateTexture(SceneColorDesc, GLivForegroundName, ERDGTextureFlags::None);

		if (IsForegroundCapture(*View.Family))
		{
			RDG_EVENT_SCOPE(GraphBuilder, "Liv Copy Full Scene Color");

			const auto GlobalShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());

			const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
			const TShaderMapRef<FLivRDGCopyFullSceneColorPS> PixelShader(GlobalShaderMap);

			FLivRDGCopyFullSceneColorPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGCopyFullSceneColorPS::FParameters>();
			Parameters->RenderTargets[0] = FRenderTargetBinding(LivForegroundTexture, ERenderTargetLoadAction::EClear);
			Parameters->View = View.ViewUniformBuffer;
			Parameters->SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All); // @TODO: optimise setup mode

			const FScreenPassTextureViewport ScreenPassTextureViewport(LivForegroundTexture);
			const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

			FLivRenderPass::AddLivPass(
				GraphBuilder,
				RDG_EVENT_NAME("Liv RDG Copy Full Scene Color Pass"),
				ScreenPassTextureViewport,
				PipelineState,
				PixelShader,
				Parameters
			);

			ForegroundFrameNumber = View.Family->FrameNumber;
		}

		if (IsReadyForSubmit())
		{
			RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

			check(BackgroundRenderTarget2D.IsValid());
			check(BackgroundRenderTarget2D->Resource);
			check(BackgroundRenderTarget2D->GetRenderTargetResource());
			// @TODO: remove this
			check(BackgroundRenderTarget2D->GetFormat() == PF_B8G8R8A8);


			const FTextureResource* BackgroundResource = static_cast<FTextureResource*>(BackgroundRenderTarget2D->GetRenderTargetResource());

			FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();
			Parameters->ForegroundTexture = LivForegroundTexture;
			Parameters->BackgroundTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
				GraphBuilder,
				BackgroundResource,
				GLivBackgroundName
			);

			FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
		}
	}
#endif

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}

FScreenPassTexture FLivSceneViewExtensionCombo::PostProcessPassAfterFXAABackgroundOnly_RenderThread(
	FRDGBuilder& GraphBuilder, 
	const FSceneView& View, 
	const FPostProcessMaterialInputs& InOutInputs)
{
#if PLATFORM_WINDOWS
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

		const FRDGTextureRef ForegroundTexture = RegisterExternalOrPassthroughTexture(&GraphBuilder, GSystemTextures.BlackDummy);
		FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();

		Parameters->ForegroundTexture = ForegroundTexture;
		Parameters->BackgroundTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
			GraphBuilder,
			View.Family->RenderTarget,
			TEXT("LivBackground")
		);

		FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
	}
#endif

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}

FScreenPassTexture FLivSceneViewExtensionCombo::PostProcessPassAfterFXAATransparency_RenderThread(
	FRDGBuilder& GraphBuilder, 
	const FSceneView& View, 
	const FPostProcessMaterialInputs& InOutInputs)
{

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}
