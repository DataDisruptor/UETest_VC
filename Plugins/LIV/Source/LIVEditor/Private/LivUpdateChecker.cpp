// Copyright 2021 LIV Inc. - MIT License


#include "LivUpdateChecker.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "Async/Async.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/EngineVersion.h"
#include "Misc/EngineVersionBase.h"

bool ParseUpdateCheckResponse(const FString& ResponseText, TArray<FLivUpdateChecker::FVersionInfo>& Infos)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseText);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}
	
	const TArray<TSharedPtr<FJsonValue>>* Results;
	if(!(*JsonObject).TryGetArrayField(TEXT("result"), Results))
	{
		return false;
	}

	for(const auto& Result : *Results)
	{
		const TSharedPtr<FJsonObject>* ResultObject;
		if(!Result->TryGetObject(ResultObject))
		{
			return false;
		}

		FString Type;
		if(!(*ResultObject)->TryGetStringField(TEXT("type"), Type))
		{
			return false;
		}

		const FString UnrealType(TEXT("UNREAL"));
		if(Type != UnrealType)
		{
			// ignore other engines
			continue;
		}

		FString EngineMajorMinorVersion;
		if(!(*ResultObject)->TryGetStringField(TEXT("version"), EngineMajorMinorVersion))
		{
			return false;
		}
		
		FString PluginVersion;
		if (!(*ResultObject)->TryGetStringField(TEXT("plugin_version"), PluginVersion))
		{
			return false;
		}

		Infos.Add({ EngineMajorMinorVersion, PluginVersion });
	}

	return true;
}

void FLivUpdateChecker::Initialize()
{
	Request = CreateUpdateCheckRequest();
	Request->OnProcessRequestComplete().BindLambda([&](
		FHttpRequestPtr RequestPtr,
		FHttpResponsePtr ResponsePtr,
		bool bConnectedSuccessfully)
		{
			if (bConnectedSuccessfully && ResponsePtr->GetResponseCode() == 200)
			{
				const FString ResponseText = ResponsePtr->GetContentAsString();

				TArray<FVersionInfo> Info;
				Info.Sort([](const FVersionInfo& A, const FVersionInfo& B) { return A.EngineMajorMinorVersion < B.EngineMajorMinorVersion; });
				if(ParseUpdateCheckResponse(ResponseText, Info))
				{
					AsyncTask(ENamedThreads::GameThread, [this, Info]()
						{
							VersionInfo = Info;
							State = EState::Success;
						});
				}
				else
				{
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							State = EState::Failure;
						});
				}
				
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						State = EState::Failure;
					});
			}
		}
	);

	Request->ProcessRequest();
	Timeout = 10.0f;
	State = EState::WaitingForCheckResponse;
}

bool FLivUpdateChecker::IsPending() const
{
	return State == EState::WaitingForCheckResponse;
}

bool FLivUpdateChecker::CheckedSuccessfully() const
{
	return State == EState::Success;
}

TArray<FLivUpdateChecker::FVersionInfo> FLivUpdateChecker::GetVersionInfo() const
{
	return VersionInfo;
}

bool FLivUpdateChecker::IsUpdateAvailable() const
{
	const FString ThisEngineMajorMinorVersion = GetCurrentEngineMajorMinorVersion();

	for(const auto& Info : VersionInfo)
	{
		if(Info.EngineMajorMinorVersion == ThisEngineMajorMinorVersion)
		{
			const FString ThisPluginVersionStr = GetCurrentPluginVersion();

			FEngineVersion ThisPluginVersion;
			if(!FEngineVersion::Parse(ThisPluginVersionStr, ThisPluginVersion))
			{
				return false;
			}

			FEngineVersion LatestPluginVersion;
			if (!FEngineVersion::Parse(Info.PluginVersion, LatestPluginVersion))
			{
				return false;
			}

			// highjack engine version code rather than rewriting this
			const auto NewestVersion = FEngineVersionBase::GetNewest(LatestPluginVersion, ThisPluginVersion, nullptr);

			return NewestVersion == EVersionComparison::First;
		}
	}

	return false;
}

bool FLivUpdateChecker::IsCurrentVersionSupported() const
{
	// @todo: check deprecation field
	const FString ThisEngineMajorMinorVersion = GetCurrentEngineMajorMinorVersion();

	for (const auto& Info : VersionInfo)
	{
		if (Info.EngineMajorMinorVersion == ThisEngineMajorMinorVersion)
		{
			return true;
		}
	}

	return false;
}

FString FLivUpdateChecker::GetCurrentPluginVersion() const
{
	const auto LivPlugin = IPluginManager::Get().FindPlugin(TEXT("Liv"));
	const FString ThisPluginVersionStr = LivPlugin->GetDescriptor().VersionName;

	return ThisPluginVersionStr;
}

FString FLivUpdateChecker::GetCurrentEngineMajorMinorVersion() const
{
	return FEngineVersion::Current().ToString(EVersionComponent::Minor);
}

FLivUpdateChecker::FVersionInfo FLivUpdateChecker::GetVersionInfoForThisEngine() const
{
	const FString ThisEngineMajorMinorVersion = GetCurrentEngineMajorMinorVersion();

	for (const auto& Info : VersionInfo)
	{
		if (Info.EngineMajorMinorVersion == ThisEngineMajorMinorVersion)
		{
			return Info;
		}
	}

	return FVersionInfo{};
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLivUpdateChecker::CreateUpdateCheckRequest() const
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");

	const auto HttpRequest = HttpModule.CreateRequest();

	const FString ApiServer(TEXT("https://dev.liv.tv"));
	const FString URLPath(TEXT("api/sdks"));

	HttpRequest->SetURL(ApiServer / URLPath);
	HttpRequest->SetVerb(TEXT("GET"));

	return HttpRequest;
}

