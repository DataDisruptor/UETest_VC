// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "LivSpringArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class LIV_API ULivSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:

	ULivSpringArmComponent(const FObjectInitializer& ObjectInitializer);

	/* Restrict pitch between MinPitch and MaxPitch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		bool bClampPitch;

	/* Minimum pitch in degrees (if bClampPitch is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot", meta = (UIMin = "-90.0", UIMax = "90.0", ClampMin = "-90.0", ClampMax = "90.0"))
		float MinPitch;

	/* Maximum pitch in degrees (if bClampPitch is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot", meta = (UIMin = "-90.0", UIMax = "90.0", ClampMin = "-90.0", ClampMax = "90.0"))
		float MaxPitch;

	/* Only adjust yaw if our target is greater than this threshold in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		float YawDeltaThreshold;

	/* We divide the difference in yaw per frame by this to calculate interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		float MaxYawDelta;

	/* Min speed to interpolate to target rotation (alpha = Yaw Difference / MaxYawDelta in 0..1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		float MinInterpolationSpeed;

	/* Max speed to interpolate to target rotation (alpha = Yaw Difference / MaxYawDelta in 0..1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		float MaxInterpolationSpeed;

public:
	
	UFUNCTION(BlueprintCallable, Category = "LIV|Shot")
		virtual void UpdatePoseForCamera(class UCameraComponent* Camera, float DeltaTime);
};
