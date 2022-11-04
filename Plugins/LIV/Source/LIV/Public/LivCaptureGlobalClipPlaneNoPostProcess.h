// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureGlobalClipPlaneNoPostProcess.generated.h"

/**
 * ULivCaptureGlobalClipPlaneNoPostProcess
 * 
 * Render foreground with clipping plane without post processing for opacity mask and color,
 * render full scene without post processing for color.
 * Draw the background,
 * Draw the foreground where the mask is 1 and write the mask to alpha too.
 */
UCLASS(ClassGroup = LIV, BlueprintType)
class LIV_API ULivCaptureGlobalClipPlaneNoPostProcess : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureGlobalClipPlaneNoPostProcess(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output))
		UTextureRenderTarget2D* BackgroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Input,LivMask=A))
		UTextureRenderTarget2D* ForegroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output,LivMask=A))
		UTextureRenderTarget2D* ForegroundMaskedRenderTarget;

protected:

	void OnActivated() override;
	void OnDeactivated() override;

	void CreateRenderTargets() override;
	void ReleaseRenderTargets() override;

	void Capture(const struct FLivCaptureContext& Context) override;
};
