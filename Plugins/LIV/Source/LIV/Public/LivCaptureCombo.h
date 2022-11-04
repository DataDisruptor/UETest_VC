// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureCombo.generated.h"

/**
 * 
 */
UCLASS()
class LIV_API ULivCaptureCombo : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureCombo(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "LIV", meta = (LivStage = Input))
		USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* BackgroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* ForegroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* ForegroundOutputRenderTarget;

protected:

	virtual void OnActivated() override;
	virtual void OnDeactivated() override;

	virtual void CreateRenderTargets() override;
	virtual void ReleaseRenderTargets() override;

	virtual void Capture(const FLivCaptureContext& Context) override;

	TSharedPtr<class FLivSceneViewExtensionCombo, ESPMode::ThreadSafe> SceneViewExtension;
};
