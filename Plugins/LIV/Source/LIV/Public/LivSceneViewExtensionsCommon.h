// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"

class LIV_API FLivSceneViewExtensionBase : public FSceneViewExtensionBase
{
public:
	FLivSceneViewExtensionBase(const FAutoRegister& AutoRegister, FViewportClient* AssociatedViewportClient = nullptr);
	virtual ~FLivSceneViewExtensionBase() override {}

	virtual bool IsActiveThisFrameInContext(FSceneViewExtensionContext& Context) const override;

	// Required due to being abstract:
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}

protected:

	FScreenPassTexture AddCopyPassIfLastPass(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs) const;

};
