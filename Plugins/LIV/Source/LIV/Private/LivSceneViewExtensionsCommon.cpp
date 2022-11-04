// Copyright 2021 LIV Inc. - MIT License

#include "LivSceneViewExtensionsCommon.h"
#include "ScreenPass.h"
#include "PostProcess/PostProcessMaterial.h"

FLivSceneViewExtensionBase::FLivSceneViewExtensionBase(
	const FAutoRegister& AutoRegister,
	FViewportClient* AssociatedViewportClient)
	: FSceneViewExtensionBase(AutoRegister)
{

}

bool FLivSceneViewExtensionBase::IsActiveThisFrameInContext(FSceneViewExtensionContext& Context) const
{
	// Viewport is nullptr for scene capture components
	// and we only want to apply to scene capture components
	return Context.Viewport == nullptr;
}

FScreenPassTexture FLivSceneViewExtensionBase::AddCopyPassIfLastPass(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs) const
{
	const FScreenPassTexture& SceneColor = InOutInputs.Textures[static_cast<uint32>(EPostProcessMaterialInput::SceneColor)];

	// @NOTE:	Using the scene color above _should_ be fine but in VR for some reason its junk
	//			So we've got to create a uniform buffer manually with this view and then grab the scene color
	//const TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures = CreateSceneTextureUniformBuffer(GraphBuilder, View.GetFeatureLevel(), ESceneTextureSetupMode::SceneColor);
	//const FScreenPassRenderTarget SceneColor((*SceneTextures)->SceneColorTexture, ERenderTargetLoadAction::ELoad);

	// Generally this is the last pass in the chain so we need to copy into override output 
	if (InOutInputs.OverrideOutput.IsValid())
	{
		// @NOTE: See FScreenPassTexture AddPostProcessMaterialPass
		// @TODO: Might need to handle bCopyAndFlip

		check(View.bIsViewInfo);
		const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
		//AddDrawTexturePass(GraphBuilder, ViewInfo, SceneColor.Texture, InOutInputs.OverrideOutput.Texture);
		AddDrawTexturePass(
			GraphBuilder, 
			ViewInfo,
			SceneColor.Texture, 
			InOutInputs.OverrideOutput.Texture,
			FIntPoint::ZeroValue,
			FIntPoint::ZeroValue,
			SceneColor.Texture->Desc.Extent);
		return FScreenPassTexture(InOutInputs.OverrideOutput);
	}

	return FScreenPassTexture(SceneColor);
}
