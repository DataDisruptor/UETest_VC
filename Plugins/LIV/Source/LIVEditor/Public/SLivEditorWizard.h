// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Workflow/SWizard.h"

class SLivEditorWizard : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivEditorWizard) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

private:

	TWeakPtr<SDockTab> OwnerTab;
	TSharedPtr<SWizard> RootWizard;

	bool OnCanFinish() const;
	void OnFinishClicked();
	void OnCancelClicked() const;
	int32 GetInitialPageIndex() const;
	int32 GetNextPageIndex(int32 Current) const;
	FText GetPageTitle(int32 PageIndex) const;
	static void OnNavigateDevHelp();
	static void OnNavigateDevHelpAdvanced();
};

