// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLivAuthentication, Log, All);

/**
 * 
 */
class LIVEDITOR_API FLivAuthentication
{
public:

	enum class EState : uint8
	{
		Initializing,
		RequestToken,
		WaitForRequestTokenResponse,
		WaitForLaunchBrowserInput,
		LaunchBrowserAuthFlow,
		DelayCheckAuthFlow,
		SendCheckAuthFlowRequest,
		WaitingForAuthCheckResponse,
		AuthSuccess,

		RequestPortalSession,
		WaitForPortalSessionResponse,
		LaunchBrowserPortalSession,

		Failure,
		Timeout
	};

	DECLARE_DELEGATE_ThreeParams(FOnTokenRequestSuccess, const FString& PortalToken, const FString& Url, const int& ExpiresIn);

	void Initialize();

	void SetRequestToken(const FString& NewRequestToken);
	FString GetRequestToken() const;

	void SetPortalToken(const FString& NewPortalToken);
	FString GetPortalToken() const;
	
	
	void UpdateState(float DeltaTime);

private:
	FString RequestToken;
	FString PortalToken;
	FString AuthFlowUrl;
	FString ErrorReason;
	float Timeout{ 0.0f };
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreatePortalTokenRequest() const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateCheckAuthFlowRequest() const;

	void SendPortalTokenRequest();
	void SendCheckAuthFlowRequest();

	FString GetUserSettingsDir() const;
	FString GetPortalTokenFilePath() const;

	void LaunchBrowserAuthFlow();

	EState State { EState::Initializing };

	friend class SLivAuthWidget;
};
