#include "LivShotComponent.h"

ULivShotComponent::ULivShotComponent()
	: Super()
	, Score(0.5f)
	, FOVAngle(90.0f)
	, bOverrideCamera(true)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}

void ULivShotComponent::TickShot_Implementation(ALivCameraController* Controller, ULivCaptureBase* CaptureComponent, float ShotTime, float DeltaTime)
{
	TickShotEvent.Broadcast(Controller, CaptureComponent, ShotTime, DeltaTime);
}

void ULivShotComponent::OnCutTo_Implementation(ALivCameraController* Controller)
{
	CutToEvent.Broadcast(Controller);
}

void ULivShotComponent::OnCutFrom_Implementation(ALivCameraController* Controller)
{
	CutFromEvent.Broadcast(Controller);
}