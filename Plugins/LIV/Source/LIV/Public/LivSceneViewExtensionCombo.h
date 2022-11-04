// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivSceneViewExtensionsCommon.h"
#include "SceneViewExtension.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneView.h"

class LIV_API FLivSceneViewExtensionCombo : public FLivSceneViewExtensionBase
{
public:
	FLivSceneViewExtensionCombo(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient = nullptr);
	virtual ~FLivSceneViewExtensionCombo() override;

	// Required due to being abstract:
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}

	virtual void SubscribeToPostProcessingPass(
		EPostProcessingPass Pass,
		FAfterPassCallbackDelegateArray& InOutPassCallbacks,
		bool bIsPassEnabled) override;

	TWeakObjectPtr<UTextureRenderTarget2D> ForegroundRenderTarget2D;
	TWeakObjectPtr<UTextureRenderTarget2D> BackgroundRenderTarget2D;

	bool IsForegroundCapture(const FSceneViewFamily& Family) const
	{
		return ForegroundRenderTarget2D.IsValid() && Family.RenderTarget == ForegroundRenderTarget2D->GetRenderTargetResource();
	}

	bool IsBackgroundCapture(const FSceneViewFamily& Family) const
	{
		return BackgroundRenderTarget2D.IsValid() && Family.RenderTarget == BackgroundRenderTarget2D->GetRenderTargetResource();
	}

	bool IsReadyForSubmit() const;

protected:

	uint32 ForegroundFrameNumber{ 0u };
	uint32 BackgroundFrameNumber{ 0u };

	bool bTransparency {false};
	bool bBackgroundOnly {false};
	
	FScreenPassTexture PostProcessPassAfterFXAA_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);

	FScreenPassTexture PostProcessPassAfterFXAABackgroundOnly_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);

	FScreenPassTexture PostProcessPassAfterFXAATransparency_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);
};
