// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LivWorldSubsystem.generated.h"

class UCameraComponent;
class ULivCaptureBase;
DECLARE_LOG_CATEGORY_EXTERN(LogLivWorldSubsystem, Log, Log);

/**
 * 
 */
UCLASS()
class LIV_API ULivWorldSubsystem : public UWorldSubsystem
{
public:
	

	GENERATED_BODY()

	ULivWorldSubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "LIV")
		TSubclassOf<ULivCaptureBase> GetCaptureComponentClass() const;

	UFUNCTION(BlueprintPure, Category = "LIV")
		FTransform GetTrackingOriginTransform() const;

	UFUNCTION(BlueprintPure, Category = "LIV")
		ULivCaptureBase* GetCaptureComponent() const { return CaptureComponent; }

	UFUNCTION(BlueprintPure, Category = "LIV")
		USceneComponent* GetCameraRoot() const { return CameraRoot; }

	UFUNCTION(BlueprintPure, Category = "LIV")
		UCameraComponent* GetPlayerCamera() const;

	UFUNCTION(BlueprintPure, Category = "LIV")
		USceneComponent* GetPlayerCameraParent() const;

	void Capture(struct FLivCaptureContext& Context);

	void CreateCaptureResources();

	void DestroyCaptureResources();

private:

	void HandleTrackingOrigin();

	void Tick();

	UPROPERTY(Transient)
		USceneComponent* CameraRoot;
	
	UPROPERTY(Transient)
		ULivCaptureBase* CaptureComponent;

	UPROPERTY(Transient)
		USceneComponent* TrackingOriginComponent;

	UPROPERTY(Transient)
		class ALivCameraController* CameraController;

	friend class ULivLocalPlayerSubsystem;
};
