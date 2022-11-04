// Copyright 2021 LIV Inc. - MIT License

#include "LivSceneViewExtensionSingle.h"

#include "LivConversions.h"
#include "LivCustomClipPlane.h"
#include "LivPluginSettings.h"
#include "LivRenderPass.h"
#include "LivShaders.h"
#include "MeshPassProcessor.h"
#include "MeshPassProcessor.inl"
#include "PostProcessing.h"
#include "PostProcessMaterial.h"
#include "SceneView.h"
#include "RHIStaticStates.h"
#include "ClipPlaneMeshPassProcessor.h"
#include "EngineModule.h"

TAutoConsoleVariable<bool> CVarRenderClipPlanes(TEXT("Liv.Debug.RenderClipPlanes"),
                                                true,
                                                TEXT("Debug disable render clip plane pass in scene view extension.")
);

#if PLATFORM_WINDOWS

static FRDGTextureRef CreateRDGTextureFromRenderTarget(
	FRDGBuilder& GraphBuilder,
	const FRenderTarget* RenderTarget,
	const TCHAR* DebugName)
{
	const FTexture2DRHIRef Texture = RenderTarget->GetRenderTargetTexture();

	ensure(Texture.IsValid());

	const FRHITexture2D* Texture2D = Texture->GetTexture2D();
	check(Texture2D != nullptr);

	FSceneRenderTargetItem Item;
	Item.TargetableTexture = Texture;
	Item.ShaderResourceTexture = Texture;

	FPooledRenderTargetDesc Desc;

	Desc.Extent = Texture2D->GetSizeXY();
	Desc.Format = Texture2D->GetFormat();
	Desc.NumMips = Texture2D->GetNumMips();

	Desc.DebugName = DebugName ? DebugName : TEXT("RenderTarget");
	Desc.TargetableFlags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;

	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	GRenderTargetPool.CreateUntrackedElement(Desc, PooledRenderTarget, Item);

	return GraphBuilder.RegisterExternalTexture(PooledRenderTarget, DebugName ? DebugName : TEXT("RenderTarget"));
}

///

BEGIN_SHADER_PARAMETER_STRUCT(FLivTranslucentBasePassParameters, )
SHADER_PARAMETER_STRUCT_REF(FSceneTextureUniformParameters, SceneTextures)
RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()


template<bool PostProcessing>
void ProcessLivPasses_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	FRDGTextureRef SceneColorTexture,
	FRDGTextureRef SceneDepthTexture,
	const TArray<ULivCustomClipPlane*>& ClipPlanes
)
{
	FRDGTextureDesc SceneColorDesc = SceneColorTexture->Desc;
	SceneColorDesc.Format = EPixelFormat::PF_B8G8R8A8;
	//SceneColorDesc.Flags |= TexCreate_RenderTargetable;
	SceneColorDesc.Extent = View.Family->RenderTarget->GetSizeXY();
	const FRDGTextureRef LivBackgroundTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("LivBackground"), ERDGTextureFlags::None);
	const FRDGTextureRef LivForegroundTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("LivForeground"), ERDGTextureFlags::None);
	const FRDGTextureRef LivBackgroundCopyTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("LivBackgroundCopy"), ERDGTextureFlags::None);

	//

	FRDGTextureDesc SceneDepthDesc = SceneColorTexture->Desc;
	SceneDepthDesc.Format = EPixelFormat::PF_R32_FLOAT;
	//SceneDepthDesc.Flags |= (TexCreate_UAV | TexCreate_RenderTargetable);
	SceneDepthDesc.Extent = View.Family->RenderTarget->GetSizeXY();
	const FRDGTextureRef LivBackgroundDepthTexture = GraphBuilder.CreateTexture(SceneDepthDesc, TEXT("LivForegroundDepth"), ERDGTextureFlags::None);
	//check(LivBackgroundDepthTexture->Desc.Flags & (TexCreate_UAV | TexCreate_RenderTargetable));

	// Copy scene depth
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Copy Scene Color And Depth");

		const ERHIFeatureLevel::Type FeatureLevel = View.GetFeatureLevel();
		const auto GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

		const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
		const TShaderMapRef<FLivRDGCopySceneColorAndDepthPS> PixelShader(GlobalShaderMap);

		FLivRDGCopySceneColorAndDepthPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGCopySceneColorAndDepthPS::FParameters>();
		Parameters->RenderTargets[0] = FRenderTargetBinding(LivBackgroundCopyTexture, ERenderTargetLoadAction::EClear);
		Parameters->RenderTargets[1] = FRenderTargetBinding(LivBackgroundDepthTexture, ERenderTargetLoadAction::EClear);
		Parameters->View = View.ViewUniformBuffer;
		Parameters->SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All); // @TODO: optimise setup mode

		const FScreenPassTextureViewport ScreenPassTextureViewport(SceneColorTexture);
		const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

		FLivRenderPass::AddLivPass(
			GraphBuilder,
			RDG_EVENT_NAME("Liv RDG Copy Scene Color And Depth Pass"),
			ScreenPassTextureViewport,
			PipelineState,
			PixelShader,
			Parameters
		);
	}

	// Render clip planes
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Render Clip Planes");

		FLivRenderClipPlanesParameters* PassParameters = GraphBuilder.AllocParameters<FLivRenderClipPlanesParameters>();
		PassParameters->RenderTargets[0] = FRenderTargetBinding(SceneColorTexture, ERenderTargetLoadAction::ELoad);;
		PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
			SceneDepthTexture,
			ERenderTargetLoadAction::ELoad,
			ERenderTargetLoadAction::ENoAction,
			FExclusiveDepthStencil(FExclusiveDepthStencil::DepthWrite_StencilNop)
		);
		PassParameters->View = View.ViewUniformBuffer;

		TArray<const FPrimitiveSceneProxy*> ClipPlaneSceneProxies = GetClipPlaneSceneProxies(ClipPlanes);

		if (ClipPlaneSceneProxies.Num() > 0 && CVarRenderClipPlanes.GetValueOnRenderThread())
		{
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("Liv Render Clip Planes Pass"),
				PassParameters,
				ERDGPassFlags::Raster,
				[&View, ClipPlaneSceneProxies, PassParameters](FRHICommandList& RHICmdList)
				{
					FMeshPassProcessorRenderState DrawRenderState(View);
					RHICmdList.SetViewport(View.UnscaledViewRect.Min.X, View.UnscaledViewRect.Min.Y, 0.0f, View.UnscaledViewRect.Max.X, View.UnscaledViewRect.Max.Y, 1.0f);

					DrawDynamicMeshPass(View, RHICmdList,
						[&View, &DrawRenderState, ClipPlaneSceneProxies](FDynamicPassMeshDrawListContext* DynamicMeshPassContext)
						{
							FLivClipPlaneBasePassMeshProcessor PassMeshProcessor(
								View.Family->Scene->GetRenderScene(),
								View.GetFeatureLevel(),
								&View,
								DrawRenderState,
								DynamicMeshPassContext);

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
				}
			);
		}
	}

	/*
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Render Transparency");


		FLivTranslucentBasePassParameters* PassParameters = GraphBuilder.AllocParameters<FLivTranslucentBasePassParameters>();

		const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("SeparateTranslucency"),
			PassParameters,
			ERDGPassFlags::Raster,
			[&View=ViewInfo](FRHICommandListImmediate& RHICmdList)
			{
				//RenderViewTranslucencyInner(RHICmdList, SceneRenderer, View, Viewport, ViewportScale, TranslucencyPass, nullptr);

				FMeshPassProcessorRenderState DrawRenderState(View);
				DrawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_DepthNearOrEqual>::GetRHI());

				View.SimpleElementCollector.DrawBatchedElements(RHICmdList, DrawRenderState, View, EBlendModeFilter::Translucent, SDPG_World);
				View.SimpleElementCollector.DrawBatchedElements(RHICmdList, DrawRenderState, View, EBlendModeFilter::Translucent, SDPG_Foreground);
			});
	}
	*/

	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Segment");

		const ERHIFeatureLevel::Type FeatureLevel = View.GetFeatureLevel();
		const auto GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

		const TShaderMapRef<FLivRDGScreenPassVS> VertexShader(GlobalShaderMap);
		const TShaderRef<FLivRDGSegmentPS> PixelShader = GlobalShaderMap->GetShader<TLivRDGSegmentPS<PostProcessing>>();

		FLivRDGSegmentPS::FParameters* Parameters = GraphBuilder.AllocParameters<FLivRDGSegmentPS::FParameters>();
		if (PostProcessing)
		{
			Parameters->InputTexture = SceneColorTexture;
			Parameters->InputSampler = TStaticSamplerState<>::GetRHI();
		}
		Parameters->InputBackgroundTexture = LivBackgroundCopyTexture;
		Parameters->InputBackgroundSampler = TStaticSamplerState<>::GetRHI();
		Parameters->InputBackgroundDepthTexture = LivBackgroundDepthTexture;
		Parameters->InputBackgroundDepthSampler = TStaticSamplerState<>::GetRHI();
		Parameters->RenderTargets[0] = FRenderTargetBinding(LivForegroundTexture, ERenderTargetLoadAction::EClear);
		Parameters->RenderTargets[1] = FRenderTargetBinding(LivBackgroundTexture, ERenderTargetLoadAction::EClear);
		Parameters->View = View.ViewUniformBuffer;
		Parameters->SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All);

		const FScreenPassTextureViewport ScreenPassTextureViewport(PostProcessing ? SceneColorTexture : LivForegroundTexture);
		const FScreenPassPipelineState PipelineState(VertexShader, PixelShader);

		FLivRenderPass::AddLivPass(
			GraphBuilder,
			RDG_EVENT_NAME("Liv Segment Pass"),
			ScreenPassTextureViewport,
			PipelineState,
			PixelShader,
			Parameters
		);
	}

	{
		RDG_EVENT_SCOPE(GraphBuilder, "Liv Submit");

		FLivSubmitParameters* Parameters = GraphBuilder.AllocParameters<FLivSubmitParameters>();
		Parameters->ForegroundTexture = LivForegroundTexture;
		Parameters->BackgroundTexture = LivBackgroundTexture;

		FLivRenderPass::AddSubmitPass(GraphBuilder, Parameters);
	}
}

void ProcessLivBackgroundOnly_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View)
{
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
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////

FLivSceneViewExtensionSingle::FLivSceneViewExtensionSingle(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient)
	: FLivSceneViewExtensionBase(AutoRegister, AssociatedViewportClient)
{
}

FLivSceneViewExtensionSingle::~FLivSceneViewExtensionSingle()
{
}

/**
 * NOTE: We probably want to restrict this to after FXAA (should be last pass)
 * Flexible atm whilst working things out
 */

void FLivSceneViewExtensionSingle::SubscribeToPostProcessingPass(
	EPostProcessingPass Pass,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks, 
	bool bIsPassEnabled)
{
	const ELivSceneViewExtensionCaptureStage CaptureStage = GetDefault<ULivPluginSettings>()->SceneViewExtensionCaptureStage;
	if (Pass == EPostProcessingPass::Tonemap && CaptureStage == ELivSceneViewExtensionCaptureStage::AfterTonemap)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionSingle::PostProcessPassAfterTonemap_RenderThread));
	}
	else if(Pass == EPostProcessingPass::FXAA && CaptureStage == ELivSceneViewExtensionCaptureStage::AfterFXAA)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FLivSceneViewExtensionSingle::PostProcessPassAfterFXAA_RenderThread));
	}
}

void FLivSceneViewExtensionSingle::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	FLivSceneViewExtensionBase::SetupView(InViewFamily, InView);
}

void FLivSceneViewExtensionSingle::PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList,
                                                                   FSceneView& View)
{
	FLivSceneViewExtensionBase::PostRenderBasePass_RenderThread(RHICmdList, View);

	return;

	//FRHIRenderPassInfo RPInfo(View.Family->RenderTarget->GetRenderTargetTexture(), ERenderTargetActions::Load_Store);

	//RHICmdList.BeginRenderPass(RPInfo, TEXT("LivPostRenderBasePass"));

	TArray<const FPrimitiveSceneProxy*> ClipPlaneSceneProxies = GetClipPlaneSceneProxies(ClipPlanes);

	FMeshPassProcessorRenderState DrawRenderState(View);

	/*DrawRenderState.SetDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilNop);

	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	FTexture2DRHIRef DepthTarget = SceneContext.GetSceneDepthSurface();

	RHICmdList.Transition(FRHITransitionInfo(DepthTarget, ERHIAccess::Unknown, ERHIAccess::DSVWrite | ERHIAccess::DSVRead));*/

	RHICmdList.SetViewport(View.UnscaledViewRect.Min.X, View.UnscaledViewRect.Min.Y, 0.0f, View.UnscaledViewRect.Max.X, View.UnscaledViewRect.Max.Y, 1.0f);

	DrawDynamicMeshPass(View, RHICmdList,
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

	//RHICmdList.EndRenderPass();
}

void FLivSceneViewExtensionSingle::PrePostProcessPass_RenderThread(
	FRDGBuilder& GraphBuilder, 
	const FSceneView& View,
	const FPostProcessingInputs& Inputs)
{
#if PLATFORM_WINDOWS

	if(!IsValidForBoundRenderTarget(*View.Family))
	{
		return;
	}

	// @NOTE: this path will not yield any post processing like anti-aliasing and tonemapping
	// so we'd have to live without or add it back manually
	// OR just do some processing here like rendering depth and capture later on (though may as well just do it all later?)
	if (GetDefault<ULivPluginSettings>()->SceneViewExtensionCaptureStage == ELivSceneViewExtensionCaptureStage::PrePostProcess)
	{
		ProcessLivPasses_RenderThread<false>(GraphBuilder, View, (*Inputs.SceneTextures)->SceneColorTexture, (*Inputs.SceneTextures)->SceneDepthTexture, ClipPlanes);
	}

#endif
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
FScreenPassTexture FLivSceneViewExtensionSingle::PostProcessPassAfterTonemap_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
	//check(View.bIsSceneCapture);

#if PLATFORM_WINDOWS

	if (IsValidForBoundRenderTarget(*View.Family))
	{
		if (GetDefault<ULivPluginSettings>()->bBackgroundOnly)
		{
			ProcessLivBackgroundOnly_RenderThread(GraphBuilder, View);
		}
		else
		{
			const FScreenPassTexture& SceneColor = InOutInputs.Textures[static_cast<uint32>(EPostProcessMaterialInput::SceneColor)];
			const FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);

			const TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures = CreateSceneTextureUniformBuffer(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All);
			ProcessLivPasses_RenderThread<true>(GraphBuilder, View, SceneColorRenderTarget.Texture, (*SceneTextures)->SceneDepthTexture, ClipPlanes);
		}
	}

#endif

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
FScreenPassTexture FLivSceneViewExtensionSingle::PostProcessPassAfterFXAA_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
	//check(View.bIsSceneCapture);

#if PLATFORM_WINDOWS

	if (IsValidForBoundRenderTarget(*View.Family))
	{
		if (GetDefault<ULivPluginSettings>()->bBackgroundOnly)
		{
			ProcessLivBackgroundOnly_RenderThread(GraphBuilder, View);
		}
		else
		{
			const FScreenPassTexture& SceneColor = InOutInputs.Textures[static_cast<uint32>(EPostProcessMaterialInput::SceneColor)];
			const FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);

			const TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures = CreateSceneTextureUniformBuffer(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::All);
			ProcessLivPasses_RenderThread<true>(GraphBuilder, View, SceneColorRenderTarget.Texture, (*SceneTextures)->SceneDepthTexture, ClipPlanes);
		}
	}

#endif

	return AddCopyPassIfLastPass(GraphBuilder, View, InOutInputs);
}