// Copyright 2021 LIV Inc. - MIT License
#include "LivEditorTestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLivAuthTest, "Liv.Editor.Auth Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLivAuthTest::RunTest(const FString& Parameters)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");

	FString Payload;
	const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload);

	JsonWriter->WriteObjectStart();
	{
		JsonWriter->WriteValue(TEXT("engine_type"), TEXT("UNREAL"));
		JsonWriter->WriteValue("engine_version", ENGINE_VERSION_STRING);
		JsonWriter->WriteValue(TEXT("device_name"), *FPlatformMisc::GetLoginId());
	}
	JsonWriter->WriteObjectEnd();
	
	JsonWriter->Close();

	const auto HttpRequest = HttpModule.CreateRequest();

	const FString APIServer(TEXT("https://dev.liv-latveria.com"));
	const FString URLPath(TEXT("api/auth/sdk"));
	
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetURL(APIServer / URLPath);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetContentAsString(Payload);

	AddInfo(FString::Printf(TEXT("Request Content: %s"), *Payload));

	HttpRequest->OnProcessRequestComplete().BindLambda([&](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bConnectedSuccessfully)
	{
		TestEqual(TEXT("Response Code is 200"), ResponsePtr->GetResponseCode(), 200);
		AddInfo(FString::Printf(TEXT("Response: %s"), *ResponsePtr->GetContentAsString()));
		AddInfo(FString::Printf(TEXT("Response Code: %d"), ResponsePtr->GetResponseCode()));
	});

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForHttpRequestLatentCommand(HttpRequest, 5.0));
	
	return true;
}


