// Copyright 2021 LIV Inc. - MIT License

#pragma once


#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"


/**
 * 
 */
class LIVEDITOR_API SLivEditorHelp : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivEditorHelp) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:

	static void OnNavigateDevHelp();
	FReply OnClickedDiscord() const;
	FReply OnClickedDocumentation() const;
};
