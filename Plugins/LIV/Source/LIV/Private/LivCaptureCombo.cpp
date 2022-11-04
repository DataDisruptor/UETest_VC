// Copyright 2021 LIV Inc. - MIT License


#include "LivCaptureCombo.h"

#include "LivCaptureContext.h"
#include "LivCustomClipPlane.h"
#include "LivConversions.h"
#include "LivSceneViewExtensionCombo.h"
#include "LivPluginSettings.h"

ULivCaptureCombo::ULivCaptureCombo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void ULivCaptureCombo::OnActivated()
{
	Super::OnActivated();

	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneCapture.EnableViewExtensions"));
	CVar->Set(1);

	if (!SceneViewExtension.IsValid())
	{
		// Null viewport should ensure it doesn't run anywhere yet (unless explicitly gathered)
		SceneViewExtension = FSceneViewExtensions::NewExtension<FLivSceneViewExtensionCombo>(nullptr);
	}

	// 

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);

	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(this, "LivSceneCaptureComponent");
	SceneCaptureComponent->RegisterComponentWithWorld(GetWorld());
	SceneCaptureComponent->AttachToComponent(GetAttachParent(), AttachmentRules);
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->bAlwaysPersistRenderingState = true;
	SceneCaptureComponent->SetRelativeLocation(FVector::ZeroVector);
	SceneCaptureComponent->SetRelativeRotation(FRotator::ZeroRotator);
	SceneCaptureComponent->SetRelativeScale3D(FVector::OneVector);
}

void ULivCaptureCombo::OnDeactivated()
{
	Super::OnDeactivated();

	SceneViewExtension = nullptr;
}

void ULivCaptureCombo::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background output (8bpc)
	BackgroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground (8bpc)
	ForegroundRenderTarget = CreateRenderTarget2D(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	if (GetDefault<ULivPluginSettings>()->bTransparency)
	{
		// Foreground output (8bpc)
		ForegroundOutputRenderTarget = CreateRenderTarget2D(
			GetWorld(),
			LivInputFrameWidth,
			LivInputFrameHeight,
			"ForegroundRenderTarget",
			ETextureRenderTargetFormat::RTF_RGBA8_SRGB
		);
	}
}

void ULivCaptureCombo::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	TextureTarget = nullptr;

	if (BackgroundRenderTarget)
	{
		BackgroundRenderTarget->ReleaseResource();
		BackgroundRenderTarget = nullptr;
	}

	if (ForegroundRenderTarget)
	{
		ForegroundRenderTarget->ReleaseResource();
		ForegroundRenderTarget = nullptr;
	}

	if (ForegroundOutputRenderTarget)
	{
		ForegroundOutputRenderTarget->ReleaseResource();
		ForegroundOutputRenderTarget = nullptr;
	}
}

void ULivCaptureCombo::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	// set our render targets so we can determine if relevant in scene view ext
	SceneViewExtension->BackgroundRenderTarget2D = BackgroundRenderTarget;
	SceneViewExtension->ForegroundRenderTarget2D = ForegroundRenderTarget;
	//SceneViewExtension->ForegroundOutputRenderTarget2D = ForegroundOutputRenderTarget;
	

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(SceneCaptureComponent);
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);
	Context.ApplyHideLists(SceneCaptureComponent);

	// Capture full scene with post processing (RGB)
	TextureTarget = BackgroundRenderTarget;
	CaptureSource = SCS_FinalColorLDR;
	bEnableClipPlane = false;
	PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	CaptureScene();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto ClipPlaneTransform = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto ClipPlanePosition = VROriginTransform.TransformPosition(ClipPlaneTransform.TransformPosition(FVector::ZeroVector));
	const auto ClipPlaneForward = VROriginTransform.TransformVector(ClipPlaneTransform.TransformVector(FVector::ForwardVector));

	// Capture foreground scene with post processing (RGB)
	SceneCaptureComponent->ClipPlaneBase = ClipPlanePosition;
	SceneCaptureComponent->ClipPlaneNormal = ClipPlaneForward;
	SceneCaptureComponent->bEnableClipPlane = true;
	SceneCaptureComponent->TextureTarget = ForegroundRenderTarget;
	SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
	//SceneCaptureComponent->ShowFlags.SkyLighting = 0u;
	// NOTE: disables SkyAtmosphereEditor pass in editor (annoying debug text in render)
	SceneCaptureComponent->ShowFlags.Atmosphere = 0;
	SceneCaptureComponent->PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	SceneCaptureComponent->CaptureScene();

#endif
}
