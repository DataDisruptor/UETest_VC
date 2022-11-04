// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "LivClipPlane.generated.h"

class UMaterialInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIV_API ULivClipPlane : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	
	ULivClipPlane(const FObjectInitializer& ObjectInitializer);

public:

	UPROPERTY(EditInstanceOnly, Category = "LIV")
		UMaterialInterface* ClipPlaneMaterial;

	UPROPERTY(EditAnywhere, Category = "LIV")
		UMaterialInterface* ClipPlaneDebugMaterial;

public:

	UFUNCTION(BlueprintCallable, Category = "LIV")
		void SetDebugEnabled(bool bDebugEnabled);

	UFUNCTION(BlueprintPure, Category = "LIV")
		bool GetDebugEnabled() const;
		
};
