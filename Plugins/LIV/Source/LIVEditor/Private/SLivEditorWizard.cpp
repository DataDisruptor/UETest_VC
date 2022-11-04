// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "SLivEditorWizard.h"

#include "LivEditorAnalytics.h"
#include "LivEditorStyle.h"
#include "EditorStyleSet.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "SLivAuthWidget.h"

#define LOCTEXT_NAMESPACE "LivEditorWizard"

namespace LivWizardDefs
{
	constexpr int32 LandingPageIndex = 0;
	constexpr int32 SetupAccountIndex = 1;
	constexpr int32 SetupRenderingIndex = 2;

	static const FText LandingPageTitle = LOCTEXT("LivWizard_Landing", "Setup LIV SDK");
	static const FText SetupAccountTitle = LOCTEXT("LivWizard_SetupAccount", "Setup Account");
	static const FText SetupRenderingTitle = LOCTEXT("LivWizard_SetupRendering", "Setup Rendering");
}

void SLivEditorWizard::Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab)
{
	OwnerTab = InOwnerTab;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FLivEditorStyle::Get().GetBrush("LIVEditor.Background"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SImage)
				.Image(FLivEditorStyle::Get().GetBrush("LIVEditor.HeaderLogoLarge"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SAssignNew(RootWizard, SWizard)
				.ShowBreadcrumbs(false)
				.ShowPageList(true)
				.ShowPageTitle(true)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
				.CancelButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
				.FinishButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.ButtonTextStyle(FEditorStyle::Get(), "LargeText")
				.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
				.FinishButtonText(LOCTEXT("FinishButtonText", "Finish"))
				.FinishButtonToolTip(LOCTEXT("FinishButtonTooltip", "Finish setting up LIV SDK."))
				.CanFinish(this, &SLivEditorWizard::OnCanFinish)
				.OnFinished(this, &SLivEditorWizard::OnFinishClicked)
				.OnCanceled(this, &SLivEditorWizard::OnCancelClicked)
				.InitialPageIndex(this, &SLivEditorWizard::GetInitialPageIndex)
				.OnGetNextPageIndex(this, &SLivEditorWizard::GetNextPageIndex)

				+ SWizard::Page()
				.Name(GetPageTitle(LivWizardDefs::LandingPageIndex))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.3f)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle"))
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("LandingPageHeader", "Welcome Travellers"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("LandingPageContent1", "LIV provides out-of-the-box capture tools that your players can use to record and live stream themselves in Mixed Reality, as Avatars, or through our enhanced first-person capture features."))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("LandingPageContent2", "We'll guide you through the simple setup to get started with LIV."))
						]
						+ SVerticalBox::Slot()
						.Padding(0.0f, 20.0f)
						.VAlign(VAlign_Bottom)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.Text(LOCTEXT("NeedHelp", "Need help? You can find our Unreal SDK Docs at"))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(SHyperlink)
								.OnNavigate(FSimpleDelegate::CreateStatic(&SLivEditorWizard::OnNavigateDevHelp))
								.Text(LOCTEXT("DocsLink", "https://docs.liv.tv/sdk-for-unreal"))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.7f)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							SNew(SBox)
							.Padding(40.0f)
							[
								SNew(SImage)
								.Image(FLivEditorStyle::Get().GetBrush("LIVEditor.LandingPage"))
							]
						]
					]
				]

				+ SWizard::Page()
				.Name(GetPageTitle(LivWizardDefs::SetupAccountIndex))
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SVerticalBox)
						
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Fill)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							[
								SNew(SLivAuthWidget)
							]
						]
					]
				]

				+ SWizard::Page()
				.Name(GetPageTitle(LivWizardDefs::SetupRenderingIndex))
				[
					SNew(SBox)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(0.0f, 20.0f)
						.AutoHeight()
						[
							SNew(STextBlock)
							.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle"))
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("SetupRenderingHeader", "Configure Rendering"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("SetupRenderingContent1", "LIV works out of the box with no configuration but the rendering can be configured to better suit your project."))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("SetupRenderingContent2", "Check the documentation at the link below to read about the different rendering options available"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SHyperlink)
							.OnNavigate(FSimpleDelegate::CreateStatic(&SLivEditorWizard::OnNavigateDevHelpAdvanced))
							.Text(LOCTEXT("AdvancedDocsLink", "https://docs.liv.tv/sdk-for-unreal/integrating-liv"))
						]
					]
				]
			]
		]
	];
}

bool SLivEditorWizard::OnCanFinish() const
{
	// @todo: and decided on a rendering method
	return FLivEditorAnalytics::HasValidFingerprintId();
}

void SLivEditorWizard::OnFinishClicked()
{
	const TSharedPtr<SDockTab> Tab = OwnerTab.Pin();
	if (Tab.IsValid())
	{
		Tab->RequestCloseTab();
	}
	/*const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	Window->RequestDestroyWindow();*/
}

void SLivEditorWizard::OnCancelClicked() const
{
	const TSharedPtr<SDockTab> Tab = OwnerTab.Pin();
	if(Tab.IsValid())
	{
		Tab->RequestCloseTab();
	}
	/*const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	Window->RequestDestroyWindow();*/
}

int32 SLivEditorWizard::GetInitialPageIndex() const
{
	//if(FLivEditorAnalytics::HasValidFingerprintId())
	//{
	//	return LivWizardDefs::SetupAccountIndex;
	//}
	return LivWizardDefs::LandingPageIndex;
}

int32 SLivEditorWizard::GetNextPageIndex(int32 Current) const
{
	const auto CurrentPage = RootWizard->GetCurrentPageIndex();
	switch (CurrentPage)
	{
	case LivWizardDefs::LandingPageIndex:
		if (!FLivEditorAnalytics::HasValidFingerprintId())
		{
			return LivWizardDefs::SetupAccountIndex;
		}
		return LivWizardDefs::SetupRenderingIndex;
	case LivWizardDefs::SetupAccountIndex:
		return LivWizardDefs::SetupRenderingIndex;
	default:
		return INDEX_NONE;
	}
}

FText SLivEditorWizard::GetPageTitle(int32 PageIndex) const
{
	switch (PageIndex)
	{
	case LivWizardDefs::LandingPageIndex: return LivWizardDefs::LandingPageTitle;
	case LivWizardDefs::SetupAccountIndex: return LivWizardDefs::SetupAccountTitle;
	case LivWizardDefs::SetupRenderingIndex: return LivWizardDefs::SetupRenderingTitle;

	default: return FText::GetEmpty();
	}
}

void SLivEditorWizard::OnNavigateDevHelp()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.liv.tv/sdk-for-unreal"), nullptr, nullptr);
}

void SLivEditorWizard::OnNavigateDevHelpAdvanced()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.liv.tv/sdk-for-unreal/integrating-liv"), nullptr, nullptr);
}

#undef LOCTEXT_NAMESPACE
