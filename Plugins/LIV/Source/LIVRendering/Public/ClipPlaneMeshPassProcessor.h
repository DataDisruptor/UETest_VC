// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "MeshPassProcessor.h"
#include "RHIStaticStates.h"
#include "LivShaders.h"
#include "PostProcessing.h"
#include "PostProcessMaterial.h"
#include "SceneView.h"

///

BEGIN_SHADER_PARAMETER_STRUCT(FLivRenderClipPlanesParameters, LIVRENDERING_API)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

///

class LIVRENDERING_API FLivClipPlaneBasePassMeshProcessor : public FMeshPassProcessor
{
public:
	FLivClipPlaneBasePassMeshProcessor(
		const FScene* InScene,
		ERHIFeatureLevel::Type InFeatureLevel,
		const FSceneView* InViewIfDynamicMeshCommand,
		const FMeshPassProcessorRenderState& InDrawRenderState,
		FMeshPassDrawListContext* InDrawListContext,
		const bool bInBindPixelShader = false)
		: FMeshPassProcessor(InScene, InFeatureLevel, InViewIfDynamicMeshCommand, InDrawListContext)
		, PassDrawRenderState(InDrawRenderState)
		, bBindPixelShader(bInBindPixelShader)
	{
		PassDrawRenderState.SetDepthStencilState(TStaticDepthStencilState<true, CF_GreaterEqual>::GetRHI());
		//PassDrawRenderState.SetDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilNop);
		PassDrawRenderState.SetBlendState(TStaticBlendState<>::GetRHI());
		PassDrawRenderState.SetViewUniformBuffer(InViewIfDynamicMeshCommand->ViewUniformBuffer);
		PassDrawRenderState.SetInstancedViewUniformBuffer(Scene->UniformBuffers.InstancedViewUniformBuffer);
	}

	virtual void AddMeshBatch(
		const FMeshBatch& RESTRICT MeshBatch,
		uint64 BatchElementMask,
		const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
		int32 StaticMeshId = -1) override final
	{
		// Determine the mesh's material and blend mode.
		const FMaterialRenderProxy* FallbackMaterialRenderProxyPtr = nullptr;
		const FMaterial& Material = MeshBatch.MaterialRenderProxy->GetMaterialWithFallback(FeatureLevel, FallbackMaterialRenderProxyPtr);

		const FMaterialRenderProxy& MaterialRenderProxy = FallbackMaterialRenderProxyPtr ? *FallbackMaterialRenderProxyPtr : *MeshBatch.MaterialRenderProxy;
		check(MeshBatch.VertexFactory != nullptr);
		Process(MeshBatch, BatchElementMask, PrimitiveSceneProxy, MaterialRenderProxy, Material);
	}

	FMeshPassProcessorRenderState PassDrawRenderState;

private:

	bool bBindPixelShader;

	void Process(
		const FMeshBatch& MeshBatch,
		uint64 BatchElementMask,
		const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
		const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
		const FMaterial& RESTRICT MaterialResource);
};