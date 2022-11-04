// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"

#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "UObject/NoExportTypes.h"
#include "LivCaptureContext.generated.h"

/**
 * @TODO: Deprecate this ideally
 */
USTRUCT(BlueprintType)
struct FLivCaptureContext
{
	GENERATED_BODY()

	/** The components won't rendered when LIV captures the scene. */
	UPROPERTY(Transient)
		TArray<TWeakObjectPtr<UPrimitiveComponent>> HiddenComponents;

	/** These actors won't be rendered when LIV captures the scene. */
	UPROPERTY(Transient)
		TArray<TWeakObjectPtr<AActor>> HiddenActors;

	void ApplyHideLists(USceneCaptureComponent2D* Component) const
	{
		Component->HiddenComponents = HiddenComponents;
		Component->HiddenActors.Reset(HiddenActors.Num());
		for (auto It = HiddenActors.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				Component->HiddenActors.Add(It->Get());
			}
		}
	}
};