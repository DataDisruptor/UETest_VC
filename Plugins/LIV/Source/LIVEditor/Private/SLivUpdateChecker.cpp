// Copyright 2021 LIV Inc. - MIT License


#include "SLivUpdateChecker.h"

#include "LivEditorModule.h"
#include "LivEditorStyle.h"
#include "EditorStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"

#define LOCTEXT_NAMESPACE "LivUpdateChecker"

namespace FLivUpdateCheckerIds
{
	const FName ColEngine = TEXT("Engine");
	const FName ColPlugin = TEXT("Plugin");
}

void SLivUpdateChecker::Construct(const FArguments& InArgs)
{
	UpdateChecker = FModuleManager::GetModuleChecked<FLivEditorModule>("LIVEditor").GetUpdateChecker();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SThrobber)
			.Visibility(this, &SLivUpdateChecker::GetThrobberVisibility)
			.PieceImage(FLivEditorStyle::Get().GetBrush("LIVEditor.Throbber.Chunk"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 10.0f)
		[
			SAssignNew(ListView, SListView<ItemType>)
			.ListItemsSource(&Items)
			.OnGenerateRow(this, &SLivUpdateChecker::OnGenerateRow)
			.SelectionMode(ESelectionMode::Single)
			.IsFocusable(false)
			.ClearSelectionOnClick(false)
			.OnIsSelectableOrNavigable(this, &SLivUpdateChecker::IsSelectableOrNavigable)
			.OnSelectionChanged(this, &SLivUpdateChecker::OnSelectionChanged)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column(FLivUpdateCheckerIds::ColEngine).DefaultLabel(LOCTEXT("ColEngine", "Engine"))
				+ SHeaderRow::Column(FLivUpdateCheckerIds::ColPlugin).DefaultLabel(LOCTEXT("ColPlugin", "Plugin"))
			)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility(this, &SLivUpdateChecker::GetStatusImageVisibility)
			.Padding(5.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SLivUpdateChecker::GetStatusImageBrush)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.TextStyle(&FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("RichTextBlock.Bold"))
			.Text(this, &SLivUpdateChecker::GetStatusText)
		]
		+ SVerticalBox::Slot()
		.Padding(0.0f, 10.0f)
		.AutoHeight()
		[
			SNew(SButton)
			.Visibility(this, &SLivUpdateChecker::GetUpdateButtonVisibility)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &SLivUpdateChecker::OnUpdateButtonClicked)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.TextStyle(FEditorStyle::Get(), "LargeText")
				.Text(LOCTEXT("UpdateButtonLabel", "Get Latest Version"))
			]
		]

	];

	if(UpdateChecker && UpdateChecker->CheckedSuccessfully())
	{
		BuildData();
	}
	else
	{
		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SLivUpdateChecker::RefreshStatus));
	}
}

EVisibility SLivUpdateChecker::GetThrobberVisibility() const
{
	return UpdateChecker && UpdateChecker->State == FLivUpdateChecker::EState::WaitingForCheckResponse
		? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SLivUpdateChecker::GetStatusImageVisibility() const
{
	return UpdateChecker && UpdateChecker->State == FLivUpdateChecker::EState::Success
		? EVisibility::Visible : EVisibility::Collapsed;
}

const FSlateBrush* SLivUpdateChecker::GetStatusImageBrush() const
{
	return (UpdateChecker && UpdateChecker->IsUpdateAvailable())
		? FLivEditorStyle::Get().GetBrush("LIVEditor.Cross")
		: FLivEditorStyle::Get().GetBrush("LIVEditor.Check");
}

FText SLivUpdateChecker::GetStatusText() const
{
	const auto State = UpdateChecker ? UpdateChecker->State : FLivUpdateChecker::EState::Failure;
	switch(State)
	{
	case FLivUpdateChecker::EState::Initializing: return LOCTEXT("State_Initializing", "Getting ready..");
	case FLivUpdateChecker::EState::WaitingForCheckResponse: return LOCTEXT("State_WaitingForCheckResponse", "Checking latest versions..");
	case FLivUpdateChecker::EState::Success:
		if(UpdateChecker->IsUpdateAvailable())
		{
			return FText::Format(LOCTEXT("State_Success_UpdateAvailable", "Update Available! You're on {0}, the newest verison is {1}"),
				FText::FromString(UpdateChecker->GetCurrentPluginVersion()),
				FText::FromString(UpdateChecker->GetVersionInfoForThisEngine().PluginVersion));
		}
		if(!UpdateChecker->IsCurrentVersionSupported())
		{
			return LOCTEXT("State_Success_VersionNotSupported", "Your engine version may not be supported.");
		}
		return FText::Format(LOCTEXT("State_Success_UpToDate", "Up to date on version {0}"), FText::FromString(UpdateChecker->GetCurrentPluginVersion()));
	case FLivUpdateChecker::EState::Failure:
		return LOCTEXT("State_Failure", "We can't check for plugin updates right now");
	default:
		return LOCTEXT("State_Unkown", "Unknown Failure");
	}
}

TSharedRef<ITableRow> SLivUpdateChecker::OnGenerateRow(ItemType Item, const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(SLivUpdateCheckerRow, InOwnerTable, Item);
}

bool SLivUpdateChecker::IsSelectableOrNavigable(ItemType Item) const
{
	return Item == CurrentVersionItem;
}

void SLivUpdateChecker::OnSelectionChanged(ItemType Item, ESelectInfo::Type) const
{
	if(CurrentVersionItem && Item != CurrentVersionItem)
	{
		ListView->SetSelection(CurrentVersionItem);
	}
}

void SLivUpdateChecker::BuildData()
{
	if(!UpdateChecker)
	{
		return;
	}

	Items.Reset();
	for(const auto& Info : UpdateChecker->GetVersionInfo())
	{
		Items.Add(MakeShared<FLivUpdateChecker::FVersionInfo>(Info));
	}

	const auto ThisEngineVersion = UpdateChecker->GetCurrentEngineMajorMinorVersion();
	if (Items.Num() && !ThisEngineVersion.IsEmpty())
	{
		for (const auto& Info : Items)
		{
			if(Info->EngineMajorMinorVersion == ThisEngineVersion)
			{
				CurrentVersionItem = Info;
				ListView->SetSelection(CurrentVersionItem);
			}
		}
	}
}

void SLivUpdateChecker::ForceUpdate()
{
	BuildData();
	if(ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

EActiveTimerReturnType SLivUpdateChecker::RefreshStatus(double InCurrentTime, float InDeltaTime)
{
	if(UpdateChecker && UpdateChecker->CheckedSuccessfully())
	{
		ForceUpdate();
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}

FReply SLivUpdateChecker::OnUpdateButtonClicked() const
{
	FPlatformProcess::LaunchURL(TEXT("https://dev.liv.tv/"), nullptr, nullptr);
	return FReply::Handled();
}

EVisibility SLivUpdateChecker::GetUpdateButtonVisibility() const
{
	return UpdateChecker && UpdateChecker->IsUpdateAvailable() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SLivUpdateCheckerRow::Construct(const FArguments& InArgs, 
                                     const TSharedRef<STableViewBase>& InOwnerTableView,
                                     const SLivUpdateChecker::ItemType InItem)
{
	Item = InItem;
	SMultiColumnTableRow<SLivUpdateChecker::ItemType>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SLivUpdateCheckerRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if(ColumnName == FLivUpdateCheckerIds::ColEngine)
	{
		return SNew(STextBlock).Text(FText::FromString(Item->EngineMajorMinorVersion));
	}

	if (ColumnName == FLivUpdateCheckerIds::ColPlugin)
	{
		return SNew(STextBlock).Text(FText::FromString(Item->PluginVersion));
	}

	return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE
