// Copyright 2021 LIV Inc. - MIT License

#include "LivNativeWrapper.h"

#include "GeneralProjectSettings.h"
#include "IXRTrackingSystem.h"
#include "LivConversions.h"
#include "LivPluginSettings.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interfaces/IPluginManager.h"
#include "Launch/Resources/Version.h"
#include "Engine/Engine.h"

#if !defined(LIV_SUPPORTED)
#if PLATFORM_WINDOWS
#define LIV_SUPPORTED 1
#else
#define LIV_SUPPORTED 0
#endif
#endif


#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "LIV.h"
#include "LIV_GL.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif


DEFINE_LOG_CATEGORY(LogLivNativeWrapper);

static const FString RHID3D11("D3D11");
static const FString RHIOpenGL("OpenGL");

float FLivInputFrame::GetHorizontalFieldOfView() const
{
#if LIV_SUPPORTED
	return ConvertVerticalFOVToHorizontalFOV(HorizontalFieldOfView, Dimensions.X, Dimensions.Y);
#else
	return 0.0f;
#endif
}

///////////////////////////////////////////////////////////////////////

bool FLivNativeWrapper::IsSupported()
{
#if LIV_SUPPORTED
	return true;
#else
	return false;
#endif
}

bool FLivNativeWrapper::StartUp()
{
#if LIV_SUPPORTED

	if(!IsSupported())
	{
		UE_LOG(LogLivNativeWrapper, Log, TEXT("LIV is not supported on this platform."));
		return false;
	}

	if(!LIV_Load())
	{
		UE_LOG(LogLivNativeWrapper, Log, TEXT("LIV did not load, it is likely not installed on this system. Message: %s"), UTF8_TO_TCHAR(LIV_GetError()));
		return false;
	}

	const FString RHIName(GDynamicRHI->GetName());
	UE_LOG(LogLivNativeWrapper, Log, TEXT("Successfully loaded LIV SDK. RHI: %s"), *RHIName);

	if (RHIName == RHID3D11)
	{
		ID3D11Device* Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
		const bool D3D11Initialized = LIV_D3D11_Init(Device);

		if (D3D11Initialized)
		{
			UE_LOG(LogLivNativeWrapper, Log, TEXT("Successfully initialized D3D11 with LIV SDK."));
			return true;
		}
		else
		{
			UE_LOG(LogLivNativeWrapper, Warning, TEXT("Failed to initialize D3D11 with LIV SDK. Error: %s"), UTF8_TO_TCHAR(LIV_GetError()));
			return false;
		}
	}
	else if (RHIName == RHIOpenGL)
	{
		// @TODO: Running with OpenGL needs testing

		const bool OpenGLInitialized = LIV_GL_Init();

		if (OpenGLInitialized)
		{
			UE_LOG(LogLivNativeWrapper, Log, TEXT("Successfully initialized OpenGL with LIV SDK."));
			UE_LOG(LogLivNativeWrapper, Warning, TEXT("LIV's OpenGL support has not been tested."));

			return true;
		}
		else
		{
			UE_LOG(LogLivNativeWrapper, Warning, TEXT("Failed to initialize D3D11 with LIV SDK. Error: %s"), UTF8_TO_TCHAR(LIV_GetError()));
			return false;
		}
	}

#endif

	return false;
}

void FLivNativeWrapper::Shutdown()
{
#if LIV_SUPPORTED
	LIV_Unload();
#endif
}

bool FLivNativeWrapper::IsActive()
{
#if LIV_SUPPORTED
	return LIV_IsActive();
#else
	return false;
#endif
}

void FLivNativeWrapper::Start()
{
#if LIV_SUPPORTED
	LIV_Start();
	SubmitApplicationInformation();
#endif
}

FString FLivNativeWrapper::GetVersionString()
{
#if LIV_SUPPORTED
	return FString(ANSI_TO_TCHAR(LIV_GetCSDKVersion()));
	#else
	return FString(TEXT("Unsupported"));
#endif
}

#if LIV_SUPPORTED
bool FLivNativeWrapper::GetInputFrame(FLivInputFrame& OutInputFrame)
{
	LIV_InputFrame LivInputFrame;

	// try to get input frame, else tear down
	if (!LIV_GetInputFrame(&LivInputFrame))
	{
		UE_LOG(LogLivNativeWrapper, Warning, TEXT("LIV capture failed as unable to obtain input frame."));
		return false;
	}
	
	OutInputFrame.CameraLocation = ConvertPosition<LIV_Vector3, FVector>(LivInputFrame.pose.local_position);
	OutInputFrame.CameraRotation = Convert<LIV_Quaternion, FQuat>(LivInputFrame.pose.local_rotation);

	OutInputFrame.Dimensions = FIntPoint(LivInputFrame.pose.width, LivInputFrame.pose.height);
	OutInputFrame.HorizontalFieldOfView = ConvertVerticalFOVToHorizontalFOV(LivInputFrame.pose.verticalFieldOfView, LivInputFrame.pose.width, LivInputFrame.pose.height);

	OutInputFrame.CameraClipPlaneMatrix = Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame.clipPlane.transform);
	OutInputFrame.FloorClipPlaneMatrix = Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame.GroundPlane.transform);

	OutInputFrame.bFloorClipPlaneEnabled = LivInputFrame.features & LIV_FEATURES::LIV_FEATURES_GROUND_CLIP_PLANE;

	return true;
}
#else
bool FLivNativeWrapper::GetInputFrame(FLivInputFrame& OutInputFrame)
{
	return false;
}
#endif


#if LIV_SUPPORTED
bool FLivNativeWrapper::UpdateInputFrame(
	FLivInputFrame& InOutInputFrame,
	USceneCaptureComponent2D* OverridingSceneCaptureComponent)
{

#if UE_BUILD_DEBUG
	check(InOutInputFrame.Dimensions.X > 0);
	check(InOutInputFrame.Dimensions.Y > 0);
#endif

	LIV_InputFrame LivInputFrame;
	LIV_ClearInputFrame(&LivInputFrame);

	// if the component is provided then we want to override
	if(OverridingSceneCaptureComponent)
	{
		LivInputFrame.pose_priority = LIV_GAME_PRIORITY;
		LivInputFrame.pose.local_position = ConvertPosition<FVector, LIV_Vector3>(OverridingSceneCaptureComponent->GetRelativeLocation());
		LivInputFrame.pose.local_rotation = Convert<FQuat, LIV_Quaternion>(OverridingSceneCaptureComponent->GetRelativeRotation().Quaternion());
		LivInputFrame.pose.verticalFieldOfView = ConvertHorizontalFOVToVerticalFOV(
			OverridingSceneCaptureComponent->FOVAngle,
			InOutInputFrame.Dimensions.X,
			InOutInputFrame.Dimensions.Y);

		const float NearClippingPlane = (OverridingSceneCaptureComponent->bOverride_CustomNearClippingPlane) ? OverridingSceneCaptureComponent->CustomNearClippingPlane : GNearClippingPlane;
		LivInputFrame.pose.projectionMatrix = CreateLivProjectionMatrix(InOutInputFrame.Dimensions, OverridingSceneCaptureComponent->FOVAngle, NearClippingPlane);
	}

	const LIV_InputFrame* NewLivInputFrame = LIV_UpdateInputFrame(&LivInputFrame);

	if(NewLivInputFrame == nullptr)
	{
		return false;
	}

#if UE_BUILD_DEBUG

	// In debug if we requested our camera pose and LIV acquiesced
	// Then check the data returned is the same (within a tolerance)

	if ((OverridingSceneCaptureComponent != nullptr) && LivInputFrame.pose_priority == LIV_GAME_PRIORITY)
	{
		for (int32 Idx{ 0 }; Idx < 16; ++Idx)
		{
			check(FMath::IsNearlyEqual(LivInputFrame.pose.projectionMatrix.data[Idx], NewLivInputFrame->pose.projectionMatrix.data[Idx]));
		}

		check(FMath::IsNearlyEqual(LivInputFrame.pose.verticalFieldOfView, NewLivInputFrame->pose.verticalFieldOfView));

		for (int32 Idx{ 0 }; Idx < 3; ++Idx)
		{
			check(FMath::IsNearlyEqual(LivInputFrame.pose.local_position.data[Idx], NewLivInputFrame->pose.local_position.data[Idx]));
		}

		for (int32 Idx{ 0 }; Idx < 4; ++Idx)
		{
			check(FMath::IsNearlyEqual(LivInputFrame.pose.local_rotation.data[Idx], NewLivInputFrame->pose.local_rotation.data[Idx]));
		}
	}

#endif

	InOutInputFrame.CameraLocation = ConvertPosition<LIV_Vector3, FVector>(NewLivInputFrame->pose.local_position);
	InOutInputFrame.CameraRotation = Convert<LIV_Quaternion, FQuat>(NewLivInputFrame->pose.local_rotation);

	InOutInputFrame.Dimensions = FIntPoint(NewLivInputFrame->pose.width, NewLivInputFrame->pose.height);
	InOutInputFrame.HorizontalFieldOfView = ConvertVerticalFOVToHorizontalFOV(NewLivInputFrame->pose.verticalFieldOfView, NewLivInputFrame->pose.width, NewLivInputFrame->pose.height);

	InOutInputFrame.CameraClipPlaneMatrix = Convert<LIV_Matrix4x4, FMatrix>(NewLivInputFrame->clipPlane.transform);
	InOutInputFrame.FloorClipPlaneMatrix = Convert<LIV_Matrix4x4, FMatrix>(NewLivInputFrame->GroundPlane.transform);

	InOutInputFrame.bFloorClipPlaneEnabled = NewLivInputFrame->features & LIV_FEATURES::LIV_FEATURES_GROUND_CLIP_PLANE;

	return true;
}
#else
bool FLivNativeWrapper::UpdateInputFrame(
	FLivInputFrame& InOutInputFrame,
	USceneCaptureComponent2D* OverridingSceneCaptureComponent)
{
	return false;
}
#endif

void FLivNativeWrapper::SubmitApplicationInformation()
{
#if LIV_SUPPORTED

	const auto LivPlugin = IPluginManager::Get().FindPlugin(TEXT("Liv"));

	const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();

	const FString EngineName = EPIC_PRODUCT_IDENTIFIER;
	const FString EngineVersion = ENGINE_VERSION_STRING;
	const FString GraphicsApi = GDynamicRHI->GetName();

	const ULivPluginSettings& PluginSettings = *GetDefault<ULivPluginSettings>();

	const FString XRDevice = GEngine && GEngine->XRSystem.IsValid() ? GEngine->XRSystem->GetSystemName().ToString() : TEXT("Unknown");

	const LIV_ApplicationInformation ApplicationInformation
	{
		TCHAR_TO_ANSI(*ProjectSettings.ProjectName),
		TCHAR_TO_ANSI(*ProjectSettings.ProjectVersion),
		TCHAR_TO_ANSI(*EngineName),
		TCHAR_TO_ANSI(*EngineVersion),
		TCHAR_TO_ANSI(*GraphicsApi),
		TCHAR_TO_ANSI(*(PluginSettings.TrackingId)),
		TCHAR_TO_ANSI(*LivPlugin->GetDescriptor().VersionName),	
		TCHAR_TO_ANSI(*XRDevice)
	};
	LIV_SubmitApplicationInformation(&ApplicationInformation);
#endif
}
