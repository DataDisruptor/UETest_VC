// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivSceneViewExtensionsCommon.h"
#include "SceneViewExtension.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneView.h"

#ifndef WITH_EYE_ADAPTATION_CALLBACK
#define WITH_EYE_ADAPTATION_CALLBACK 0
#endif

/**
 * 
 */
class LIV_API FLivSceneViewExtensionMulti : public FLivSceneViewExtensionBase
{
public:
	FLivSceneViewExtensionMulti(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient = nullptr);
	virtual ~FLivSceneViewExtensionMulti() override;

	// Required due to being abstract:
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;

	#if WITH_EYE_ADAPTATION_CALLBACK
	virtual void PostEyeAdaptation_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, FRDGTextureRef EyeAdaptationTexture) override;
	#endif
		
	virtual void SubscribeToPostProcessingPass(
		EPostProcessingPass Pass,
		FAfterPassCallbackDelegateArray& InOutPassCallbacks,
		bool bIsPassEnabled) override;
	
	TWeakObjectPtr<UTextureRenderTarget2D> ForegroundRenderTarget2D;
	TWeakObjectPtr<UTextureRenderTarget2D> ForegroundOutputRenderTarget2D;
	TWeakObjectPtr<UTextureRenderTarget2D> BackgroundRenderTarget2D;
	TWeakObjectPtr<UTextureRenderTarget2D> BackgroundOutputRenderTarget;
	TWeakObjectPtr<UTextureRenderTarget2D> EyeAdaptionRenderTarget2D;

	bool IsForegroundCapture(const FSceneViewFamily& Family) const
	{
		return ForegroundRenderTarget2D.IsValid() && Family.RenderTarget == ForegroundRenderTarget2D->GetRenderTargetResource();
	}

	bool IsBackgroundCapture(const FSceneViewFamily& Family) const
	{
		return BackgroundRenderTarget2D.IsValid() && Family.RenderTarget == BackgroundRenderTarget2D->GetRenderTargetResource();
	}

	bool IsReadyForSubmit() const;

	TArray<class ULivCustomClipPlane*> ClipPlanes;
		
protected:

	void OnPostOpaque(class FPostOpaqueRenderParameters& Parameters) const;

	FDelegateHandle PostOpaqueHandle {};
	void* ForegroundViewUid { nullptr };

	uint32 ForegroundFrameNumber{ 0u };
	uint32 BackgroundFrameNumber{ 0u };

	FScreenPassTexture PostProcessPassAfterFXAA_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& InOutInputs);
};
