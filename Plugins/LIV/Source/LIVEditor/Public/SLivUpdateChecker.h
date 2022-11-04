// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "LivUpdateChecker.h"

/**
 * 
 */
class LIVEDITOR_API SLivUpdateChecker : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivUpdateChecker) {}
	SLATE_END_ARGS()

	using ItemType = TSharedPtr<FLivUpdateChecker::FVersionInfo>;
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);


private:

	TSharedPtr<FLivUpdateChecker> UpdateChecker;
	TSharedPtr<SListView<ItemType>> ListView;
	TArray<ItemType> Items;
	ItemType CurrentVersionItem;

	EVisibility GetThrobberVisibility() const;
	EVisibility GetStatusImageVisibility() const;
	const FSlateBrush* GetStatusImageBrush() const;
	FText GetStatusText() const;
	TSharedRef<ITableRow> OnGenerateRow(ItemType Item, const TSharedRef<STableViewBase>& InOwnerTable);
	bool IsSelectableOrNavigable(ItemType Item) const;
	void OnSelectionChanged(ItemType Item, ESelectInfo::Type) const;
	void BuildData();
	void ForceUpdate();
	EActiveTimerReturnType RefreshStatus(double InCurrentTime, float InDeltaTime);
	FReply OnUpdateButtonClicked() const;
	EVisibility GetUpdateButtonVisibility() const;
};

class SLivUpdateCheckerRow : public SMultiColumnTableRow<SLivUpdateChecker::ItemType>
{
public:
	SLATE_BEGIN_ARGS(SLivUpdateCheckerRow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const SLivUpdateChecker::ItemType InItem);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	SLivUpdateChecker::ItemType Item;
};
