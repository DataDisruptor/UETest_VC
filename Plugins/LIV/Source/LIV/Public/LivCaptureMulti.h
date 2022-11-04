// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureMulti.generated.h"

class ULivCustomClipPlane;

/**
 * 
 */
UCLASS()
class LIV_API ULivCaptureMulti : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureMulti(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV", meta = (LivStage = Input))
		USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* BackgroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* BackgroundOutputRenderTarget;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* ForegroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* ForegroundOutputRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* EyeAdaptionRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* BloomRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivCustomClipPlane* CameraClipPlane;

protected:

	virtual void OnActivated() override;
	virtual void OnDeactivated() override;

	virtual void CreateRenderTargets() override;
	virtual void ReleaseRenderTargets() override;

	virtual void Capture(const FLivCaptureContext& Context) override;

	TSharedPtr<class FLivSceneViewExtensionMulti, ESPMode::ThreadSafe> SceneViewExtension;
};
