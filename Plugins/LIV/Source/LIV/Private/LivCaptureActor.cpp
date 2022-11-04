// Copyright 2021 LIV Inc. - MIT License

#include "LivCaptureActor.h"
#include "Components/SceneCaptureComponent2D.h"

ALivCaptureActor::ALivCaptureActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NewSceneCaptureComponent2D"));
	PrimaryCaptureComponent->SetupAttachment(RootComponent);
}

void ALivCaptureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

