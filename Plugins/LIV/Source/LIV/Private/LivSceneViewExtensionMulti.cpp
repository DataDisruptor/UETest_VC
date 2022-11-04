// Copyright 2021 LIV Inc. - MIT License


#include "LivSceneViewExtensionMulti.h"

#include "ClipPlaneMeshPassProcessor.h"
#include "EngineModule.h"
#include "LivConversions.h"
#include "LivCustomClipPlane.h"
#include "LivPluginSettings.h"
#include "LivRenderPass.h"
#include "LivShaders.h"
#include "MeshPassProcessor.inl"
#include "PostProcessing.h"
#include "PostProcessMaterial.h"
#include "SceneView.h"

static const TCHAR* GForegroundName = TEXT("LivForeground");
static const TCHAR* GBackgroundName = TEXT("LivBackground");

#if WITH_EYE_ADAPTATION_CALLBACK
TAutoConsoleVariable<bool> CVarOverwriteEyeAdaption(TEXT("Liv.Debug.OverwriteEyeAdaption"),
	true,
	TEXT("Whether we overwrite the eye adaption texture for foreground (from background) or not.")
);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////

FLivSceneViewExtensionMulti::FLivSceneViewExtensionMulti(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient)
	: FLivSceneViewExtensionBase(AutoRegister, AssociatedViewportClient)
{
}

FLivSceneViewExtensionMulti::~FLivSceneViewExtensionMulti()
{
	if(PostOpaqueHandle.IsValid())
	{
		GetRendererModule().RemovePostOpaqueRenderDelegate(PostOpaqueHandle);
	}
}

void FLivSceneViewExtensionMulti::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	FLivSceneViewExtensionBase::SetupView(InViewFamily, InView);

	// @TODO:
	// 1. Use FRendererModule::RegisterPostOpaqueRenderDelegate to bind a post opaque extension
	// 2. From this scene view extension detect the foreground view
	// 3. Use the view for comparison with FPostOpaqueRenderParameters::Uid [ which is (void*)(&View) ]
	// 4. In the opaque callback render the clip plane (black with depth test) remove any sky/debug text from the render
	// 5. ???
	// 6. Profit: PostRenderOpaque is just before transparency so we'll get the right sky reflections but without the sky in the back of the render when clipping with global clip plane

	if (!PostOpaqueHandle.IsValid())
	{
		PostOpaqueHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(FOnPostOpaqueRender::FDelegate::CreateRaw(this, &FLivSceneViewExtensionMulti::OnPostOpaque));
	}

	//static_cast<FViewInfo&>(InView).PreExposure = GetDefault<ULivPluginSettings>()->PreExposure;

	/*
	// @NOTE/@TODO: this could affect other scene capture components in the scene if they are using the global clipping plane
	//				I want to use IsForegroundCapture() but that's render thread only.
	//				If there's a way to check which capture this then use that instead of this
	if (!InView.GlobalClippingPlane.Equals(FPlane(0, 0, 0, 0)))
	{
		// @NOTE: basically, if using global clip plane, pretend there's no sky material so it doesn't get rendered
		if (ensure(InView.bIsViewInfo))
		{
			// static_cast<FViewInfo&>(InView).bSceneHasSkyMaterial = 0u;
			InViewFamily.EngineShowFlags.Atmosphere = 1u;
		}
	}
	*/

	/*
	if (!InView.GlobalClippingPlane.Equals(FPlane(0, 0, 0, 0)))
	{
		FSkyAtmosphereRenderSceneInfo* SkyAtmosphereRenderSceneInfo = InViewFamily.Scene->GetSkyAtmosphereSceneInfo();
		if(SkyAtmosphereRenderSceneInfo != nullptr)
		{
			InViewFamily.EngineShowFlags.Atmosphere = 0u;
		}
	}*/

	//InViewFamily.ExposureSettings.bFixed = true;

	if (!InView.GlobalClippingPlane.Equals(FPlane(0, 0, 0, 0)))
	{
		ForegroundViewUid = (void*)(&InView);
	}
}

void FLivSceneViewExtensionMulti::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	FLivSceneViewExtensionBase::PreRenderView_RenderThread(RHICmdList, InView);

	//static_cast<FViewInfo&>(InView).PreExposure = GetDefault<ULivPluginSettings>()->PreExposure;
}

#if WITH_EYE_ADAPTATION_CALLBACK
/**
 * If the define is set and this is compiled with an engine that has the changes in the PR https://github.com/EpicGames/UnrealEngine/pull/9009
 * then this code will copy the backgrounds eye adaptation texture and re-use it for the foreground which results in the correct luminance.
 * When this is not compiled the foreground is generally darker than the background. Another work around is to completely disable eye adaptation by:
 * - Setting Exposure Metering Mode to Manual
 * - Setting Exposure Compensation to 0
 * - Disable 'Apply Physical Camera Exposure'
 */
void FLivSceneViewExtensionMulti::PostEyeAdaptation_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, FRDGTextureRef EyeAdaptationTexture)
{
	FLivSceneViewExtensionBase::PostEyeAdaptation_RenderThread(GraphBuilder, View, EyeAdaptationTexture);

	if(!CVarOverwriteEyeAdaption.GetValueOnAnyThread())
	{
		return;
	}

	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
	
	if(!EyeAdaptionRenderTarget2D.IsValid())
	{
		return;
	}
	const FTextureResource* LivEyeAdaptionResource = static_cast<FTextureResource*>(EyeAdaptionRenderTarget2D->GetRenderTargetResource());
	if(!LivEyeAdaptionResource)
	{
		return;
	}

	const FRDGTextureRef LivEyeAdaptationTexture = FLivRenderPass::CreateRDGTextureFromRenderTarget(
		GraphBuilder,
		LivEyeAdaptionResource,
		GEyeAdaptionName
	);

	if(IsBackgroundCapture(*View.Family))
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Copy Eye Adaptation (Background)");

		AddDrawTexturePass(
			GraphBuilder,
			ViewInfo,
			EyeAdaptationTexture,
			LivEyeAdaptationTexture,
			FIntPoint::ZeroValue,
			FIntPoint::ZeroValue,
			EyeAdaptationTexture->Desc.Extent);
		
	}
	else if(IsForegroundCapture(*View.Family))
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Overwrite Eye Adaptation Back (Foreground)");

		AddDrawTexturePass(
			GraphBuilder,
			ViewInfo,
			LivEyeAdaptationTexture,
			EyeAdaptationTexture,
			FIntPoint::ZeroValue,
			FIntPoint::ZeroValue,
			EyeAdaptationTexture->Desc.Extent);
	}

}
#endif

void FLivSceneViewExtensionMulti::SubscribeToPostProcessingPass(
	EPostProcessingPass Pass,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks,
	bool bIsPassEnabled)
{
	const ELivSceneViewExtensionCaptureStage CaptureStage = GetDefault<ULivPluginSettings>()->SceneViewExtensionCaptureStage;
	if (Pass == EPostProcessingPass::FXAA && CaptureStage == ELivSceneViewExtensionCaptureStage::AfterFXAA && bIsPassEnabled)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionMulti::PostProcessPassAfterFXAA_RenderThread));
	}
}

bool FLivSceneViewExtensionMulti::IsReadyForSubmit() const
{
	return ForegroundRenderTarget2D.IsValid() && BackgroundRenderTarget2D.IsValid()
		&& ForegroundFrameNumber == BackgroundFrameNumber;
}

void FLivSceneViewExtensionMulti::OnPostOpaque(FPostOpaqueRenderParameters& Parameters) const
{
	// @NOTE: this isn't reliable
	/*if (ForegroundViewUid == nullptr) return;
	if (Parameters.Uid != ForegroundViewUid) return;*/

	/**
	 * For why?
	 * To get rid of the sky.
	 * We draw a black plane with our clip plane transform to get rid of the sky atmposhere that is rendered behind the clipping plane.
	 */

	FSceneView& View = *static_cast<FSceneView*>(Parameters.Uid);

	if (View.GlobalClippingPlane.Equals(FPlane(0, 0, 0, 0)))
	{
		return;
	}

	const FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(*Parameters.RHICmdList);
	const FRHIRenderPassInfo RPInfo(SceneContext.GetSceneColorTexture(), ERenderTargetActions::Load_Store, Parameters.DepthTexture, EDepthStencilTargetActions::LoadDepthStencil_StoreDepthStencil);

	Parameters.RHICmdList->BeginRenderPass(RPInfo, TEXT("LivOnPostOpaque"));

	TArray<const FPrimitiveSceneProxy*> ClipPlaneSceneProxies = GetClipPlaneSceneProxies(ClipPlanes);

	FMeshPassProcessorRenderState DrawRenderState(View);

	Parameters.RHICmdList->SetViewport(View.UnscaledViewRect.Min.X, View.UnscaledViewRect.Min.Y, 0.0f, View.UnscaledViewRect.Max.X, View.UnscaledViewRect.Max.Y, 1.0f);

	DrawDynamicMeshPass(View, *Parameters.RHICmdList,
		[&View, &DrawRenderState, ClipPlaneSceneProxies](FDynamicPassMeshDrawListContext* DynamicMeshPassContext)
		{
			FLivClipPlaneBasePassMeshProcessor PassMeshProcessor(
				View.Family->Scene->GetRenderScene(),
				View.GetFeatureLevel(),
				&View,
				DrawRenderState,
				DynamicMeshPassContext,
				true);

			constexpr uint64 DefaultBatchElementMask = ~0ull;

			for (const auto* SceneProxy : ClipPlaneSceneProxies)
			{
				TArray<FMeshBatch> MeshBatches;
				SceneProxy->GetMeshDescription(0, MeshBatches);

				for (auto& MeshBatch : MeshBatches)
				{
					PassMeshProcessor.AddMeshBatch(
						MeshBatch,
						DefaultBatchElementMask,
						SceneProxy
					);
				}
			}
		});

	Parameters.RHICmdList->EndRenderPass();
}


FScreenPassTexture FLivSceneViewExtensionMulti::PostProcessPassAfterFXAA_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
	// @todo: this doesn't trigger in 4.26 but does in 4.27? Won't make much of a difference buts odd
	// check(View.bIsSceneCapture);

#if PLATFORM_WINDOWS
	
	if (IsBackgroundCapture(*View.Family))
	{
		BackgroundFrameNumber = View.Family->FrameNumber;
		
		{
			RDG_EVENT_SCOPE(GraphBuilder, "Liv Copy Full Scene Color (BG)");

			const FScreenPassTexture& SceneColor = InOutInputs.Textures[static_cast<uint32>(EPostProcessMaterialInput::SceneColor)];
			const FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);

			const auto GlobalShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());

			const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
			const TShaderMapRef<FLivRDGCopyFullSceneColorPS> PixelShader(GlobalShaderMap);

			const FTextureResource* BackgroundOutputResource = static_cast<FTextureResource*>(BackgroundOutputRenderTarget->GetRenderTargetResource());
			const FRDGTextureRef BackgroundOutput = FLivRenderPass::CreateRDGTextureFromRenderTarget(
				GraphBuilder,
				BackgroundOutputResource,
				GBackgroundName
			);

			FLivRDGCopyFullSceneColorPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGCopyFullSceneColorPS::FParameters>();
			Parameters->InputTexture = SceneColorRenderTarget.Texture;
			Parameters->InputTextureSampler = TStaticSamplerState<>::GetRHI();
			//Parameters->EyeAdaptationTexture = EyeAdaption;
			Parameters->RenderTargets[0] = FRenderTargetBinding(BackgroundOutput, ERenderTargetLoadAction::EClear);
			Parameters->View = View.ViewUniformBuffer;
			Parameters->SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All); // @TODO: optimise setup mode

			const FScreenPassTextureViewport ScreenPassTextureViewport(BackgroundOutput);
			const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

			FLivRenderPass::AddLivPass(
				GraphBuilder,
				RDG_EVENT_NAME("Liv RDG Copy Full Scene Color Pass"),
				ScreenPassTextureViewport,
				PipelineState,
				PixelShader,
				Parameters
			);
		}

		ensure(InOutInputs.OverrideOutput.IsValid());
	}
	else if (IsForegroundCapture(*View.Family))
	{
		ensure(InOutInputs.OverrideOutput.IsValid());

		const FRDGTextureDesc SceneColorDesc = FRDGTextureDesc::Create2D(View.Family->RenderTarget->GetSizeXY(), EPixelFormat::PF_B8G8R8A8, FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_SRGB);
		const FRDGTextureRef LivForegroundTexture = GraphBuilder.CreateTexture(SceneColorDesc, GForegroundName, ERDGTextureFlags::None);
		
		{
			RDG_EVENT_SCOPE(GraphBuilder, "Liv Copy Full Scene Color (FG)");

			const FScreenPassTexture& SceneColor = InOutInputs.Textures[static_cast<uint32>(EPostProcessMaterialInput::SceneColor)];
			const FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);

			const auto GlobalShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());

			const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
			const TShaderMapRef<FLivRDGCopyFullSceneColorPS> PixelShader(GlobalShaderMap);
			
			FLivRDGCopyFullSceneColorPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGCopyFullSceneColorPS::FParameters>();
			Parameters->InputTexture = SceneColorRenderTarget.Texture;
			Parameters->InputTextureSampler = TStaticSamplerState<>::GetRHI();
			//Parameters->EyeAdaptationTexture = EyeAdaption;
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
		}

		ForegroundFrameNumber = View.Family->FrameNumber;

		if (IsReadyForSubmit())
		{
			// we've set priorities so that we should be ready in the foreground pass, not background
			checkSlow(!IsBackgroundCapture(*View.Family));
			checkSlow(IsForegroundCapture(*View.Family))

			{
				RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

				ensure(BackgroundRenderTarget2D.IsValid());
				// @TODO: this has triggered (moving windows around) so instead just bail. but for now its handy to use the check
				ensure(BackgroundRenderTarget2D->GetRenderTargetResource());

				const FTextureResource* BackgroundOutputResource = static_cast<FTextureResource*>(BackgroundOutputRenderTarget->GetRenderTargetResource());
				const FRDGTextureRef Background = FLivRenderPass::CreateRDGTextureFromRenderTarget(
					GraphBuilder,
					BackgroundOutputResource,
					GBackgroundName
				);

				FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();

				Parameters->ForegroundTexture = LivForegroundTexture;
				Parameters->BackgroundTexture = Background;

				FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
			}
		}
	}

#endif

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}