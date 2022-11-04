// Copyright 2021 LIV Inc. - MIT License
#pragma once

#pragma once
#include "Framework/Commands/Commands.h"
#include "LivEditorStyle.h"


class FLivEditorCommands : public TCommands<FLivEditorCommands>
{
public:

	FLivEditorCommands()
		: TCommands<FLivEditorCommands>
		(
			TEXT("LIVEditor"),
			NSLOCTEXT("Contexts", "LIVEditor", "LIV Plugin"),
			NAME_None,
			FLivEditorStyle::Get().GetStyleSetName()
			) {}

	virtual void RegisterCommands() override;

public:

	TSharedPtr<FUICommandInfo> OpenPluginWindow;
	TSharedPtr<FUICommandInfo> OpenWizard;
};
