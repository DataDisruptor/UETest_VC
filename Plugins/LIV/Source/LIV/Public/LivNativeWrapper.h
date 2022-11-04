// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivNativeWrapper.generated.h"

class USceneCaptureComponent2D;

DECLARE_LOG_CATEGORY_EXTERN(LogLivNativeWrapper, Log, All);

USTRUCT(BlueprintType)
struct LIV_API FLivInputFrame
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FVector CameraLocation;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FQuat CameraRotation;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FIntPoint Dimensions;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		float HorizontalFieldOfView;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FMatrix CameraClipPlaneMatrix;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FMatrix FloorClipPlaneMatrix;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		uint32 bFloorClipPlaneEnabled : 1;

	FRotator GetCameraRotator() const { return CameraRotation.Rotator(); }

	FTransform GetCameraClipPlaneTransform() const { return FTransform(CameraClipPlaneMatrix); }

	FTransform GetFloorClipPlaneTransform() const { return FTransform(FloorClipPlaneMatrix); }

	float GetHorizontalFieldOfView() const;
};

/**
 * Wraps LIV native library to help reduce the amount of exposed headers/symbols.
 */
class LIV_API FLivNativeWrapper
{
public:

	/**
	 * Returns true is LIV is supported on this platform.
	 */
	static bool IsSupported();

	/**
	 * Returns true if the LIV libraries are loaded and initialized properly.
	 */
	static bool StartUp();

	/**
	 * Call when module shuts down.
	 */
	static void Shutdown();

	/**
	 * Checks if LIV is actively connected to the game (i.e. we should be sending frames).
	 */
	static bool IsActive();

	/**
	 * Signals to LIV we are ready to send frames.
	 */
	static void Start();

	/**
	 * Get version string of LIV Native SDK if LIV is supported on this platform (else returns "Unsupported")
	 */
	static FString GetVersionString();

	/**
	 * Get latest input frame from LIV.
	 */
	static bool GetInputFrame(FLivInputFrame& OutInputFrame);
	
	/**
	 * Get latest input frame from LIV.
	 * If OverridingSceneCaptureComponent is provided then there will be a request for LIV to use that data.
	 */
	static bool UpdateInputFrame(FLivInputFrame& InOutInputFrame, USceneCaptureComponent2D* OverridingSceneCaptureComponent = nullptr);

	/**
	 * Sends data about this project to LIV.
	 */
	static void SubmitApplicationInformation();
};
