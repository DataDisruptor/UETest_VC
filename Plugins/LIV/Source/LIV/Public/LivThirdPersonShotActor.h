// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivThirdPersonShotActor.generated.h"

UCLASS()
class LIV_API ALivThirdPersonShotActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALivThirdPersonShotActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LIV|Shot")
		class ULivSpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LIV|Shot")
		class ULivShotComponent* ShotComponent;

	UFUNCTION()
		void TickSpringArm(class ALivCameraController* Controller, class ULivCaptureBase* CaptureComponent, float ShotTime, float DeltaTime);
		
};
