// Copyright 2021 LIV Inc. - MIT License
#include "LivPluginSettings.h"

#include "LivCaptureMeshClipPlanePostProcess.h"
#include "LivCaptureSingle.h"
#include "Engine/RendererSettings.h"
#include "Misc/MessageDialog.h"

#if WITH_EDITOR
#include "ISettingsEditorModule.h"
#endif

#define LOCTEXT_NAMESPACE "LivPluginSettings"

ULivPluginSettings::ULivPluginSettings()
	: CaptureMethod(ULivCaptureSingle::StaticClass())
	, bBackgroundOnly(false)
	, bTransparency(false)
	, PreExposure(1.0f)
	, bUseDebugCamera(false)
	, DebugCameraHorizontalFOV(90.0f)
	, bUseDebugCameraClipPlane(false)
	, bUseDebugFloorClipPlane(false)
#if WITH_EDITOR
	, bAutoCaptureInEditor(false)
#endif
	, CameraControllerClass(nullptr)
	, CaptureSource(SCS_FinalToneCurveHDR)
	, SceneViewExtensionCaptureStage(ELivSceneViewExtensionCaptureStage::AfterFXAA)
{

}

#if WITH_EDITOR
void ULivPluginSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULivPluginSettings, CaptureMethod))
	{
		if(CaptureMethod == nullptr)
		{
			CaptureMethod = ULivCaptureMeshClipPlanePostProcess::StaticClass();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULivPluginSettings, bTransparency))
	{
		if (bTransparency)
		{
			URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
			if(RendererSettings->bGlobalClipPlane == 0u)
			{
				const FText Message = FText::FromString(TEXT("Global Clip Plane support must be enabled to use foreground transparency which is not currently enabled. We can enable this setting for you but be aware that it can have a performance impact of around 15%. Do you wish to continue?"));
				const FText Title = FText::FromString(TEXT("Enable Global Clip Plane?"));
				const EAppReturnType::Type DialogResult = FMessageDialog::Open(EAppMsgType::OkCancel, Message, &Title);

				if(DialogResult == EAppReturnType::Cancel)
				{
					bTransparency = false;
				}
				else if(DialogResult == EAppReturnType::Ok)
				{
					RendererSettings->bGlobalClipPlane = 1u;
					RendererSettings->UpdateDefaultConfigFile();

					FModuleManager::GetModuleChecked<ISettingsEditorModule>("SettingsEditor").OnApplicationRestartRequired();
				}
			}
		}

		// check again, if bTransparency is still on then make sure bBackgroundOnly is off
		if (bTransparency && bBackgroundOnly)
		{
			bBackgroundOnly = false;
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BackgroundOnlyAutoDisabled", "Background only rendering will be automatically turned off due to enabling transparency."));
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULivPluginSettings, bBackgroundOnly))
	{
		if(bBackgroundOnly && bTransparency)
		{
			bTransparency = false;
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("TransparencyAutoDisabled", "Transparency will be automatically turned off due to enabling background only rendering."));
		}
	}
}
#endif
