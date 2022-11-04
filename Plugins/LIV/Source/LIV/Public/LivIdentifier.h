// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LivIdentifier.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct LIV_API FLivIdentifier
{
	GENERATED_BODY()

	// Name of engine
	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		FString EngineName;

	// Version of engine
	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		FString EngineVersion;

	// Liv plugin version
	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		FString ClientVersion;

	// Liv native SDK version
	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		FString NativeClientVersion;

	bool operator==(const FLivIdentifier& Identifier) const
	{
		return EngineName.Equals(Identifier.EngineName)
			&& EngineVersion.Equals(Identifier.EngineVersion)
			&& ClientVersion.Equals(Identifier.ClientVersion)
			&& NativeClientVersion.Equals(Identifier.NativeClientVersion);
	}
};
