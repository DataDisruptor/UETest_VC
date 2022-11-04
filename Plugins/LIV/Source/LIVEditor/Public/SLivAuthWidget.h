// Copyright 2021 LIV Inc. - MIT License

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

DECLARE_DELEGATE_OneParam(FOnLivAuthSucess, const FString& PortalToken)

/**
 * 
 */
class SLivAuthWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivAuthWidget) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:

	FText GetStatusText() const;
	EVisibility GetLaunchAuthFlowButtonVisibility() const;
	FReply OnLaunchAuthFlowClicked() const;
	EVisibility GetRestartButtonVisibility() const;
	FReply OnRestartClicked() const;
	EVisibility GetThrobberVisibility() const;
	EVisibility GetCheckVisibility() const;
	EActiveTimerReturnType RefreshStatus(double InCurrentTime, float InDeltaTime);

	TSharedPtr<class FLivAuthentication> LivAuthentication;
};
