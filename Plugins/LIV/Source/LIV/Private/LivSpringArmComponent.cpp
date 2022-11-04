#include "LivSpringArmComponent.h"
#include "Camera/CameraComponent.h"

ULivSpringArmComponent::ULivSpringArmComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bClampPitch(true)
	, MinPitch(-10.0f)
	, MaxPitch(10.0f)
	, YawDeltaThreshold(10.0f)
	, MaxYawDelta(100.0f)
	, MinInterpolationSpeed(0.5f)
	, MaxInterpolationSpeed(12.0f)
{
	TargetArmLength = 200.0f;
	SocketOffset.Y = 40.0f;
}

void ULivSpringArmComponent::UpdatePoseForCamera(UCameraComponent* Camera, float DeltaTime)
{
	if(!Camera)
	{
		return;
	}

	// @TODO: Potentially add smoothing options for location
	const FVector DesiredLocation = Camera->GetComponentLocation();
	SetWorldLocation(DesiredLocation);

	// Handle rotation
	const FRotator CurrentRotation = GetComponentRotation();
	const FRotator DesiredRotation = Camera->GetComponentRotation();

	const float YawDifference = FMath::Abs(DesiredRotation.Yaw - CurrentRotation.Yaw);
	
	// @TODO: Maybe allow roll, but clamped?
	const FRotator TargetRotation(FMath::Clamp(DesiredRotation.Pitch, MinPitch, MaxPitch), YawDifference > YawDeltaThreshold ? DesiredRotation.Yaw : CurrentRotation.Yaw, 0.0f);

	const float InterpSpeed = FMath::Clamp(YawDifference / MaxYawDelta, 0.0f, 1.0f);

	const FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);

	SetWorldRotation(InterpolatedRotation);
}
