#include "LivDirector.h"
#include "LivShotComponent.h"
#include "UObject/UObjectIterator.h"

ALivDirector::ALivDirector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxShotLength(2.5f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ALivDirector::SortShots()
{
	Shots = Shots.FilterByPredicate([](const ULivShotComponent* Shot) ->bool { return Shot->IsActive(); });
	Shots.Sort([](const ULivShotComponent& A, const ULivShotComponent& B) {return A.Score > B.Score; });
}

void ALivDirector::Tick(float DeltaTime)
{
	if(!CurrentShot || !CurrentShot->IsActive())
	{
		Cut();
	}

	if (CurrentShotTime > MaxShotLength)
	{
		Cut();
	}

	Super::Tick(DeltaTime);
}

void ALivDirector::FindShots()
{
	Shots.Empty();

	const UWorld* World = GetWorld();
	for(TObjectIterator<ULivShotComponent> It; It; ++It)
	{
		if(It->GetWorld() == World)
		{
			Shots.Add(*It);
		}
	}
}

void ALivDirector::Cut_Implementation()
{
	FindShots();
	SortShots();

	ULivShotComponent* NewCurrentShot = Shots.Num() > 0 ? Shots[0] : nullptr;
	SetCurrentShot(NewCurrentShot);
}

