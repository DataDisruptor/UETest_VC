// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "LivBlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "LivImagePlaneTest.generated.h"

UCLASS()
class ALivImagePlaneTest : public AActor
{
	GENERATED_BODY()
	
public:	

	ALivImagePlaneTest();

	UPROPERTY(EditAnywhere, Category = "LIV | Test")
		bool bDrawSphere;

	UPROPERTY(EditAnywhere, Category = "LIV | Test")
		ELivEye Eye;

	UPROPERTY(EditAnywhere, Category = "LIV | Test")
		FVector RayStart;

	UPROPERTY(EditAnywhere, Category = "LIV | Test")
		FVector RayEnd;

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
		class UArrowComponent* ArrowComponent;
#endif

	virtual void Tick(float DeltaTime) override;
};
