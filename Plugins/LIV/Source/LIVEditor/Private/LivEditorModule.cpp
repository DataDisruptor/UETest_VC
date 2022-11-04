// Copyright 2021 LIV Inc. - MIT License
#include "LivEditorModule.h"

#include "GameProjectUtils.h"
#include "LivEditorStyle.h"
#include "LivDebuggingTab.h"
#include "ISettingsCategory.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "LivEditorCommands.h"
#include "LivEditorSettings.h"
#include "LivPluginSettings.h"
#include "MeshDescription.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "SLivEditorWizard.h"
#include "SLivEditorHome.h"
#include "Engine/RendererSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName LIVEditorTabName("LIVEditor");

#define LOCTEXT_NAMESPACE "FLIVEditorModule"

void FLivEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	Authentication = MakeShared<FLivAuthentication>();
	Authentication->Initialize();

	Analytics = MakeShared<FLivEditorAnalytics>();
	FLivEditorAnalytics::CheckFingerprintId();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// register editor settings
		SettingsModule->RegisterSettings("Editor", "Plugins", "LIV",
			LOCTEXT("LivEditorSettingsName", "LIV Editor"),
			LOCTEXT("LivEditorSettingsDescription", "Configure LIV Editor Plugin Settings."),
			GetMutableDefault<ULivEditorSettings>()
		);

		// detect plugin settings changes for analytics
		if(const TSharedPtr<ISettingsContainer> ProjectSettingsContainer = SettingsModule->GetContainer("Project"))
		{
			if(const TSharedPtr<ISettingsCategory> PluginSettingsCategory = ProjectSettingsContainer->GetCategory("Plugins"))
			{
				if(const TSharedPtr<ISettingsSection> LivSettingsSection = PluginSettingsCategory->GetSection("LIV"))
				{
					LivSettingsSection->OnModified().BindLambda([&]
					{
						Analytics->RecordEvent(TEXT("PLUGIN_SETTINGS_MODIFIED"));
						return true;
					});
				}
			}
		}
	}

	Analytics->RecordEvent(TEXT("EDITOR_STARTED"));

	FLivEditorStyle::Initialize();
	FLivEditorStyle::ReloadTextures();

	{
		const IWorkspaceMenuStructure& MenuStructure = WorkspaceMenu::GetMenuStructure();
		TSharedRef<FWorkspaceItem> MediaBrowserGroup = MenuStructure.GetDeveloperToolsMiscCategory()->GetParent()->AddGroup(
			LOCTEXT("WorkspaceMenu_MediaCategory", "Media"),
			FSlateIcon(),
			true);

		SLivDebuggingTab::RegisterNomadTabSpawner(MediaBrowserGroup);
	}

	FLivEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLivEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLivEditorModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLivEditorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		LIVEditorTabName, 
		FOnSpawnTab::CreateRaw(this, &FLivEditorModule::OnSpawnPluginTab))
			.SetDisplayName(LOCTEXT("LivEditorWindowTabTitle", "LIV"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

	UpdateChecker = MakeShared<FLivUpdateChecker>();
	UpdateChecker->Initialize();

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FLivEditorModule::OnMainFrameLoaded);
}

void FLivEditorModule::ShutdownModule()
{
	// unregister editor settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "LIV");
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLivEditorStyle::Shutdown();

	FLivEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LIVEditorTabName);

	if (CheckForUpdatesHandle.IsValid())
	{
		FTicker::GetCoreTicker().RemoveTicker(CheckForUpdatesHandle);
	}
}

TSharedPtr<FLivEditorAnalytics> FLivEditorModule::GetAnalytics() const
{
	return Analytics;
}

TSharedPtr<FLivAuthentication> FLivEditorModule::GetAuthentication() const
{
	return Authentication;
}

TSharedPtr<FLivUpdateChecker> FLivEditorModule::GetUpdateChecker() const
{
	return UpdateChecker;
}

TSharedRef<SDockTab> FLivEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> ResultTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	if (!FLivEditorAnalytics::HasValidFingerprintId())
	{
		const TSharedRef<SWidget> TabContentWidget = SNew(SLivEditorWizard, ResultTab);
		ResultTab->SetContent(TabContentWidget);
	}
	else
	{
		const TSharedRef<SWidget> TabContentWidget = SNew(SLivEditorHome, ResultTab);
		ResultTab->SetContent(TabContentWidget);
	}

	return ResultTab;
}

void FLivEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LIVEditorTabName);
}

void FLivEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLivEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLivEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FLivEditorModule::OnMainFrameLoaded(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow)
{
	if (!FLivEditorAnalytics::HasValidFingerprintId())
	{
		FNotificationInfo Info(LOCTEXT("SetupLivNotification", "Setup LIV SDK"));
		Info.bFireAndForget = false;
		Info.bUseLargeFont = true;
		Info.bUseThrobber = false;
		Info.FadeOutDuration = 0.0f;
		Info.ExpireDuration = 0.0f;
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("OpenWizard", "Go to Setup Wizard"),
			LOCTEXT("OpenWizardTooltip", "Open a guided setup"),
			FSimpleDelegate::CreateRaw(this, &FLivEditorModule::OnSetupClicked)));
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("OpenWizardDismiss", "Dismiss"),
			LOCTEXT("OpenWizardDismissTooltip", "Dismiss this notification"),
			FSimpleDelegate::CreateRaw(this, &FLivEditorModule::OnSetupDismissed)));

		SetupLivNotification = FSlateNotificationManager::Get().AddNotification(Info);
		SetupLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}

	// add a ticker to check for updates
	CheckForUpdatesHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FLivEditorModule::OnCheckUpdateStatus), 5.0f);

	SettingsSurvey();
}

void FLivEditorModule::OnSetupClicked()
{
	PluginButtonClicked();
	SetupLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	SetupLivNotification.Pin()->ExpireAndFadeout();
}

void FLivEditorModule::OnSetupDismissed()
{
	SetupLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	SetupLivNotification.Pin()->ExpireAndFadeout();
}

void FLivEditorModule::OnUpdateClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://dev.liv.tv/"), nullptr, nullptr);
	UpdateLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	UpdateLivNotification.Pin()->ExpireAndFadeout();
}

void FLivEditorModule::OnUpdateDismissed()
{
	UpdateLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	UpdateLivNotification.Pin()->ExpireAndFadeout();
}

bool FLivEditorModule::OnCheckUpdateStatus(float DeltaTime)
{
	if(UpdateChecker->IsPending())
	{
		return true;
	}

	if (UpdateChecker->CheckedSuccessfully() && UpdateChecker->IsUpdateAvailable())
	{
		FNotificationInfo Info(LOCTEXT("UpdateLivNotification", "Update LIV SDK"));
		Info.bFireAndForget = false;
		Info.bUseLargeFont = true;
		Info.bUseThrobber = false;
		Info.FadeOutDuration = 0.0f;
		Info.ExpireDuration = 0.0f;
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("UpdateLiv", "Go to Dev Portal"),
			LOCTEXT("UpdateLivTooltip", "Go to the Dev Portal for the new version"),
			FSimpleDelegate::CreateRaw(this, &FLivEditorModule::OnUpdateClicked)));
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("UpdateLivDismiss", "Dismiss"),
			LOCTEXT("UpdateLivDismissTooltip", "Dismiss this notification"),
			FSimpleDelegate::CreateRaw(this, &FLivEditorModule::OnUpdateDismissed)));

		UpdateLivNotification = FSlateNotificationManager::Get().AddNotification(Info);
		UpdateLivNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}

	return false;
}

void FLivEditorModule::SettingsSurvey()
{
	const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
	//const UGeneralProjectSettings* GeneralProjectSettings = GetDefault<UGeneralProjectSettings>();
	const TArray<FAnalyticsEventAttribute> SurveyAttributes =
	{
		FAnalyticsEventAttribute(TEXT("forward_rendering"), RendererSettings->bForwardShading ? true : false),
		FAnalyticsEventAttribute(TEXT("antialiasing_method"), UEnum::GetDisplayValueAsText(RendererSettings->DefaultFeatureAntiAliasing).ToString()),
		FAnalyticsEventAttribute(TEXT("global_clip_plane"), RendererSettings->bGlobalClipPlane ? true : false),
		FAnalyticsEventAttribute(TEXT("auto_exposure"), RendererSettings->bDefaultFeatureAutoExposure ? true : false),
		FAnalyticsEventAttribute(TEXT("auto_exposure_method"), UEnum::GetDisplayValueAsText(RendererSettings->DefaultFeatureAutoExposure).ToString()),
		FAnalyticsEventAttribute(TEXT("auto_exposure_bias"), RendererSettings->DefaultFeatureAutoExposureBias),
		FAnalyticsEventAttribute(TEXT("pre_exposure"), RendererSettings->bUsePreExposure ? true : false),
		FAnalyticsEventAttribute(TEXT("bloom"), RendererSettings->bDefaultFeatureBloom ? true : false),
		FAnalyticsEventAttribute(TEXT("instanced_stereo"), RendererSettings->bMultiView ? true : false),
		FAnalyticsEventAttribute(TEXT("has_cpp"), GameProjectUtils::ProjectHasCodeFiles() ? true : false),
		FAnalyticsEventAttribute(TEXT("source_control"), ISourceControlModule::Get().GetProvider().GetName().ToString()),
	};
	Analytics->RecordEvent(TEXT("SETTINGS_SURVEY"), SurveyAttributes);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLivEditorModule, LIVEditor)