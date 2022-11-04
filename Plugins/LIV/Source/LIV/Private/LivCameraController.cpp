// Copyright 2021 LIV Inc. - MIT License
#include "LivCameraController.h"

#include "LivCaptureBase.h"
#include "LivShotComponent.h"
#include "LivWorldSubsystem.h"
#include "Engine/World.h"

ALivCameraController::ALivCameraController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentShot(nullptr)
	, CurrentShotTime(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

ULivShotComponent* ALivCameraController::GetCurrentShot() const
{
	return CurrentShot;
}

void ALivCameraController::SetCurrentShot(ULivShotComponent* ShotComponent)
{
	if (CurrentShot)
	{
		CurrentShot->OnCutFrom(this);
	}

	CurrentShot = ShotComponent;

	// could be setting the shot component to nullptr to clear it so check here
	if (CurrentShot)
	{
		CurrentShot->OnCutTo(this);
	}

	CurrentShotTime = 0.0f;
}

void ALivCameraController::TickCurrentShot(float DeltaTime)
{
	ULivWorldSubsystem* LivWorldSubsystem = GetWorld()->GetSubsystem<ULivWorldSubsystem>();

	if (!LivWorldSubsystem)
	{
		return;
	}

	ULivCaptureBase* CaptureComponent = LivWorldSubsystem->GetCaptureComponent();

	if (CaptureComponent)
	{
		if (CurrentShot)
		{
			CurrentShot->TickShot(this, CaptureComponent, CurrentShotTime, DeltaTime);

			CaptureComponent->bOverrideCameraPose = CurrentShot->bOverrideCamera;

			if (CurrentShot->bOverrideCamera)
			{
				CaptureComponent->SetWorldTransform(CurrentShot->GetComponentTransform());
				CaptureComponent->FOVAngle = CurrentShot->FOVAngle;
			}

			CurrentShotTime += DeltaTime;
		}
		else
		{
			CaptureComponent->bOverrideCameraPose = false;
		}
	}
}

void ALivCameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickCurrentShot(DeltaTime);
}

void ALivCameraController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// to avoid bad state, disable overriding camera pose on end play

	ULivWorldSubsystem* LivWorldSubsystem = GetWorld()->GetSubsystem<ULivWorldSubsystem>();

	if (!LivWorldSubsystem)
	{
		return;
	}

	ULivCaptureBase* CaptureComponent = LivWorldSubsystem->GetCaptureComponent();

	if (CaptureComponent)
	{
		CaptureComponent->bOverrideCameraPose = false;
	}
}

