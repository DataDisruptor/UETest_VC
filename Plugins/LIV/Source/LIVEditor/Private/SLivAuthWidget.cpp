// Copyright 2021 LIV Inc. - MIT License

#include "SLivAuthWidget.h"

#include "EditorStyleSet.h"
#include "LivEditorModule.h"
#include "LivEditorStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Text/TextLayout.h"

#define LOCTEXT_NAMESPACE "LivAuthWidget"

void SLivAuthWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.WidthOverride(550)
		.HeightOverride(200)
		[
			SNew(SBorder)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.Padding(40.0f)
			.BorderImage(FLivEditorStyle::Get().GetBrush("LIVEditor.DarkGroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.TextStyle(FLivEditorStyle::Get(), "LIVEditor.Wizard.StatusText")
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
					.Text(this, &SLivAuthWidget::GetStatusText)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Visibility(this, &SLivAuthWidget::GetLaunchAuthFlowButtonVisibility)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(&FLivEditorStyle::Get().GetWidgetStyle<FButtonStyle>("LIVEditor.FlatButton.Success"))
						//.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.TextStyle(FEditorStyle::Get(), "LargeText")
						.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
						.ContentPadding(FCoreStyle::Get().GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(this, &SLivAuthWidget::OnLaunchAuthFlowClicked)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "LargeText")
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("LaunchBrowserAuthFlow", "Continue in Browser"))
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Visibility(this, &SLivAuthWidget::GetRestartButtonVisibility)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FCoreStyle::Get().GetMargin("StandardDialog.ContentPadding"))
						.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
						.OnClicked(this, &SLivAuthWidget::OnRestartClicked)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText"))
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("Restart", "Restart"))
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Visibility(this, &SLivAuthWidget::GetThrobberVisibility)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SCircularThrobber)
						.PieceImage(FLivEditorStyle::Get().GetBrush("LIVEditor.Throbber.CircleChunk"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Visibility(this, &SLivAuthWidget::GetCheckVisibility)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FLivEditorStyle::Get().GetBrush("LIVEditor.Check"))
					]
				]
			]
		]
	];

	LivAuthentication = FModuleManager::GetModuleChecked<FLivEditorModule>("LIVEditor").GetAuthentication();
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SLivAuthWidget::RefreshStatus));
}

FText SLivAuthWidget::GetStatusText() const
{
	if (!LivAuthentication) return FText();

	switch(LivAuthentication->State)
	{
	case FLivAuthentication::EState::Initializing: return LOCTEXT("State_Initializing", "Getting ready..");
	case FLivAuthentication::EState::RequestToken: return LOCTEXT("State_RequestToken", "Requesting token");
	case FLivAuthentication::EState::WaitForRequestTokenResponse: return LOCTEXT("State_WaitForRequestTokenResponse", "Waiting for response from LIV Dev Portal");
	case FLivAuthentication::EState::WaitForLaunchBrowserInput:
		if (!FLivEditorAnalytics::HasValidFingerprintId())
		{
			return LOCTEXT("State_WaitForLaunchBrowserInput", "Head over to the LIV Dev Portal to link your game to this project.");
		}
		return LOCTEXT("State_WaitForLaunchBrowserInput_Alt", "Head over to the LIV Dev Portal to login.");
	case FLivAuthentication::EState::LaunchBrowserAuthFlow: return LOCTEXT("State_LaunchBrowserAuthFlow", "Launching browser..");
	case FLivAuthentication::EState::DelayCheckAuthFlow: return LOCTEXT("State_DelayCheckAuthFlow", "Return after you've finished setting up in your browser.");
	case FLivAuthentication::EState::SendCheckAuthFlowRequest: return LOCTEXT("State_SendCheckAuthFlowRequest", "Return after you've finished setting up in your browser.");
	case FLivAuthentication::EState::WaitingForAuthCheckResponse: return LOCTEXT("State_WaitingForAuthCheckResponse", "Return after you've finished setting up in your browser.");
	case FLivAuthentication::EState::AuthSuccess: return LOCTEXT("State_AuthSuccess", "Done! You've linked your project with the LIV Dev Portal.");
	case FLivAuthentication::EState::RequestPortalSession: return LOCTEXT("State_RequestPortalSession", "Requesting a Dev Portal Session..");
	case FLivAuthentication::EState::WaitForPortalSessionResponse: return LOCTEXT("State_WaitForPortalSessionResponse", "Waiting for Dev Portal response..");
	case FLivAuthentication::EState::LaunchBrowserPortalSession: return LOCTEXT("State_LaunchBrowserPortalSession", "Continue in browser");
	case FLivAuthentication::EState::Timeout: return LOCTEXT("State_Timeout", "Timed out, please try again or contact support.");
	case FLivAuthentication::EState::Failure: return FText::FromString(LivAuthentication->ErrorReason);
	default: return LOCTEXT("StateUnknown", "Unknown State");
	}
}

EVisibility SLivAuthWidget::GetLaunchAuthFlowButtonVisibility() const
{
	if (!LivAuthentication)
	{
		return EVisibility::Collapsed;
	}

	switch (LivAuthentication->State)
	{
	case FLivAuthentication::EState::WaitForLaunchBrowserInput:
	case FLivAuthentication::EState::LaunchBrowserAuthFlow:
	case FLivAuthentication::EState::DelayCheckAuthFlow:
	case FLivAuthentication::EState::SendCheckAuthFlowRequest:
	case FLivAuthentication::EState::WaitingForAuthCheckResponse:
		return EVisibility::Visible;
	default:
		return EVisibility::Collapsed;
	}
}

FReply SLivAuthWidget::OnLaunchAuthFlowClicked() const
{
	if(LivAuthentication)
	{
		LivAuthentication->State = FLivAuthentication::EState::LaunchBrowserAuthFlow;
	}
	return FReply::Handled();
}

EVisibility SLivAuthWidget::GetRestartButtonVisibility() const
{
	return LivAuthentication
		&& (LivAuthentication->State == FLivAuthentication::EState::Failure
			|| LivAuthentication->State == FLivAuthentication::EState::Timeout)
		? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SLivAuthWidget::OnRestartClicked() const
{
	if (LivAuthentication)
	{
		LivAuthentication->State = FLivAuthentication::EState::Initializing;
	}
	return FReply::Handled();
}

EVisibility SLivAuthWidget::GetThrobberVisibility() const
{
	if(!LivAuthentication)
	{
		return EVisibility::Collapsed;
	}

	switch(LivAuthentication->State)
	{
	case FLivAuthentication::EState::RequestToken:
	case FLivAuthentication::EState::DelayCheckAuthFlow:
	case FLivAuthentication::EState::WaitForRequestTokenResponse:
	case FLivAuthentication::EState::SendCheckAuthFlowRequest:
	case FLivAuthentication::EState::WaitingForAuthCheckResponse:
		return EVisibility::Visible;
	default:
		return EVisibility::Collapsed;
	}
}

EVisibility SLivAuthWidget::GetCheckVisibility() const
{
	return LivAuthentication && LivAuthentication->State == FLivAuthentication::EState::AuthSuccess
		? EVisibility::Visible : EVisibility::Hidden;
}

EActiveTimerReturnType SLivAuthWidget::RefreshStatus(double InCurrentTime, float InDeltaTime)
{
	LivAuthentication->UpdateState(InDeltaTime);
	return EActiveTimerReturnType::Continue;
}

#undef LOCTEXT_NAMESPACE
