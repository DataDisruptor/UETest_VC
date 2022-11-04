#include "LivThirdPersonShotActor.h"

#include "LivShotComponent.h"
#include "LivSpringArmComponent.h"
#include "LivWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ALivThirdPersonShotActor::ALivThirdPersonShotActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<ULivSpringArmComponent>("SpringArm");
	ShotComponent = CreateDefaultSubobject<ULivShotComponent>("ShotComponent");

	ShotComponent->SetupAttachment(SpringArm);

	ShotComponent->TickShotEvent.AddDynamic(this, &ALivThirdPersonShotActor::TickSpringArm);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ALivThirdPersonShotActor::TickSpringArm(ALivCameraController* Controller, ULivCaptureBase* CaptureComponent, float ShotTime, float DeltaTime)
{
	const UWorld* World = GetWorld();
	if(World)
	{
		const ULivWorldSubsystem* LivWorldSubsystem = World->GetSubsystem<ULivWorldSubsystem>();
		if(LivWorldSubsystem)
		{
			UCameraComponent* LocalPlayerCamera = LivWorldSubsystem->GetPlayerCamera();

			if (LocalPlayerCamera)
			{
				SpringArm->UpdatePoseForCamera(LocalPlayerCamera, DeltaTime);
			}
		}
	}
}
