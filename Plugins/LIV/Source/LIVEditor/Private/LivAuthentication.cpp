// Copyright 2021 LIV Inc. - MIT License


#include "LivAuthentication.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "LivEditorAnalytics.h"
#include "Async/Async.h"
#include "Launch/Resources/Version.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogLivAuthentication);

// @TODO: change to production URL
namespace LivAuthentication
{
	const FString APIServer(TEXT("https://dev.liv.tv"));
}

void FLivAuthentication::Initialize()
{
	const FString UserSettingsDir = GetUserSettingsDir();
	if (!FPaths::DirectoryExists(UserSettingsDir))
	{
		if (!IFileManager::Get().MakeDirectory(*UserSettingsDir, false))
		{
			UE_LOG(LogLivAuthentication, Warning, TEXT("Failed to create LIV user settings directory: %s"), *UserSettingsDir);
			return;
		}
	}

	const FString TokenFile = GetPortalTokenFilePath();
	if (FPaths::FileExists(TokenFile))
	{
		if (!FFileHelper::LoadFileToString(PortalToken, *TokenFile))
		{
			UE_LOG(LogLivAuthentication, Warning, TEXT("Failed to load LIV token: %s"), *TokenFile);
		}
	}
	else
	{
		UE_LOG(LogLivAuthentication, Log, TEXT("No LIV token available yet."));
	}
}

void FLivAuthentication::SetRequestToken(const FString& NewRequestToken)
{
	RequestToken = NewRequestToken;
}

FString FLivAuthentication::GetRequestToken() const
{
	return RequestToken;
}

void FLivAuthentication::SetPortalToken(const FString& NewPortalToken)
{
	PortalToken = NewPortalToken;

	const FString TokenFile = GetPortalTokenFilePath();
	if (!FFileHelper::SaveStringToFile(PortalToken, *TokenFile))
	{
		UE_LOG(LogLivAuthentication, Warning, TEXT("Failed to write LIV token: %s"), *TokenFile);
	}
}

FString FLivAuthentication::GetPortalToken() const
{
	return PortalToken;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLivAuthentication::CreatePortalTokenRequest() const
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");

	FString Payload;
	const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload);

	JsonWriter->WriteObjectStart();
	{
		JsonWriter->WriteValue(TEXT("engine_type"), TEXT("UNREAL"));
		JsonWriter->WriteValue("engine_version", ENGINE_VERSION_STRING);
		JsonWriter->WriteValue(TEXT("device_name"), *FPlatformMisc::GetLoginId());

		// set tracking id if we already have one setup
		if(FLivEditorAnalytics::HasValidFingerprintId())
		{
			JsonWriter->WriteValue(TEXT("tracking_id"), *FLivEditorAnalytics::GetFingerprintId());
		}
	}
	JsonWriter->WriteObjectEnd();

	JsonWriter->Close();

	const auto HttpRequest = HttpModule.CreateRequest();

	const FString URLPath(TEXT("api/auth/sdk"));

	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetURL(LivAuthentication::APIServer / URLPath);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetContentAsString(Payload);


	return HttpRequest;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLivAuthentication::CreateCheckAuthFlowRequest() const
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");

	const auto HttpRequest = HttpModule.CreateRequest();

	const FString URLPath(TEXT("api/auth/sdk"));

	HttpRequest->SetURL(LivAuthentication::APIServer / URLPath / GetRequestToken());
	HttpRequest->SetVerb(TEXT("GET"));

	return HttpRequest;
}


bool ParsePortalToken(const FString& ResponseText, FString& RequestToken, FString& Url, int32& ExpiresIn)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseText);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* JsonResult;
	if (!JsonObject->TryGetObjectField(TEXT("result"), JsonResult))
	{
		return false;
	}

	if (!(*JsonResult)->TryGetStringField(TEXT("request_token"), RequestToken))
	{
		return false;
	}

	if (!(*JsonResult)->TryGetStringField(TEXT("url"), Url))
	{
		return false;
	}

	if (!(*JsonResult)->TryGetNumberField(TEXT("expires_in"), ExpiresIn))
	{
		return false;
	}

	return true;
}

bool ParseAuthCheckResponse(const FString& ResponseText, bool& bCompleted, FString& PortalToken, FString& TrackingId, int32& ExpiresIn)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseText);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* JsonResult;
	if (!JsonObject->TryGetObjectField(TEXT("result"), JsonResult))
	{
		return false;
	}

	if(!(*JsonResult)->TryGetBoolField(TEXT("completed"), bCompleted))
	{
		return false;
	}

	// if not completed then we just need expires_in to finish parsing correctly then can return 
	if(!bCompleted)
	{
		if (!(*JsonResult)->TryGetNumberField(TEXT("expire_in"), ExpiresIn))
		{
			return false;
		}

		return true;
	}

	// if completed we should get tracking id and portal token
	if (!(*JsonResult)->TryGetStringField(TEXT("tracking_id"), TrackingId))
	{
		return false;
	}

	if (!(*JsonResult)->TryGetStringField(TEXT("portal_token"), PortalToken))
	{
		return false;
	}

	return true;
}

void FLivAuthentication::SendPortalTokenRequest()
{
	Request = CreatePortalTokenRequest();
	Request->OnProcessRequestComplete().BindLambda([&](
		FHttpRequestPtr RequestPtr,
		FHttpResponsePtr ResponsePtr,
		bool bConnectedSuccessfully)
		{
			if (bConnectedSuccessfully && ResponsePtr->GetResponseCode() == 200)
			{
				const FString ResponseText = ResponsePtr->GetContentAsString();

				FString RequestToken;
				FString Url;
				int32 ExpiresIn;
				if (ParsePortalToken(ResponseText, RequestToken, Url, ExpiresIn))
				{
					AsyncTask(ENamedThreads::GameThread, [this, RequestToken, Url, ExpiresIn]()
						{
							SetRequestToken(RequestToken);
							AuthFlowUrl = Url;
							Timeout = static_cast<int32>(ExpiresIn);
							State = EState::WaitForLaunchBrowserInput;
						});
				}
				else
				{
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							ErrorReason = TEXT("Failed to parse portal token.");
							State = EState::Failure;
						});
				}
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						ErrorReason = TEXT("Failed to connect to LIV API.");
						State = EState::Failure;
					});
			}
		});

	Request->ProcessRequest();
	Timeout = 10.0f;
	State = EState::WaitForRequestTokenResponse;
}

void FLivAuthentication::SendCheckAuthFlowRequest()
{
	Request = CreateCheckAuthFlowRequest();
	Request->OnProcessRequestComplete().BindLambda([&](
		FHttpRequestPtr RequestPtr,
		FHttpResponsePtr ResponsePtr,
		bool bConnectedSuccessfully)
		{
			if (bConnectedSuccessfully && ResponsePtr->GetResponseCode() == 200)
			{
				const FString ResponseText = ResponsePtr->GetContentAsString();

				bool bCompleted;
				FString PortalToken;
				FString TrackingId;
				int32 ExpiresIn;

				if (ParseAuthCheckResponse(ResponseText, bCompleted, PortalToken, TrackingId, ExpiresIn))
				{
					AsyncTask(ENamedThreads::GameThread, [this, bCompleted, PortalToken, TrackingId, ExpiresIn]()
						{
							if(!bCompleted)
							{
								constexpr int32 Delay = 1.0f;
								const float Expires = static_cast<int32>(ExpiresIn);
								if(Expires < Delay)
								{
									State = EState::Timeout;
									ErrorReason = TEXT("Timed out checking auth response.");
								}
								else
								{
									State = EState::DelayCheckAuthFlow;
									Timeout = Delay;
								}
							}
							else
							{
								SetPortalToken(PortalToken);
								if (TrackingId != FLivEditorAnalytics::GetFingerprintId())
								{
									FLivEditorAnalytics::SetFingerprintId(TrackingId);
								}
								Timeout = static_cast<int32>(ExpiresIn);
								State = EState::AuthSuccess;
							}
						});
				}
				else
				{
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							ErrorReason = TEXT("Failed to parse auth flow response.");
							State = EState::Failure;
						});
				}
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						ErrorReason = TEXT("Failed to connect to LIV API.");
						State = EState::Failure;
					});
			}
		});

	Request->ProcessRequest();
	Timeout = 10.0f;
	State = EState::WaitingForAuthCheckResponse;
}

void FLivAuthentication::UpdateState(float DeltaTime)
{
	const auto IsTimeoutExpired = [this]() { return Timeout <= 0.0f; };

	switch (State)
	{
	case EState::Initializing:
		if (GetPortalToken().IsEmpty() || !FLivEditorAnalytics::HasValidFingerprintId())
		{
			State = EState::RequestToken;
		}
		else
		{
			// @todo: I guess nothing to do in this case?
			State = EState::AuthSuccess;
		}
		break;
	case EState::RequestToken:
		SendPortalTokenRequest();
		break;
	case EState::WaitForRequestTokenResponse:
		Timeout -= DeltaTime;
		if(IsTimeoutExpired())
		{
			// @note: **should** call OnProcessRequestComplete
			Request->CancelRequest();
			ErrorReason = TEXT("Request token response expired.");
			State = EState::Failure;
		}
		break;
	case EState::WaitForLaunchBrowserInput:
		Timeout -= DeltaTime;
		if(IsTimeoutExpired())
		{
			ErrorReason = TEXT("Timed out waiting for user to launch browser.");
			State = EState::Failure;
		}
		break;
	case EState::LaunchBrowserAuthFlow:
		// launch browser for user
		LaunchBrowserAuthFlow();
		// wait at least 1 second before checking if they've completed the in browser flow
		Timeout = 1.0f;
		State = EState::DelayCheckAuthFlow;
		break;
	case EState::DelayCheckAuthFlow:
		Timeout -= DeltaTime;
		if(IsTimeoutExpired())
		{
			State = EState::SendCheckAuthFlowRequest;
		}
		break;
	case EState::SendCheckAuthFlowRequest:
		SendCheckAuthFlowRequest();
		break;
	case EState::WaitingForAuthCheckResponse:
		Timeout -= DeltaTime;
		if(IsTimeoutExpired())
		{
			// @todo: we could retry a few more times tbh here..
			ErrorReason = TEXT("Timed out waiting for response for auth check");
			State = EState::Failure;
		}
		break;
	case EState::AuthSuccess:
		// No-Op
		if (GetPortalToken().IsEmpty() || !FLivEditorAnalytics::HasValidFingerprintId())
		{
			State = EState::Initializing;
		}
		break;
	case EState::Failure:
		// No-Op (?)
		break;
	case EState::RequestPortalSession: check(false); break;
	case EState::WaitForPortalSessionResponse: check(false); break;
	case EState::LaunchBrowserPortalSession: check(false); break;
	case EState::Timeout: break;
	default: ;
	}

}

FString FLivAuthentication::GetUserSettingsDir() const
{
	return FPaths::Combine(FPlatformProcess::UserSettingsDir(), TEXT("LIVUnrealSDK"));
}

FString FLivAuthentication::GetPortalTokenFilePath() const
{
	return FPaths::Combine(GetUserSettingsDir(), TEXT("portal.token"));
}

void FLivAuthentication::LaunchBrowserAuthFlow()
{
	FPlatformProcess::LaunchURL(*AuthFlowUrl, nullptr, nullptr);
}

