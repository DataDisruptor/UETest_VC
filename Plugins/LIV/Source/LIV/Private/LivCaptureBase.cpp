// Copyright 2021 LIV Inc. - MIT License
#include "LivCaptureBase.h"

#include "BasePassRendering.h"
#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "LivConversions.h"
#include "LivShaders.h"

// @TODO: move to native wrapper
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "LIV.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "LivLocalPlayerSubsystem.h"
#include "LivPluginSettings.h"
#include "LivWorldSubsystem.h"

#ifdef WITH_EDITORONLY//Editor only
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "LivEditorSettings.h"
#endif

#if PLATFORM_WINDOWS
TAutoConsoleVariable<int32> CVarLivColorSpace(TEXT("Liv.ColorSpace"),
	LIV_TEXTURE_COLOR_SPACE_SRGB,
	TEXT("Colorspace: Linear (1), sRGB (2)")
);
#endif

DEFINE_LOG_CATEGORY(LogLivCapture);

ULivCaptureBase::ULivCaptureBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bOverrideCameraPose(false)
	, bLivActive(false)
	, LivInputFrameWidth(0)
	, LivInputFrameHeight(0)
#if WITH_EDITORONLY_DATA
	, bRequestedCapture(false)
#endif

{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

bool ULivCaptureBase::IsLivCapturing() const
{
	return bLivActive;
}

void ULivCaptureBase::OnActivated()
{
#if PLATFORM_WINDOWS
	const bool bSuccess = FLivNativeWrapper::GetInputFrame(InputFrame);

	if(!bSuccess)
	{
		UE_LOG(LogLivCapture, Warning, TEXT("LIV capture failed as unable to obtain input frame."));
		return;
	}

	LivInputFrameWidth = InputFrame.Dimensions.X;
	LivInputFrameHeight = InputFrame.Dimensions.Y;

	// create render targets needed for rendering
	CreateRenderTargets();

	// track LIV is active
	bLivActive = true;

	// broadcast callback for when activated
	OnLivCaptureActivated.Broadcast();

#endif
}

void ULivCaptureBase::OnDeactivated()
{
#if PLATFORM_WINDOWS

	// release render targets used for capture
	ReleaseRenderTargets();

	// track LIV inactive
	bLivActive = false;

	// broadcast callback for when deactivated
	OnLivCaptureDeactivated.Broadcast();

#endif
}

void ULivCaptureBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UWorld* World = GetWorld())
	{
		if (ULocalPlayer* LocalPlayer = GEngine->GetFirstGamePlayer(World))
		{
			if (ULivLocalPlayerSubsystem* LivLocalPlayerSubsystem = LocalPlayer->GetSubsystem<ULivLocalPlayerSubsystem>())
			{
				if (LivLocalPlayerSubsystem->IsCaptureActive())
				{
					FLivCaptureContext CaptureContext = LivLocalPlayerSubsystem->GetCaptureContext();

					if (ULivWorldSubsystem* LivWorldSubsystem = World->GetSubsystem<ULivWorldSubsystem>())
					{
						LivWorldSubsystem->Capture(CaptureContext);
					}
				}
			}
		}
	}
}

void ULivCaptureBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bLivActive)
	{
		OnDeactivated();
	}
}

void ULivCaptureBase::CreateRenderTargets()
{
	// Implement in subclass	
}

void ULivCaptureBase::RecreateRenderTargets()
{
	ReleaseRenderTargets();
	CreateRenderTargets();
}

void ULivCaptureBase::ReleaseRenderTargets()
{
	// Implement in subclass	
}

void ULivCaptureBase::UpdateLivInputFrame(USceneCaptureComponent2D* InSceneCaptureComponent)
{
	const bool bSuccess = FLivNativeWrapper::UpdateInputFrame(InputFrame, bOverrideCameraPose ? InSceneCaptureComponent : nullptr);

	if(!bSuccess)
	{
		UE_LOG(LogLivCapture, Warning, TEXT("Failed to update input frame."));
		return;
	}

	// Check if output dimensions changed
	if (LivInputFrameWidth != InputFrame.Dimensions.X || LivInputFrameHeight != InputFrame.Dimensions.Y)
	{
		LivInputFrameWidth = InputFrame.Dimensions.X;
		LivInputFrameHeight = InputFrame.Dimensions.Y;
		RecreateRenderTargets();
	}
}

void ULivCaptureBase::SetSceneCaptureComponentParameters(USceneCaptureComponent2D* InSceneCaptureComponent)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// Guarded to prevent debug settings in shipping builds
	const ULivPluginSettings* Settings = GetDefault<ULivPluginSettings>();
	if (Settings->bUseDebugCamera)
	{
		InSceneCaptureComponent->SetWorldLocation(Settings->DebugCameraWorldLocation);
		InSceneCaptureComponent->SetWorldRotation(Settings->DebugCameraWorldRotation);
		InSceneCaptureComponent->FOVAngle = Settings->DebugCameraHorizontalFOV;
		return;
	}
#endif

	InSceneCaptureComponent->SetRelativeLocation(InputFrame.CameraLocation);
	InSceneCaptureComponent->SetRelativeRotation(InputFrame.GetCameraRotator());
	InSceneCaptureComponent->FOVAngle = InputFrame.HorizontalFieldOfView;
}

void ULivCaptureBase::Capture(const struct FLivCaptureContext& Context)
{
	// if not active, return early
	if (!bLivActive)
	{
		// editor auto-capture handling (see settings)
#if WITH_EDITORONLY_DATA && PLATFORM_WINDOWS
		if (!bRequestedCapture && GetMutableDefault<ULivPluginSettings>()->bAutoCaptureInEditor)
		{
			LIV_RequestCapture();
			bRequestedCapture = true;
		}
#endif
		return;
	}
}

UTextureRenderTarget2D* ULivCaptureBase::CreateRenderTarget2D(UObject* Owner,
	int32 Width,
	int32 Height,
	FName Name /*= NAME_None*/,
	ETextureRenderTargetFormat Format /*= ETextureRenderTargetFormat::RTF_RGBA8*/,
	FLinearColor ClearColor /*= FLinearColor::Black*/,
	float TargetGamma /*= 0.0f*/)
{
	/*UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);*/

	if (Width > 0 && Height > 0/* && World*/)
	{
		UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), Name);
		check(NewRenderTarget2D);
		NewRenderTarget2D->RenderTargetFormat = Format;
		NewRenderTarget2D->ClearColor = ClearColor;
		NewRenderTarget2D->bAutoGenerateMips = false;
		NewRenderTarget2D->TargetGamma = TargetGamma;
		NewRenderTarget2D->bForceLinearGamma = true;
		NewRenderTarget2D->InitAutoFormat(Width, Height);
		NewRenderTarget2D->UpdateResourceImmediate(true);

		return NewRenderTarget2D;
	}

	return nullptr;
}
