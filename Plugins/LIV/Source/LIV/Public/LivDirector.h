#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivCameraController.h"
#include "LivDirector.generated.h"

class ULivShotComponent;

/**
 * A camera controller that tries to cut between different shots.
 * Experimental, may be removed or refactored.
 */
UCLASS(Experimental)
class LIV_API ALivDirector : public ALivCameraController
{
	GENERATED_BODY()
	
public:	
	ALivDirector(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Transient, Category = "LIV|Shot")
		TArray<ULivShotComponent*> Shots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LIV|Shot")
		float MaxShotLength;

protected:

	void SortShots();

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "LIV|Shot")
		void FindShots();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LIV|Shot")
		void Cut();

};
