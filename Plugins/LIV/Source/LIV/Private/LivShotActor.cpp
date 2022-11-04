#include "LivShotActor.h"
#include "LivShotComponent.h"

ALivShotActor::ALivShotActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	// Make the scene component the root component
	RootComponent = SceneComponent;

	ShotComponent = CreateDefaultSubobject<ULivShotComponent>("ShotComponent");
	ShotComponent->SetupAttachment(RootComponent);
}

