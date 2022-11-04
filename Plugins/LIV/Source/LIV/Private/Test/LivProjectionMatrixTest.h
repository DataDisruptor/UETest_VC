// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LivProjectionMatrixTest.generated.h"

UCLASS()
class ALivProjectionMatrixTest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALivProjectionMatrixTest();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LIV|Debug")
		class ASceneCapture2D* SceneCaptureActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LIV|Debug")
		class USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FMatrix ProjectionMatrix;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ProjectionMatrixRow0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ProjectionMatrixRow1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ProjectionMatrixRow2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ProjectionMatrixRow3;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 LivProjectionMatrixRow0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 LivProjectionMatrixRow1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 LivProjectionMatrixRow2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 LivProjectionMatrixRow3;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ManualProjectionMatrixRow0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ManualProjectionMatrixRow1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ManualProjectionMatrixRow2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "LIV|Debug")
		FVector4 ManualProjectionMatrixRow3;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
