// Copyright 2021 LIV Inc. - MIT License


#include "SLivEditorHelp.h"

#include "LivEditorStyle.h"
#include "Components/VerticalBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "LivEditorHelp"

void SLivEditorHelp::Construct(const FArguments& InArgs)
{

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(5.0f)
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("LivEditorHelpContent", "Need any help integrating LIV into your project? You can check out our documentation or get in touch with us on Discord."))
		]
		/*+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SHyperlink)
			.OnNavigate(FSimpleDelegate::CreateStatic(&SLivEditorHelp::OnNavigateDevHelp))
			.Text(LOCTEXT("DocsLink", "https://docs.liv.tv/sdk-for-unreal"))
		]*/
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ButtonStyle(&FLivEditorStyle::Get().GetWidgetStyle<FButtonStyle>("LIVEditor.FlatButton.Success"))
			.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
			.OnClicked(this, &SLivEditorHelp::OnClickedDocumentation)
			[
				SNew(STextBlock)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle"))
				.Justification(ETextJustify::Center)
				.Text(LOCTEXT("DocsButtonLabel", "Documentation"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ButtonStyle(FLivEditorStyle::Get(), TEXT("ToolBar.Button"))
			.ButtonColorAndOpacity(FSlateColor(FLinearColor::Transparent))
			.OnClicked(this, &SLivEditorHelp::OnClickedDiscord)
			[
				SNew(SImage)
				.Image(FLivEditorStyle::Get().GetBrush(TEXT("LivEditor.Discord")))
			]
		]
	];
}


void SLivEditorHelp::OnNavigateDevHelp()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.liv.tv/sdk-for-unreal"), nullptr, nullptr);
}

FReply SLivEditorHelp::OnClickedDiscord() const
{
	FPlatformProcess::LaunchURL(TEXT("https://discord.com/invite/liv"), nullptr, nullptr);
	return FReply::Handled();
}

FReply SLivEditorHelp::OnClickedDocumentation() const
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.liv.tv/sdk-for-unreal"), nullptr, nullptr);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
