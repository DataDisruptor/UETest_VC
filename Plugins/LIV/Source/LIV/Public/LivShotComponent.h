// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LivShotComponent.generated.h"

class ALivDirector;
class ALivCameraController;
class ULivCaptureBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLivShotTickDelegate, ALivCameraController*, Controller, ULivCaptureBase*, CaptureComponent, float, ShotTime, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLivShotCutDelegate, ALivCameraController*, Controller);


class ALivDirector;
UCLASS( ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class LIV_API ULivShotComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	
	ULivShotComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LIV|Shot")
		float Score;

	/** Camera field of view (in degrees). */
	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category = Projection, meta = (DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float FOVAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LIV|Shot")
		uint32 bOverrideCamera:1;

	UPROPERTY(BlueprintAssignable, Category = "LIV|Shot")
		FLivShotTickDelegate TickShotEvent;

	UPROPERTY(BlueprintAssignable, Category = "LIV|Shot")
		FLivShotCutDelegate CutToEvent;

	UPROPERTY(BlueprintAssignable, Category = "LIV|Shot")
		FLivShotCutDelegate CutFromEvent;

public:

	UFUNCTION(BlueprintNativeEvent, Category="LIV|Shot")
	void TickShot(ALivCameraController* Controller, ULivCaptureBase* CaptureComponent, float ShotTime, float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "LIV|Shot")
	void OnCutTo(ALivCameraController* Controller);

	UFUNCTION(BlueprintNativeEvent, Category = "LIV|Shot")
	void OnCutFrom(ALivCameraController* Controller);
		
};
