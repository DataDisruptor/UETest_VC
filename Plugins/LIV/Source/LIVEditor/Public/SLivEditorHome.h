// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"


class SLivEditorHome : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivEditorHome) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

private:

	TWeakPtr<SDockTab> OwnerTab;
};
