// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivSceneViewExtensionsCommon.h"
#include "SceneViewExtension.h"
#include "SceneView.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneView.h"

/**
 * 
 */
class LIV_API FLivSceneViewExtensionSingle : public FLivSceneViewExtensionBase
{
public:

	FLivSceneViewExtensionSingle(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient = nullptr);
	virtual ~FLivSceneViewExtensionSingle() override;
	
	virtual void SubscribeToPostProcessingPass(
		EPostProcessingPass Pass, 
		FAfterPassCallbackDelegateArray& InOutPassCallbacks, 
		bool bIsPassEnabled) override;

	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;

	virtual void PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;

	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;

	TArray<class ULivCustomClipPlane*> ClipPlanes;
	TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget2D;

	/**
	 * Checks if the view family render target matches the one we set from the
	 * scene capture component each frame. Allows us to filter to only
	 * our capture components rather than others in the scene (e.g. VR Spectator)
	 */
	bool IsValidForBoundRenderTarget(const FSceneViewFamily& Family) const
	{
		return RenderTarget2D.IsValid() && Family.RenderTarget == RenderTarget2D->GetRenderTargetResource();
	}

protected:

	FScreenPassTexture PostProcessPassAfterTonemap_RenderThread(
		FRDGBuilder& GraphBuilder, 
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);

	FScreenPassTexture PostProcessPassAfterFXAA_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);
};


