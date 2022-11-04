// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "HttpModule.h"
#include "HttpRetrySystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "Interfaces/IHttpResponse.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "LIV.h"
#include "Windows/HideWindowsPlatformTypes.h"

class FWaitForHttpRequestLatentCommand : public IAutomationLatentCommand \
{
public:
	FWaitForHttpRequestLatentCommand(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest, double InTimeout)
		: Request(InRequest)
		, Timeout(InTimeout)
	{
		Request->ProcessRequest();
	}
	virtual ~FWaitForHttpRequestLatentCommand() {}
	virtual bool Update() override
	{
		const double NewTime = FPlatformTime::Seconds();

		if (NewTime - StartTime >= Timeout)
		{
			return true;
		}

		switch(Request->GetStatus())
		{
		case EHttpRequestStatus::Succeeded:
		case EHttpRequestStatus::Failed:
		case EHttpRequestStatus::Failed_ConnectionError:
			return true;
		default:
			return false;
		}
	}
private:
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request;
	double Timeout;
};

