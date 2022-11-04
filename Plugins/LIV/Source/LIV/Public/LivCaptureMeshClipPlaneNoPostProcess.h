// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "LivCaptureBase.h"
#include "LivCaptureMeshClipPlaneNoPostProcess.generated.h"

// Forward declarations
class ULivClipPlane;

/**
 * ULivCaptureMeshClipPlaneNoPostProcess
 * 
 * Render full scene for color and depth,
 * render foreground using mesh as a clipping plane for just depth.
 * Draw the full scene, using the depths from the two images create 
 * a mask where when depth is equal mask == 1 else 0.
 * Draw the full post processes scene where mask is 1 and write it to alpha too.
 */
UCLASS(ClassGroup = LIV, BlueprintType)
class LIV_API ULivCaptureMeshClipPlaneNoPostProcess : public ULivCaptureBase
{
	GENERATED_BODY()

public:

	ULivCaptureMeshClipPlaneNoPostProcess(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivClipPlane* CameraClipPlane;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIV")
		ULivClipPlane* FloorClipPlane;

	// @TODO: add floor clip plane

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Input,LivDepth=A))
		UTextureRenderTarget2D* BackgroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Input,LivDepth=R))
		UTextureRenderTarget2D* ForegroundRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output))
		UTextureRenderTarget2D* BackgroundOutputRenderTarget;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LIV", meta=(LivStage=Output,LivMask=A))
		UTextureRenderTarget2D* ForegroundOutputRenderTarget;


protected:

	void OnActivated() override;
	void OnDeactivated() override;

	void CreateRenderTargets() override;
	void ReleaseRenderTargets() override;

	void Capture(const struct FLivCaptureContext& Context) override;
};
