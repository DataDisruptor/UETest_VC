// Copyright 2021 LIV Inc. - MIT License
#pragma once
#include "CoreMinimal.h"


#include "AnalyticsEventAttribute.h"
#include "Containers/Ticker.h"

/**
 * Similar to FAnalyticsProviderET
 */
class LIVEDITOR_API FLivEditorAnalytics : public FTickerObjectBase
{
public:


	static void SetFingerprintId(FString FingerprintId);
	static FString GetFingerprintId();
	static bool HasValidFingerprintId();
	static void CheckFingerprintId();

	FLivEditorAnalytics();
	~FLivEditorAnalytics();
	
	virtual bool Tick(float DeltaTime) override;
	void FlushEvents();

	void RecordEvent(const FString& EventName);
	void RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes);
	void RecordEvent(FString EventName, TArray<FAnalyticsEventAttribute>&& Attributes);
	void OptOut();

	bool IsAnalyticsEnabled() const;

private:


private:

	struct FLivAnalyticsEventEntry
	{
		/** name of event */
		FString EventName;
		/** optional list of attributes */
		TArray<FAnalyticsEventAttribute> Attributes;
		/**
		* Constructor. Requires rvalue-refs to ensure we move values efficiently into this struct.
		*/
		FLivAnalyticsEventEntry(FString&& InEventName, TArray<FAnalyticsEventAttribute>&& InAttributes, bool bInIsJsonEvent, bool bInIsDefaultAttributes)
			: EventName(MoveTemp(InEventName))
			, Attributes(MoveTemp(InAttributes))
		{}
	};
	
	mutable FCriticalSection CachedEventsCS;

	float FlushEventsCountdown;

	FString TrackingID;

	/**
	 * List of analytic events pending a server update.
	 * NOTE: This MUST be accessed inside a lock on CachedEventsCS!!
	 */
	TArray<FLivAnalyticsEventEntry> CachedEvents;
};
