// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivTestActor.generated.h"

/**
 * Simple test actor to log transform data.
 */
UCLASS()
class LIV_API ALivTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALivTestActor();

	virtual void Tick(float DeltaTime) override;
};
