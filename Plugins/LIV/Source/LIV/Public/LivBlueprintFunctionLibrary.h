// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LivBlueprintFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class ELivEye : uint8
{
	Center,
	Left,
	Right
};

/**
 * Common functionality for LIV accessible in blueprints.
 */
UCLASS()
class LIV_API ULivBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/**
	 * Adjust incoming camera world location and rotation to account for given eye. Does nothing if eye is center.
	 */
	UFUNCTION(BlueprintPure, Category = "Liv")
		static void OffsetCameraPoseForEye(ELivEye Eye, const FVector& CameraLocation, const FRotator& CameraRotation, FVector& EyeLocation, FRotator& EyeRotation);

	/**
	 * Create a transient render target. This version exposes bForceLinearGamma.
	 */
	UFUNCTION(BlueprintCallable, Category = "Liv", meta = (WorldContext = "WorldContextObject"))
		static UTextureRenderTarget2D* CreateRenderTarget2D(
			UObject* WorldContextObject,
			int32 Width,
			int32 Height,
			bool bForceLinearGamma = true,
			FName Name = NAME_None,
			ETextureRenderTargetFormat Format = ETextureRenderTargetFormat::RTF_RGBA8,
			FLinearColor ClearColor = FLinearColor::Black,
			float TargetGamma = 0.0f);
};
