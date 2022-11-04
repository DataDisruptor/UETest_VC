// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivCaptureActor.generated.h"

/**
 * @TODO: Will likely switch to spawning an actor to capture instead of just free
 * floating scene capture components (that need to spawn other components)
 */
UCLASS(Abstract)
class LIV_API ALivCaptureActor : public AActor
{
	GENERATED_BODY()

public:

	ALivCaptureActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = "LIV", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USceneCaptureComponent2D* PrimaryCaptureComponent;

public:

	class USceneCaptureComponent2D* GetPrimaryCaptureComponent() const { return PrimaryCaptureComponent; }

	virtual void Tick(float DeltaTime) override;

};
