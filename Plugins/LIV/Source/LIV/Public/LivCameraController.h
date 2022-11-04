// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivCameraController.generated.h"

class ULivShotComponent;

/**
 * If a camera controller class is assigned in the plugin settings then an instance of the class will be
 * spawned by the LIV world subsystem.
 *
 * If a camera controller has an active shot that wants control over the camera then it will tick
 * that shot and override the camera.
 */
UCLASS(BlueprintType, Transient, Blueprintable, NotPlaceable)
class LIV_API ALivCameraController : public AActor
{
	GENERATED_BODY()
	
public:	
	ALivCameraController(const FObjectInitializer& ObjectInitializer);

	/**
	 * Currently active shot that controls the camera.
	 */
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "LIV|Shot")
		ULivShotComponent* CurrentShot;

	/**
	 * Get current shot if any.
	 */
	UFUNCTION(BlueprintPure, Category = "LIV|Shot")
		ULivShotComponent* GetCurrentShot() const;

	/**
	 * Switch to another shot and call any relevant events.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV|Shot")
		void SetCurrentShot(ULivShotComponent* ShotComponent);

	/**
	 * How long the current shot has been active.
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "LIV|Shot")
		float CurrentShotTime;

	/**
	 * Tick the current shot.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV|Shot")
		virtual void TickCurrentShot(float DeltaTime);

	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
