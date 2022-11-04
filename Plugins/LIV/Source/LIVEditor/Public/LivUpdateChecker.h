// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * 
 */
class LIVEDITOR_API FLivUpdateChecker
{
public:

	struct FVersionInfo
	{
		FString EngineMajorMinorVersion;
		FString PluginVersion;
	};

	void Initialize();

	bool IsPending() const;
	bool CheckedSuccessfully() const;
	TArray<FVersionInfo> GetVersionInfo() const;
	bool IsUpdateAvailable() const;
	bool IsCurrentVersionSupported() const;
	FString GetCurrentPluginVersion() const;
	FString GetCurrentEngineMajorMinorVersion() const;
	FVersionInfo GetVersionInfoForThisEngine() const;

private:

	enum class EState : uint8
	{
		Initializing,
		WaitingForCheckResponse,
		Success,
		Failure
	};

	float Timeout{ 10.0f };
	EState State{ EState::Initializing };
	TArray<FVersionInfo> VersionInfo;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateUpdateCheckRequest() const;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;

	friend class SLivUpdateChecker;
};
