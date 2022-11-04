// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Engine/Scene.h"
#include "Templates/SubclassOf.h"
#include "LivPluginSettings.generated.h"

UENUM(BlueprintType)
enum class ELivSceneViewExtensionCaptureStage : uint8
{
	PrePostProcess,
	AfterTonemap,
	AfterFXAA
};

/**
 * Liv plugin settings
 */
UCLASS(Config = Engine, DefaultConfig)
class LIV_API ULivPluginSettings : public UObject
{
	GENERATED_BODY()
	
public:

	ULivPluginSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Choose a subclass of ULivCaptureBase to select which rendering
	 * technique the LIV plugin will use to capture gameplay.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		TSubclassOf<class ULivCaptureBase> CaptureMethod;

	/**
	 * If your game uses its in-game avatar only and does not support
	 * LIV avatars then use background only capture to save performance.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		bool bBackgroundOnly;

	/**
	 * Does your game require transparency in the foreground?
	 * If so enable this but it requires the global clip plane be enabled
	 * which has a ~15% performance implication.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		bool bTransparency;

	/**
	 * Post process setting that will only apply to LIV output (if post processing
	 * is supported for current capture method).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		FPostProcessSettings PostProcessSettings;

	/**
	 * 
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		float PreExposure;

	/**
	 * Debugging Settings
	 */

	/**
	 * If enabled use debug settings values rather than setting them from LIV data.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		bool bUseDebugCamera;

	/**
	 * If bUseDebugCamera is enabled use this world location for the camera.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		FVector DebugCameraWorldLocation;

	/**
	 * If bUseDebugCamera is enabled use this world rotation for the camera.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		FRotator DebugCameraWorldRotation;

	/**
	 * If bUseDebugCamera is enabled use this (horizontal) field of view for the camera.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug", meta = (DisplayName = "Horizontal FOV", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float DebugCameraHorizontalFOV;

	/**
	 * Use debug values in settings for the clip plane rather than data from LIV.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		bool bUseDebugCameraClipPlane;

	/**
	 * Debug transform for camera clip plane.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		FTransform DebugCameraClipPlaneTransform;

	/**
	 * Use debug values in settings for the clip plane rather than data from LIV.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		bool bUseDebugFloorClipPlane;

	/**
	 * Debug transform for floor clip plane.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv|Debug")
		FTransform DebugFloorClipPlaneTransform;
	

#if WITH_EDITORONLY_DATA

	/**
	 * If enabled a message will be sent to request LIV
	 * capture out process automatically when developing in editor.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv", DisplayName = "Automatic Capture in Editor")
		bool bAutoCaptureInEditor;
	
#endif

	/**
	 * If set then the world subsystem will ensure an instance of this class exists in the world.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Liv")
		TSoftClassPtr<class ALivCameraController> CameraControllerClass;

	/**
	 * In general this setting should always be set to SCS_FinalToneCurveHDR.
	 */
	UPROPERTY(config, EditAnywhere, AdvancedDisplay, Category = "Liv", meta = (DisplayName = "Capture Source"))
		TEnumAsByte<enum ESceneCaptureSource> CaptureSource;

	/**
	 * Used internally, do not use.
	 * @TODO: Remove
	 */
	UPROPERTY(config, EditAnywhere, AdvancedDisplay, Category = "Liv")
		ELivSceneViewExtensionCaptureStage SceneViewExtensionCaptureStage;

	/**
	 * Links your project to the LIV Dev Portal.
	 * Use the setup wizard to set, do not set by hand.
	 */
	UPROPERTY(config, VisibleAnywhere, AdvancedDisplay, Category = "Liv")
		FString TrackingId;
};
