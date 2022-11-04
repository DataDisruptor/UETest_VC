// Copyright 2021 LIV Inc. - MIT License
#include "LivEditorSettings.h"

#include "LivEditorModule.h"

ULivEditorSettings::ULivEditorSettings()
	: bAnalyticsEnabled(true)
{
	
}

#if WITH_EDITOR
void ULivEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULivEditorSettings, bAnalyticsEnabled))
	{
		if (!bAnalyticsEnabled)
		{
			FLivEditorModule::Get().GetAnalytics()->OptOut();
		}
		else
		{
			FLivEditorModule::Get().GetAnalytics()->RecordEvent(TEXT("ANALYTICS_OPT_IN"));
		}
	}
}
#endif
