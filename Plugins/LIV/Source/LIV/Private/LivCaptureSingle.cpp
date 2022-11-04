// Copyright 2021 LIV Inc. - MIT License

#include "LivCaptureSingle.h"

#include "LivCaptureContext.h"
#include "LivConversions.h"
#include "LivSceneViewExtensionSingle.h"
#include "LivCustomClipPlane.h"
#include "LivPluginSettings.h"

ULivCaptureSingle::ULivCaptureSingle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CameraClipPlane(nullptr)
	, FloorClipPlane(nullptr)
	, BackgroundOutputRenderTarget(nullptr)
{
}

void ULivCaptureSingle::OnActivated()
{
	Super::OnActivated();

	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneCapture.EnableViewExtensions"));
	CVar->Set(1);

	if (!SceneViewExtension.IsValid())
	{
		// Null viewport should ensure it doesn't run anywhere yet (unless explicitly gathered)
		SceneViewExtension = FSceneViewExtensions::NewExtension<FLivSceneViewExtensionSingle>(nullptr);
	}

	// 

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);

	// Create the clip plane mesh
	CameraClipPlane = NewObject<ULivCustomClipPlane>(this, "LivCameraClipPlane");
	CameraClipPlane->RegisterComponentWithWorld(GetWorld());
	CameraClipPlane->AttachToComponent(this, AttachmentRules);
	CameraClipPlane->SetHiddenInGame(false);

	// Create the floor clip plane mesh
	FloorClipPlane = NewObject<ULivCustomClipPlane>(this, "LivFloorClipPlane");
	FloorClipPlane->RegisterComponentWithWorld(GetWorld());
	FloorClipPlane->AttachToComponent(this, AttachmentRules);
	FloorClipPlane->SetHiddenInGame(false);

	SceneViewExtension->ClipPlanes = { CameraClipPlane , FloorClipPlane };
}

void ULivCaptureSingle::OnDeactivated()
{
	Super::OnDeactivated();

	SceneViewExtension = nullptr;
	
	CameraClipPlane->DestroyComponent();
	CameraClipPlane = nullptr;

	FloorClipPlane->DestroyComponent();
	FloorClipPlane = nullptr;
}

void ULivCaptureSingle::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background output (8bpc)
	BackgroundOutputRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);
}

void ULivCaptureSingle::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	TextureTarget = nullptr;
	
	if (BackgroundOutputRenderTarget)
	{
		BackgroundOutputRenderTarget->ReleaseResource();
		BackgroundOutputRenderTarget = nullptr;
	}
}

void ULivCaptureSingle::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	// set our render target so we can determine if relevant in scene view ext
	SceneViewExtension->RenderTarget2D = BackgroundOutputRenderTarget;

	// make sure global clip plane is off for all captures
	bEnableClipPlane = false;

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto CameraClipPlaneMatrix = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto CameraClipPlanePosition = VROriginTransform.TransformPosition(CameraClipPlaneMatrix.TransformPosition(FVector::ZeroVector));
	const auto CameraClipPlaneForward = VROriginTransform.TransformVector(CameraClipPlaneMatrix.TransformVector(FVector::ForwardVector));
	const auto CameraClipPlaneRotation = CameraClipPlaneForward.Rotation();
	const auto CameraClipPlaneScale = VROriginTransform.GetScale3D() * CameraClipPlaneMatrix.GetScaleVector();

	// Transform camera clip plane mesh
	CameraClipPlane->SetWorldLocationAndRotation(CameraClipPlanePosition, CameraClipPlaneRotation);
	CameraClipPlane->SetWorldScale3D(CameraClipPlaneScale);
	CameraClipPlane->SetHiddenInGame(false);

	const ULivPluginSettings* PluginSettings = GetDefault<ULivPluginSettings>();
	if(PluginSettings->bUseDebugCameraClipPlane)
	{
		CameraClipPlane->SetWorldTransform(PluginSettings->DebugCameraClipPlaneTransform);
	}

	// @TODO: this doesn't seem to be working? Should pretty much always be on
	if (InputFrame.bFloorClipPlaneEnabled)
	{
		// Transform floor clip plane mesh
		const auto FloorClipPlaneMatrix = InputFrame.FloorClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->GroundPlane.transform);
		const auto FloorClipPlanePosition = VROriginTransform.TransformPosition(FloorClipPlaneMatrix.TransformPosition(FVector::ZeroVector));
		const auto FloorClipPlaneForward = VROriginTransform.TransformVector(FloorClipPlaneMatrix.TransformVector(FVector::ForwardVector));
		const auto FloorClipPlaneRotation = FloorClipPlaneForward.Rotation();
		const auto FloorClipPlaneScale = VROriginTransform.GetScale3D() * FloorClipPlaneMatrix.GetScaleVector();

		FloorClipPlane->SetWorldLocationAndRotation(FloorClipPlanePosition, FloorClipPlaneRotation);
		FloorClipPlane->SetWorldScale3D(FloorClipPlaneScale);
		FloorClipPlane->SetHiddenInGame(false);

		if (PluginSettings->bUseDebugFloorClipPlane)
		{
			FloorClipPlane->SetWorldTransform(PluginSettings->DebugFloorClipPlaneTransform);
		}
	}
	else
	{
		FloorClipPlane->SetHiddenInGame(true);
	}

	// Capture Background
	PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	TextureTarget = BackgroundOutputRenderTarget;
	CaptureSource = GetDefault<ULivPluginSettings>()->CaptureSource;
	CaptureSceneDeferred();

#endif
}
