// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LivCaptureContext.h"
#include "Engine/LocalPlayer.h"
#include "LivLocalPlayerSubsystem.generated.h"

class ULivWorldSubsystem;
class ULivCaptureBase;

DECLARE_LOG_CATEGORY_EXTERN(LogLivLocalPlayerSubsystem, Log, Log);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLivLocalPlayerActivationDelegate);

/**
 * @TODO: Get rid of this
 */
UCLASS()
class LIV_API ULivLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
public:

	GENERATED_BODY()

	ULivLocalPlayerSubsystem();
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, meta=(DisplayName="GetLocalPlayer"), Category = "LIV")
		ULocalPlayer* GetLocalPlayerBP() const { return GetLocalPlayer<ULocalPlayer>(); }

	UFUNCTION(BlueprintPure, Category = "LIV")
		ULivWorldSubsystem* GetLivWorldSubsystem() const;

	UFUNCTION(BlueprintPure, Category = "LIV")
		TSubclassOf<ULivCaptureBase> GetCaptureComponentClass() const;

	/**
	 * True is LIV has requested capture and we have acknowledge and setup
	 * the necessary resources for capture.
	 */
	UFUNCTION(BlueprintPure, Category = "LIV")
		bool IsCaptureActive() const;

	/**
	 * Shutdown LIV capture and try to reconnect and recreate capture resources.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void ResetCapture();

	/**
	 * Hide a component from rendering during LIV scene capture.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void HideComponent(UPrimitiveComponent* InComponent);

	/**
	 * Stop hiding a component previously hidden with HideComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void ShowComponent(UPrimitiveComponent* InComponent);

	/**
	 * Clear components previously hidden with HideComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void ClearHiddenComponents();

	/**
	 * Hide an actor from rendering during LIV scene capture.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void HideActor(AActor* InActor);

	/**
	 * Stop hiding an actor previously hidden with HideComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void ShowActor(AActor* InActor);

	/**
	 * Clear actors previously hidden with HideComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "LIV")
		void ClearHiddenActors();

	UFUNCTION(BlueprintPure, Category = "LIV")
		FLivCaptureContext GetCaptureContext() const;

public:

	UPROPERTY(BlueprintAssignable, Category = "LIV")
		FLivLocalPlayerActivationDelegate OnCaptureActivated;

	UPROPERTY(BlueprintAssignable, Category = "LIV")
		FLivLocalPlayerActivationDelegate OnCaptureDeactivated;

	UPROPERTY(Transient)
		FLivCaptureContext CaptureContext;

protected:
	
	void HandleLivConnection();
	void LivCaptureActivated();
	void LivCaptureDeactivated();
	bool Tick(float DeltaTime);

private:
	
	FDelegateHandle TickHandle;
	bool bLivActive {false};
};
