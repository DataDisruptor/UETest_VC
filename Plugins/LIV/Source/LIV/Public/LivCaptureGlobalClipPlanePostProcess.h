// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureGlobalClipPlanePostProcess.generated.h"

/**
 * ULivCaptureGlobalClipPlanePostProcess
 * 
 * Render foreground with clipping plane and post processing for foreground color,
 * render foreground with clipping plane without post processing for opacity mask,
 * Render Background with post processing for background color.
 * Draw the post processed background,
 * Draw the post processed foreground where the mask from the non post processed foreground is 1. 
 * Write the mask to alpha too.
 */
UCLASS(ClassGroup = LIV, BlueprintType)
class LIV_API ULivCaptureGlobalClipPlanePostProcess : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureGlobalClipPlanePostProcess(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditAnywhere, Category = "LIV", meta=(LivStage=Input))
		USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Input))
		UTextureRenderTarget2D* PostProcessedBackgroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Input))
		UTextureRenderTarget2D* PostProcessedForegroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output,LivMask=A))
		UTextureRenderTarget2D* ForegroundInverseOpacityRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output,LivMask=A))
		UTextureRenderTarget2D* ForegroundOutputRenderTarget;

protected:

	void OnActivated() override;
	void OnDeactivated() override;

	void CreateRenderTargets() override;
	void ReleaseRenderTargets() override;

	void Capture(const struct FLivCaptureContext& Context) override;
};
