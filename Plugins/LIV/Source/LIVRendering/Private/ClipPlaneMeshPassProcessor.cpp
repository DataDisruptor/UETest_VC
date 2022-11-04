// Copyright 2021 LIV Inc. - MIT License


#include "ClipPlaneMeshPassProcessor.h"
#include "MeshPassProcessor.inl"

void FLivClipPlaneBasePassMeshProcessor::Process(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	const FPrimitiveSceneProxy* PrimitiveSceneProxy,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& MaterialResource)
{
	using LightMapPolicyType = FUniformLightMapPolicy;

	const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

	TMeshProcessorShaders<
		FLivClipPlaneVS,
		FMeshMaterialShader,
		FMeshMaterialShader,
		FLivClipPlanePS> BasePassShaders;

	BasePassShaders.VertexShader = MaterialResource.GetShader<FLivClipPlaneVS>(VertexFactory->GetType());
	if (bBindPixelShader)
	{
		BasePassShaders.PixelShader = MaterialResource.GetShader<FLivClipPlanePS>(VertexFactory->GetType());
	}

	const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
	const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(MeshBatch, MaterialResource, OverrideSettings);
	constexpr ERasterizerCullMode MeshCullMode = CM_None;

	FMeshMaterialShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, -1, false);

	const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(BasePassShaders.VertexShader, BasePassShaders.PixelShader);

	BuildMeshDrawCommands(
		MeshBatch,
		BatchElementMask,
		PrimitiveSceneProxy,
		MaterialRenderProxy,
		MaterialResource,
		PassDrawRenderState,
		BasePassShaders,
		MeshFillMode,
		MeshCullMode,
		SortKey,
		EMeshPassFeatures::Default,
		ShaderElementData);
}