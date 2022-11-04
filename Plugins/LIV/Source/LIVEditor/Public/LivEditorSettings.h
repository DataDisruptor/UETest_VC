// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "LivEditorSettings.generated.h"

/**
 * Liv editor plugin settings
 */
UCLASS(config = Engine)
class LIVEDITOR_API ULivEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	
	ULivEditorSettings();

	UPROPERTY(config, EditAnywhere, Category = "Liv")
	bool bAnalyticsEnabled;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
