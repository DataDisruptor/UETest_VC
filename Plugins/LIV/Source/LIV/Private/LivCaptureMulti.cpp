// Copyright 2021 LIV Inc. - MIT License


#include "LivCaptureMulti.h"

#include "LivCaptureContext.h"
#include "LivConversions.h"
#include "LivCustomClipPlane.h"
#include "LivSceneViewExtensionMulti.h"
#include "LivPluginSettings.h"

TAutoConsoleVariable<bool> CVarEyeAdaption(TEXT("Liv.Debug.EyeAdaption"),
	true,
	TEXT("Whether eye adaption is on or not.")
);

// Background must precede foreground
static constexpr int32 ForegroundPriority = 0;
static constexpr int32 BackgroundPriority = ForegroundPriority + 1;

ULivCaptureMulti::ULivCaptureMulti(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULivCaptureMulti::OnActivated()
{
	Super::OnActivated();

	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneCapture.EnableViewExtensions"));
	CVar->Set(1);

	if (!SceneViewExtension.IsValid())
	{
		// Null viewport should ensure it doesn't run anywhere yet (unless explicitly gathered)
		SceneViewExtension = FSceneViewExtensions::NewExtension<FLivSceneViewExtensionMulti>(nullptr);
	}

	// 

	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;

	//

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);

	// Create the clip plane mesh
	CameraClipPlane = NewObject<ULivCustomClipPlane>(this, "LivCameraClipPlane");
	CameraClipPlane->RegisterComponentWithWorld(GetWorld());
	CameraClipPlane->AttachToComponent(this, AttachmentRules);
	CameraClipPlane->SetHiddenInGame(false);
	
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

	SceneViewExtension->ClipPlanes = { CameraClipPlane };

	/*SceneViewExtensions.AddUnique(SceneViewExtension);
	SceneCaptureComponent->SceneViewExtensions.AddUnique(SceneViewExtension);*/
}

void ULivCaptureMulti::OnDeactivated()
{
	Super::OnDeactivated();

	CameraClipPlane->DestroyComponent();
	CameraClipPlane = nullptr;

	SceneViewExtension = nullptr;
}

UTextureRenderTarget2D* CreateRenderTarget2D_Alt(UObject* Owner,
	int32 Width,
	int32 Height,
	FName Name /*= NAME_None*/,
	ETextureRenderTargetFormat Format = ETextureRenderTargetFormat::RTF_RGBA8,
	bool bForceLinearGamma = false,
	FLinearColor ClearColor = FLinearColor::Black,
	float TargetGamma = 0.0f)
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
		NewRenderTarget2D->bForceLinearGamma = bForceLinearGamma;
		NewRenderTarget2D->InitAutoFormat(Width, Height);
		NewRenderTarget2D->UpdateResourceImmediate(true);

		return NewRenderTarget2D;
	}

	return nullptr;
}


void ULivCaptureMulti::CreateRenderTargets()
{
	Super::CreateRenderTargets();

	// Background (8bpc)
	BackgroundRenderTarget = CreateRenderTarget2D_Alt(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Background output (8bpc)
	BackgroundOutputRenderTarget = CreateRenderTarget2D_Alt(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"BackgroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground (8bpc)
	ForegroundRenderTarget = CreateRenderTarget2D_Alt(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	// Foreground output (8bpc)
	ForegroundOutputRenderTarget = CreateRenderTarget2D_Alt(
		GetWorld(),
		LivInputFrameWidth,
		LivInputFrameHeight,
		"ForegroundOutputRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA8_SRGB
	);

	EyeAdaptionRenderTarget = CreateRenderTarget2D_Alt(
		GetWorld(),
		1,
		1,
		"LivEyeAdaptionRenderTarget",
		ETextureRenderTargetFormat::RTF_RGBA32f,
		true
	);
}

void ULivCaptureMulti::ReleaseRenderTargets()
{
	Super::ReleaseRenderTargets();

	TextureTarget = nullptr;

	if (BackgroundRenderTarget)
	{
		BackgroundRenderTarget->ReleaseResource();
		BackgroundRenderTarget = nullptr;
	}

	if(BackgroundOutputRenderTarget)
	{
		BackgroundOutputRenderTarget->ReleaseResource();
		BackgroundOutputRenderTarget = nullptr;
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

void ULivCaptureMulti::Capture(const FLivCaptureContext& Context)
{
	Super::Capture(Context);

#if PLATFORM_WINDOWS

	// set our render targets so we can determine if relevant in scene view ext
	SceneViewExtension->BackgroundRenderTarget2D = BackgroundRenderTarget;
	SceneViewExtension->BackgroundOutputRenderTarget = BackgroundOutputRenderTarget;
	SceneViewExtension->ForegroundRenderTarget2D = ForegroundRenderTarget;
	SceneViewExtension->ForegroundOutputRenderTarget2D = ForegroundOutputRenderTarget;
	SceneViewExtension->EyeAdaptionRenderTarget2D = EyeAdaptionRenderTarget;

	UpdateLivInputFrame(this);

	// set scene capture transform / FOV from input frame data
	SetSceneCaptureComponentParameters(SceneCaptureComponent);
	SetSceneCaptureComponentParameters(this);

	// Apply context to scene capture (set hide list)
	Context.ApplyHideLists(this);
	Context.ApplyHideLists(SceneCaptureComponent);

	// Capture full scene with post processing (RGB)
	TextureTarget = BackgroundRenderTarget;
	CaptureSource = SCS_FinalToneCurveHDR;
	bEnableClipPlane = false;
	PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	/*PostProcessSettings.bOverride_AutoExposureMethod = 1;
	PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = 1;
	PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = false;*/
	CaptureSortPriority = BackgroundPriority;
	CaptureSceneDeferred();

	// Calculate clip plane transform
	const auto VROriginTransform = GetAttachParent()->GetComponentTransform();

	const auto ClipPlaneTransform = InputFrame.CameraClipPlaneMatrix;// Convert<LIV_Matrix4x4, FMatrix>(LivInputFrame->clipPlane.transform);
	const auto ClipPlanePosition = VROriginTransform.TransformPosition(ClipPlaneTransform.TransformPosition(FVector::ZeroVector));
	const auto ClipPlaneForward = VROriginTransform.TransformVector(ClipPlaneTransform.TransformVector(FVector::ForwardVector));

	const auto CameraClipPlaneRotation = ClipPlaneForward.Rotation();
	const auto CameraClipPlaneScale = VROriginTransform.GetScale3D() * ClipPlaneTransform.GetScaleVector();

	// Transform camera clip plane mesh
	CameraClipPlane->SetWorldLocationAndRotation(ClipPlanePosition, CameraClipPlaneRotation);
	CameraClipPlane->SetWorldScale3D(CameraClipPlaneScale);
	CameraClipPlane->SetHiddenInGame(false);

	ShowFlags.EyeAdaptation = CVarEyeAdaption.GetValueOnGameThread();
	SceneCaptureComponent->ShowFlags.EyeAdaptation = CVarEyeAdaption.GetValueOnGameThread();

	/*ShowFlags.EyeAdaptation = 1;
	SceneCaptureComponent->ShowFlags.EyeAdaptation = 1;*/
	
	// Capture foreground scene with post processing (RGB)
	SceneCaptureComponent->ClipPlaneBase = ClipPlanePosition;
	SceneCaptureComponent->ClipPlaneNormal = ClipPlaneForward;
	SceneCaptureComponent->bEnableClipPlane = true;
	SceneCaptureComponent->TextureTarget = ForegroundRenderTarget;
	SceneCaptureComponent->CaptureSource = SCS_FinalToneCurveHDR;
	SceneCaptureComponent->PostProcessSettings = GetDefault<ULivPluginSettings>()->PostProcessSettings;
	/*SceneCaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = 1;
	SceneCaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	SceneCaptureComponent->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = 1;
	SceneCaptureComponent->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = false;*/
	SceneCaptureComponent->CaptureSortPriority = ForegroundPriority;
	SceneCaptureComponent->CaptureSceneDeferred();

#endif
}
