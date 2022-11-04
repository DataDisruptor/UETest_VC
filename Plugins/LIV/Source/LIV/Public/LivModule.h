// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Delegates/Delegate.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLivModule, Log, All);

class ULivCaptureContextComponent;

class ILivModule : public IModuleInterface
{

public:
	/**
	 * Singleton-like access to LIV's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. LIV might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading LIV on demand if needed
	 */
	static ILivModule& Get()
	{
		return FModuleManager::LoadModuleChecked<ILivModule>("LIV");
	}

	/**
	 * Checks to see if LIV is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if LIV is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("LIV");
	}

	DECLARE_MULTICAST_DELEGATE(FLivModuleCaptureActivation);
	
	FLivModuleCaptureActivation& OnLivCaptureActivated() { return LivCaptureActivatedEvent; }
	FLivModuleCaptureActivation& OnLivCaptureDeactivated() { return LivCaptureDeactivatedEvent; }

	virtual bool IsSDKLoaded() const = 0;

private:

	FLivModuleCaptureActivation LivCaptureActivatedEvent;
	FLivModuleCaptureActivation LivCaptureDeactivatedEvent;
};
