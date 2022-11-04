// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureMeshClipPlanePostProcess.generated.h"

/**
 * ULivCaptureMeshClipPlanePostProcess
 * 
 * Render full scene post processed for color,
 * Render full scene not post processed for depth,
 * Render full scene with clip plane mesh for depth.
 * Draw full post processed scene,
 * Using the same full post processed scene image,
 * Using the depths from the two other images create a mask where when depth is equal mask == 1 else 0.
 * Draw the post processed scene masked by the mask. Write the mask into alpha too.
 * 
 * Development Notes:
 * - We need 2 scene capture components so that the rendering state can persist 
 *   across frames for the post processing
 * - We currently need to copy the background from a floating point pixel format 
 *   to an 8bpc pixel format
 */
UCLASS(ClassGroup = LIV, BlueprintType)
class LIV_API ULivCaptureMeshClipPlanePostProcess : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureMeshClipPlanePostProcess(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV")
		USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivClipPlane* CameraClipPlane;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivClipPlane* FloorClipPlane;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "LIV", meta=(LivStage=Input))
		UTextureRenderTarget2D* PostProcessedSceneRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "LIV", meta=(LivStage=Input,LivDepth=R))
		UTextureRenderTarget2D* BackgroundDepthRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "LIV", meta=(LivStage=Input,LivDepth=R))
		UTextureRenderTarget2D* ForegroundDepthRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "LIV", meta=(LivStage=Output))
		UTextureRenderTarget2D* BackgroundOutputRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "LIV", meta=(LivStage=Output,LivMask=A))
		UTextureRenderTarget2D* ForegroundOutputRenderTarget;

protected:

	void OnActivated() override;
	void OnDeactivated() override;

	void CreateRenderTargets() override;
	void ReleaseRenderTargets() override;

	void Capture(const struct FLivCaptureContext& Context) override;
};
