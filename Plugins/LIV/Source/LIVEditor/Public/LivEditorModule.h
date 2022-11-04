// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "LivAuthentication.h"
#include "LivUpdateChecker.h"
#include "LivEditorAnalytics.h"
#include "Widgets/Docking/SDockTab.h"

class FToolBarBuilder;
class FMenuBuilder;
class SNotificationItem;

class FLivEditorModule : public IModuleInterface
{
public:

	static FLivEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FLivEditorModule>("LIVEditor");
	}

	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("LIVEditor");
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FLivEditorAnalytics> GetAnalytics() const;
	TSharedPtr<FLivAuthentication> GetAuthentication() const;
	TSharedPtr<FLivUpdateChecker> GetUpdateChecker() const;

private:

	TSharedPtr<class FUICommandList> PluginCommands;

	TSharedPtr<FLivEditorAnalytics> Analytics;
	TSharedPtr<FLivAuthentication> Authentication;
	TSharedPtr<FLivUpdateChecker> UpdateChecker;

	TWeakPtr<SNotificationItem> SetupLivNotification;
	TWeakPtr<SNotificationItem> UpdateLivNotification;

	FDelegateHandle CheckForUpdatesHandle;

	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
	void PluginButtonClicked();
	void RegisterMenus();
	void OnMainFrameLoaded(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow);
	void OnSetupClicked();
	void OnSetupDismissed();
	void OnUpdateClicked();
	void OnUpdateDismissed();
	bool OnCheckUpdateStatus(float DeltaTime);
	void SettingsSurvey();
};
