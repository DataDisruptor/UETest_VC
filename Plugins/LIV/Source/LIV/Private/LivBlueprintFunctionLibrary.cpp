// Copyright 2021 LIV Inc. - MIT License
#include "LivBlueprintFunctionLibrary.h"

#include "IXRCamera.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"

void ULivBlueprintFunctionLibrary::OffsetCameraPoseForEye(
	ELivEye Eye, 
	const FVector& CameraLocation,
	const FRotator& CameraRotation, 
	FVector& EyeLocation, 
	FRotator& EyeRotation)
{
	EyeLocation = CameraLocation;
	EyeRotation = CameraRotation;

	if(Eye == ELivEye::Center)
	{
		return;
	}
	
	if (GEngine && GEngine->XRSystem.IsValid() && GEngine->StereoRenderingDevice.IsValid() && GEngine->StereoRenderingDevice->IsStereoEnabled())
	{
		const auto XRCamera = GEngine->XRSystem->GetXRCamera();
		if (XRCamera.IsValid())
		{
			const auto StereoscopicPass = Eye == ELivEye::Left ? EStereoscopicPass::eSSP_LEFT_EYE : EStereoscopicPass::eSSP_RIGHT_EYE;
			XRCamera->CalculateStereoCameraOffset(StereoscopicPass, EyeRotation, EyeLocation);
		}
	}
}

UTextureRenderTarget2D* ULivBlueprintFunctionLibrary::CreateRenderTarget2D(
	UObject* WorldContextObject, 
	int32 Width, 
	int32 Height,
	bool bForceLinearGamma, 
	FName Name, 
	ETextureRenderTargetFormat Format, 
	FLinearColor ClearColor, 
	float TargetGamma)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (Width > 0 && Height > 0 && World)
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


