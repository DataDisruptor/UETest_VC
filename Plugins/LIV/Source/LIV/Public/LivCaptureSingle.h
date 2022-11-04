// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureSingle.generated.h"

class ULivCustomClipPlane;

/**
 * 
 */
UCLASS()
class LIV_API ULivCaptureSingle : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureSingle(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivCustomClipPlane* CameraClipPlane;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivCustomClipPlane* FloorClipPlane;
	
	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta = (LivStage = Output))
		UTextureRenderTarget2D* BackgroundOutputRenderTarget;

protected:

	virtual void OnActivated() override;
	virtual void OnDeactivated() override;

	virtual void CreateRenderTargets() override;
	virtual void ReleaseRenderTargets() override;

	virtual void Capture(const FLivCaptureContext& Context) override;

	TSharedPtr<class FLivSceneViewExtensionSingle, ESPMode::ThreadSafe> SceneViewExtension;
};
