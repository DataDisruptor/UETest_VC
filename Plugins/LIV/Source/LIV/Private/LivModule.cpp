// Copyright 2021 LIV Inc. - MIT License
#include "LivModule.h"
#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "JsonObjectConverter.h"
#include "LivIdentifier.h"
#include "Misc/Build.h"
#include "Misc/FileHelper.h"

#if ALLOW_CONSOLE
#include "Engine/Console.h"
#include "ConsoleSettings.h"
#include "HAL/IConsoleManager.h"
#endif // ALLOW_CONSOLE

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "LivCaptureBase.h"
#include "LivLocalPlayerSubsystem.h"
#include "LivNativeWrapper.h"
#include "LivPluginSettings.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif //WITH_EDITOR

DEFINE_LOG_CATEGORY(LogLivModule);

#define LOCTEXT_NAMESPACE "FLivPluginModule"

class FLivModule : public ILivModule
{
public:

	virtual bool IsSDKLoaded() const override { return bLivSDKLoaded; };

	void StartupModule() override;
	void ShutdownModule() override;

	void RegisterSettings();
	void UnregisterSettings();

	void EnsureSdkIdentifier();

	static void StartupConsoleCommands();

	bool bLivSDKLoaded{ false };
};

void FLivModule::StartupModule()
{
	if (!FLivNativeWrapper::IsSupported())
	{
		return;
	}

	RegisterSettings();

	EnsureSdkIdentifier();

	bLivSDKLoaded = FLivNativeWrapper::StartUp();
	
	StartupConsoleCommands();
}

void FLivModule::ShutdownModule()
{
	if (!bLivSDKLoaded)
	{
		return;
	}

	FLivNativeWrapper::Shutdown();

	UnregisterSettings();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FLivModule::RegisterSettings()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		// register settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule != nullptr)
		{
			// play-in settings
			SettingsModule->RegisterSettings("Project", "Plugins", "LIV",
				LOCTEXT("LivSettingsName", "LIV"),
				LOCTEXT("LivSettingsDescription", "Configure LIV Plugin Settings."),
				GetMutableDefault<ULivPluginSettings>()
			);
		}
	}
#endif //WITH_EDITOR
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FLivModule::UnregisterSettings()
{
	// @TODO: split runtime & editor settings
#if WITH_EDITOR
	if (GIsEditor)
	{
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "LIV");
		}
	}
#endif //WITH_EDITOR
}

void FLivModule::EnsureSdkIdentifier()
{
#if WITH_EDITOR && PLATFORM_WINDOWS
	const auto LivPlugin = IPluginManager::Get().FindPlugin(TEXT("Liv"));

	const FLivIdentifier Identifier
	{
		EPIC_PRODUCT_IDENTIFIER,
		ENGINE_VERSION_STRING,
		LivPlugin->GetDescriptor().VersionName,
		FLivNativeWrapper::GetVersionString()
	};

	FString IdentifierString;
	FJsonObjectConverter::UStructToJsonObjectString(Identifier, IdentifierString);

	const FString IdentifierFilePath = FPaths::Combine(LivPlugin->GetBaseDir(), TEXT("Resources"), TEXT("app.livsdk"));

	FString CurrentFile;
	if(FFileHelper::LoadFileToString(CurrentFile, *IdentifierFilePath))
	{
		TSharedPtr<FJsonObject> RootJsonObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CurrentFile);
		FJsonSerializer::Deserialize(Reader, RootJsonObject);

		if(RootJsonObject.IsValid())
		{
			FLivIdentifier ExistingIdentifier;
			if(FJsonObjectConverter::JsonObjectToUStruct<FLivIdentifier>(RootJsonObject.ToSharedRef(), &ExistingIdentifier))
			{
				if(ExistingIdentifier == Identifier)
				{
					// it's the same, don't overwrite
					return;
				}
			}
		}
	}

	if (!FFileHelper::SaveStringToFile(IdentifierString, *IdentifierFilePath))
	{
		UE_LOG(LogLivModule, Warning, TEXT("Failed to write LIV identifier file to: %s"), *IdentifierFilePath);
	}
#endif //WITH_EDITOR
}

#if ALLOW_CONSOLE

/**
 * Console commands
 */
struct FLivConsoleCommands
{
	FLivConsoleCommands()
		: GetCaptureClassesCommand(
			TEXT("Liv.PrintCaptureClasses"),
			TEXT("Prints a list of classes that derive from ULivCaptureBase."),
			FConsoleCommandDelegate::CreateRaw(this, &FLivConsoleCommands::PrintCaptureClasses))
		, SetCaptureClassCommand(
			TEXT("Liv.SetCaptureClass"),
			TEXT("Set the class for capture method to use."),
			FConsoleCommandWithArgsDelegate::CreateRaw(this, &FLivConsoleCommands::SetCaptureClass))
		, ResetCaptureCommand(
			TEXT("Liv.ResetCapture"),
			TEXT("Reset capture state."),
			FConsoleCommandDelegate::CreateRaw(this, &FLivConsoleCommands::ResetCapture))
	{}

	FAutoConsoleCommand GetCaptureClassesCommand;
	FAutoConsoleCommand SetCaptureClassCommand;
	FAutoConsoleCommand ResetCaptureCommand;

	static TArray<TSubclassOf<ULivCaptureBase>> GetLivCaptureClasses()
	{
		TArray<TSubclassOf<ULivCaptureBase>> Classes;
		// should be at least 4 non abstract capture classes
		Classes.Reserve(4);
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (!It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated) && It->IsChildOf<ULivCaptureBase>())
			{
				Classes.Add(*It);
			}
		}
		return Classes;
	}

	void PrintCaptureClasses()
	{
		UE_LOG(LogLivModule, Log, TEXT("Classes that derive from ULivCaptureBase: "));
		const auto Classes = GetLivCaptureClasses();
		for (auto It = Classes.CreateConstIterator(); It; ++It)
		{
			const UClass* UClass = *It;
			UE_LOG(LogLivModule, Log, TEXT("Class: %s"), *UClass->GetName());
		}
	}

	void SetCaptureClass(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogLivModule, Warning, TEXT("Usage: Liv.SetCaptureClass ClassName"));
			return;
		}

		const FString& ClassName = Args[0];

		const auto Classes = GetLivCaptureClasses();

		for (auto It = Classes.CreateConstIterator(); It; ++It)
		{
			if((*It)->GetName().Equals(ClassName))
			{
				GetMutableDefault<ULivPluginSettings>()->CaptureMethod = *It;
			}
		}
	}

	void ResetCapture()
	{
		for(TObjectIterator<ULivLocalPlayerSubsystem> It; It; ++It)
		{
			It->ResetCapture();
		}
	}

	static void PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteList)
	{
		const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();

		const auto Classes = GetLivCaptureClasses();
		for (auto It = Classes.CreateConstIterator(); It; ++It)
		{
			AutoCompleteList.AddDefaulted();
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.Last();
			AutoCompleteCommand.Command = TEXT("Liv.SetCaptureClass ") + (*It)->GetName();
			AutoCompleteCommand.Desc = TEXT("Set the class for capture method to use.");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}
	}
};

static FLivConsoleCommands LivConsoleCommands;

void FLivModule::StartupConsoleCommands()
{
	UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&FLivConsoleCommands::PopulateAutoCompleteEntries);
}

#else
void FLivModule::StartupConsoleCommands()
{
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLivModule, LIV)