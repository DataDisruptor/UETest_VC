// Copyright 2021 LIV Inc. - MIT License
#include "LivEditorCommands.h"

#define LOCTEXT_NAMESPACE "FLivEditorCommands"

void FLivEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "LIV", "LIV", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenWizard, "OpenWizard", "Open LIV Editor Wizard", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE