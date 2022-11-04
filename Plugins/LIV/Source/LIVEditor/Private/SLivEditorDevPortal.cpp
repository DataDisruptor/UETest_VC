// Copyright 2021 LIV Inc. - MIT License


#include "SLivEditorDevPortal.h"
#include "SLivAuthWidget.h"
#include "Styling/SlateTypes.h"
#include "LivEditorStyle.h"
#include "Widgets/Input/SButton.h"
#include "Components/VerticalBox.h"

#define LOCTEXT_NAMESPACE "LivEditorDevPortal"


void SLivEditorDevPortal::Construct(const FArguments& InArgs)
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
			.Text(LOCTEXT("LivEditorDevPortalContent", "Want to see how your game has been performing since integrating LIV? Head to the Dev Portal to see the latest stats for your game."))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SLivAuthWidget)
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
			.ButtonStyle(&FLivEditorStyle::Get().GetWidgetStyle<FButtonStyle>("LIVEditor.FlatButton.Success"))
			.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
			.OnClicked(this, &SLivEditorDevPortal::OnClickedDevPortal)
			[
				SNew(STextBlock)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle"))
				.Justification(ETextJustify::Center)
				.Text(LOCTEXT("DevPortalButtonLabel", "Dev Portal"))
			]
		]
	];

}

FReply SLivEditorDevPortal::OnClickedDevPortal() const
{
	FPlatformProcess::LaunchURL(TEXT("https://dev.liv.tv/"), nullptr, nullptr);
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
