// Copyright 2021 LIV Inc. - MIT License

#include "SLivEditorHome.h"
#include "LivEditorStyle.h"
#include "SLivUpdateChecker.h"
#include "SLivEditorHelp.h"
#include "SLivEditorDevPortal.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "LivEditorHome"

void SLivEditorHome::Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab)
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
			/*+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle"))
				.Justification(ETextJustify::Center)
				.Text(LOCTEXT("LivHomeHeader", "Hello There"))
			]*/
			+ SVerticalBox::Slot()
			.FillHeight(0.9f)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(20.0f)
				+ SUniformGridPanel::Slot(0, 0)
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
						[
							SNew(STextBlock)
							.TextStyle(&FLivEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("LivEditor.Home.SubHeader"))
							.Text(LOCTEXT("UpdatesSubHeader", "Plugin Updates"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SLivUpdateChecker)
						]
					]
				]
				+ SUniformGridPanel::Slot(1, 0)
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
						[
							SNew(STextBlock)
							.TextStyle(&FLivEditorStyle::Get().GetWidgetStyle< FTextBlockStyle >("LivEditor.Home.SubHeader"))
							.Text(LOCTEXT("DevPortalSubHeader", "Dev Portal"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SLivEditorDevPortal)
						]
					]
				]
				+ SUniformGridPanel::Slot(2, 0)
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
						[
							SNew(STextBlock)
							.TextStyle(&FLivEditorStyle::Get().GetWidgetStyle< FTextBlockStyle >("LivEditor.Home.SubHeader"))
							.Text(LOCTEXT("HelpSubHeader", "Help"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SLivEditorHelp)
						]
					]
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
