// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LivNativeWrapper.h"
#include "LivCaptureBase.generated.h"

class UProceduralMeshComponent;
class ULivClipPlane;
class UTextureRenderTarget2D;
struct LIV_InputFrame;

DECLARE_LOG_CATEGORY_EXTERN(LogLivCapture, Log, Log);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLivActivationDelegate);

UCLASS(ClassGroup = LIV, BlueprintType, meta = (BlueprintSpawnableComponent), Abstract)
class LIV_API ULivCaptureBase : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:

	ULivCaptureBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "LIV")
		FLivActivationDelegate OnLivCaptureActivated;

	UPROPERTY(BlueprintAssignable, Category = "LIV")
		FLivActivationDelegate OnLivCaptureDeactivated;

	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		uint32 bOverrideCameraPose:1;

	UPROPERTY(BlueprintReadWrite, Category = "LIV")
		uint32 bOverrideCameraFOV : 1;

	UPROPERTY(BlueprintReadOnly, Category = "LIV")
		FLivInputFrame InputFrame;
	
	UFUNCTION(BlueprintPure, Category = "LIV")
		bool IsLivCapturing() const;

protected:

	// each frame assigned from LIV_IsActive()
	bool bLivActive;

	// Expected render target dimensions
	int32 LivInputFrameWidth;
	int32 LivInputFrameHeight;
	
#ifdef WITH_EDITORONLY_DATA
	// Guard bool to request capture from LIV once
	bool bRequestedCapture;
#endif

public:
		
	virtual void OnActivated();
	virtual void OnDeactivated();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Capture(const struct FLivCaptureContext& Context);

protected:

	virtual void CreateRenderTargets();
	virtual void RecreateRenderTargets();
	virtual void ReleaseRenderTargets();

	void UpdateLivInputFrame(USceneCaptureComponent2D* InSceneCaptureComponent);

	// Set LIV camera parameters on scene capture component
	virtual void SetSceneCaptureComponentParameters(USceneCaptureComponent2D* InSceneCaptureComponent);

	static UTextureRenderTarget2D* CreateRenderTarget2D(UObject* WorldContextObject,
		int32 Width,
		int32 Height,
		FName Name = NAME_None,
		ETextureRenderTargetFormat Format = ETextureRenderTargetFormat::RTF_RGBA8,
		FLinearColor ClearColor = FLinearColor::Black,
		float TargetGamma = 0.0f);
	
public:
	// void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason);
};
