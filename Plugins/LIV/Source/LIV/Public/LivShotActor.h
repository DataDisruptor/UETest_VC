// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivShotActor.generated.h"

/**
 * Convenience actor that already have a shot component in it.
 * Rely on the shot component for interfaces rather than this actor as the shot
 * component could be placed in any actor.
 */
UCLASS(BlueprintType, Blueprintable, ComponentWrapperClass)
class LIV_API ALivShotActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	ALivShotActor(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "LIV|Shot")
		class USceneComponent* SceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LIV|Shot")
		class ULivShotComponent* ShotComponent;
};
