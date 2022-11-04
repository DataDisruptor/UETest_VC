// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class LIVEDITOR_API SLivEditorDevPortal : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivEditorDevPortal) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:

	FReply OnClickedDevPortal() const;
};
