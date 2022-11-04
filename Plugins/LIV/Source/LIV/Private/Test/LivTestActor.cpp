// Copyright 2021 LIV Inc. - MIT License
#include "LivTestActor.h"

ALivTestActor::ALivTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALivTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FTransform t = GetActorTransform();
	const auto m = t.ToMatrixWithScale();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *(m.ToString()) );

	const auto q = t.Rotator().Quaternion();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *(q.ToString()));
}

