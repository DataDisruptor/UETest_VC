// Copyright 2021 LIV Inc. - MIT License
#include "LivWorldSubsystem.h"
#include "IXRTrackingSystem.h"
#include "LivCaptureContext.h"
#include "LivCaptureMeshClipPlaneNoPostProcess.h"
#include "LivPluginSettings.h"
#include "LivDirector.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogLivWorldSubsystem);

static const FAttachmentTransformRules GDefaultLivAttachmentRules(EAttachmentRule::KeepRelative, false);
static const FDetachmentTransformRules GDefaultLivDetachmentRules(EDetachmentRule::KeepRelative, true);

ULivWorldSubsystem::ULivWorldSubsystem()
	: Super()
	, CameraRoot(nullptr)
	, CaptureComponent(nullptr)
{
}

void ULivWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG(LogLivWorldSubsystem, Log, TEXT("LIV World Subsystem Initialize (%s)."), *GetWorld()->GetName());
}

bool ULivWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	check(Outer);

	if(IsRunningDedicatedServer())
	{
		return false;
	}

	if(UWorld* World = Cast<UWorld>(Outer))
	{
		switch(World->WorldType)
		{
		case EWorldType::Game:
		case EWorldType::PIE:
			return true;
		default:
			return false;
		}
	}

	return true;
}

void ULivWorldSubsystem::Deinitialize()
{
	UE_LOG(LogLivWorldSubsystem, Log, TEXT("LIV World Subsystem Deinitialize (%s)."), *GetWorld()->GetName());

	DestroyCaptureResources();
}

TSubclassOf<ULivCaptureBase> ULivWorldSubsystem::GetCaptureComponentClass() const
{
	ULivPluginSettings* Settings = GetMutableDefault<ULivPluginSettings>();
	if (Settings && Settings->CaptureMethod)
	{
		return Settings->CaptureMethod;
	}
	
	return ULivCaptureMeshClipPlaneNoPostProcess::StaticClass();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
FTransform ULivWorldSubsystem::GetTrackingOriginTransform() const
{
	if (GEngine->XRSystem)
	{
		return GEngine->XRSystem->GetTrackingToWorldTransform();
	}

	return FTransform::Identity;
}

UCameraComponent* ULivWorldSubsystem::GetPlayerCamera() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		const ULocalPlayer* XRPlayer = GEngine->GetFirstGamePlayer(World);

		if (XRPlayer)
		{
			const APlayerController* PlayerController = XRPlayer->GetPlayerController(World);
			if (PlayerController)
			{
				const AActor* PlayerViewTarget = PlayerController->GetViewTarget();

				TArray<UCameraComponent*> CameraComponents;
				PlayerViewTarget->GetComponents<UCameraComponent>(CameraComponents);

				for (UCameraComponent* Camera : CameraComponents)
				{
					if(Camera->IsActive())
					{
						return Camera;
					}
				}
			}
		}
	}

	return nullptr;
}

USceneComponent* ULivWorldSubsystem::GetPlayerCameraParent() const
{
	UCameraComponent* PlayerCamera = GetPlayerCamera();
	
	if(PlayerCamera != nullptr)
	{
		return PlayerCamera->GetAttachParent();
	}
	
	return nullptr;
}

void ULivWorldSubsystem::Capture(FLivCaptureContext& Context)
{
	// check we have our resources - if world changed whilst still capturing we're in a new
	// system and have to create the resources again
	if(CaptureComponent == nullptr || CameraRoot == nullptr)
	{
		CreateCaptureResources();
	}

	// ensure camera is relative to correct origin each frame
	HandleTrackingOrigin();

	// Ensure a director is in the world if one is specified in the plugin settings
	if(CameraController == nullptr)
	{
		const ULivPluginSettings* LivPluginsSettings = GetDefault<ULivPluginSettings>();

		LivPluginsSettings->CameraControllerClass.LoadSynchronous();
		
		if(LivPluginsSettings->CameraControllerClass.IsValid())
		{
			// check if there's one already in the world
			CameraController = Cast<ALivCameraController>(UGameplayStatics::GetActorOfClass(GetWorld(), LivPluginsSettings->CameraControllerClass.Get()));

			// Spawn one if not found
			if (CameraController == nullptr)
			{
				CameraController = GetWorld()->SpawnActor<ALivCameraController>(LivPluginsSettings->CameraControllerClass.Get());
			}
		}
	}
	
	CaptureComponent->Capture(Context);
}

void ULivWorldSubsystem::CreateCaptureResources()
{
	UWorld* World = GetWorld();
	
	// create the camera root that the capture component attaches to
	CameraRoot = NewObject<USceneComponent>(this, "LivCameraRoot");
	CameraRoot->RegisterComponentWithWorld(World);
	
	// create the capture component based on class set in settings
	const TSubclassOf<ULivCaptureBase> CaptureComponentClass = GetCaptureComponentClass();
	CaptureComponent = NewObject<ULivCaptureBase>(CameraRoot, CaptureComponentClass.Get());

	UE_LOG(LogLivWorldSubsystem, Log, TEXT("LIV World Subsystem : Using Capture Class (%s)."), *CaptureComponentClass->GetName());

	// setup/init the capture component
	CaptureComponent->RegisterComponentWithWorld(World);
	CaptureComponent->AttachToComponent(CameraRoot, GDefaultLivAttachmentRules);
	CaptureComponent->OnActivated();

	HandleTrackingOrigin();
}

void ULivWorldSubsystem::DestroyCaptureResources()
{
	if (CaptureComponent)
	{
		CaptureComponent->OnDeactivated();
		CaptureComponent->DestroyComponent();
		CaptureComponent = nullptr;
	}

	if (CameraRoot)
	{
		CameraRoot->DestroyComponent();
		CameraRoot = nullptr;
	}
}

void ULivWorldSubsystem::HandleTrackingOrigin()
{
	USceneComponent* PlayerCameraParent = GetPlayerCameraParent();
	
	if(PlayerCameraParent)
	{
		if(CameraRoot->GetAttachParent() == PlayerCameraParent)
		{
			// already attached to right component, done
			return;
		}
		else
		{
			// if we're attached to something (like a previously possessed pawn) then detach
			if(CameraRoot->GetAttachParent() != nullptr)
			{
				CameraRoot->DetachFromComponent(GDefaultLivDetachmentRules);
			}

			// attach root to players camera parent
			CameraRoot->AttachToComponent(PlayerCameraParent, GDefaultLivAttachmentRules);
			CameraRoot->SetRelativeTransform(FTransform::Identity);
		}
	}
	else
	{
		// UE_LOG(LogLivWorldSubsystem, Warning, TEXT("Cannot find camera parent for tracking origin. Fallback to XR transform."));

		// can't find the player's camera origin so fallback to setting the root to the XR tracking system transform
		const FTransform TrackingTransform = GetTrackingOriginTransform();
		CameraRoot->SetWorldTransform(TrackingTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void ULivWorldSubsystem::Tick()
{
	// check we have our resources - if world changed whilst still capturing we're in a new
	// system and have to create the resources again
	if (CaptureComponent == nullptr || CameraRoot == nullptr)
	{
		CreateCaptureResources();
	}
}


