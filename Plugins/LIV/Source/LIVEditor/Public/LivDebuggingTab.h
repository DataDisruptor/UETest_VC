// Copyright 2021 LIV Inc. - MIT License
#pragma once

#include "CoreMinimal.h"

#include "Delegates/IDelegateInstance.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STileView.h"
#include "Engine/TextureRenderTarget2D.h"

class ULivLocalPlayerSubsystem;
class ULivCaptureBase;
class UTextureRenderTarget2D;

struct FLivRenderTargetItem final
{
	FLivRenderTargetItem(UTextureRenderTarget2D* InRenderTarget2D, 
		FVector4 InDepthSwitch = FVector4(0,0,0,0),
		FVector4 InMaskSwitch = FVector4(0, 0, 0, 0))
		: RenderTarget(InRenderTarget2D)
		, DepthSwitch(InDepthSwitch)
		, MaskSwitch(InMaskSwitch)
	{}

	FLivRenderTargetItem()
		: RenderTarget(nullptr)
		, DepthSwitch(0, 0, 0, 0)
		, MaskSwitch(0, 0, 0, 0)
	{}
	
	TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget;
	FVector4 DepthSwitch;
	FVector4 MaskSwitch;
};

using FLivRenderTargetItemPtr = TSharedPtr<FLivRenderTargetItem>;
using FLivRenderTargetItemRef = TSharedRef<FLivRenderTargetItem>;

class SLivDebuggingTab : public SCompoundWidget
{
public:
	static void RegisterNomadTabSpawner(TSharedRef<FWorkspaceItem> InWorkspaceItem);
	static void UnregisterNomadTabSpawner();

public:

	SLATE_BEGIN_ARGS(SLivDebuggingTab){}
	SLATE_END_ARGS()

	SLivDebuggingTab();
	virtual ~SLivDebuggingTab();

	void Construct(const FArguments& InArgs);

private:

	void OnLivCaptureActivated();
	void OnLivCaptureDeactivated();
	
private:
	FDelegateHandle LivCaptureActivatedHandle;
	FDelegateHandle LivCaptureDeactivatedHandle;
	
	static FDelegateHandle LevelEditorTabManagerChangedHandle;

	TSharedPtr<class SLivCaptureDebugView> CaptureDebugView;
};

class SLivCaptureDebugView : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLivCaptureDebugView) {}
	SLATE_ARGUMENT(TWeakObjectPtr<ULivCaptureBase>, CaptureComponent)
	SLATE_END_ARGS()

	SLivCaptureDebugView();
	~SLivCaptureDebugView();

	void Construct(const FArguments& InArgs);

	FText GetCaptureMethodName() const;
	TSharedRef<SWidget> GenerateWidgetForCaptureClassesCombo(const TSharedPtr<FString> ClassName) const;
	void CaptureClassChanged(const TSharedPtr<FString> ClassName, ESelectInfo::Type SelectInfo);

	TArray<TSharedPtr<FString>> CaptureClasses;
	FString CaptureClass;
	TSharedPtr<SHorizontalBox> InputsContainer;
	TSharedPtr<SHorizontalBox> IntermediatesContainer;
	TSharedPtr<SHorizontalBox> OutputsContainer;
	TWeakObjectPtr<ULivCaptureBase> CaptureComponent;
};

class SLivCaptureDebugViewItem : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SLivCaptureDebugViewItem) {}
	SLATE_END_ARGS()

	SLivCaptureDebugViewItem();
	virtual ~SLivCaptureDebugViewItem();
	
	void Construct(const FArguments& InArgs, const FLivRenderTargetItemRef& InItem);
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:

	FReply OnCalculateMaxDepth();

	bool IsDepth() const;

	TOptional<float> GetMaxDepth() const;
	void SetMaxDepth(float NewMaxDepth);

	const FSlateBrush* GetBrush() const;
	FText GetRenderTargetName() const;

	FReply OnClick() const;
	
	TSharedPtr<struct FSlateImageBrush> Brush;
	class UMaterialInstanceDynamic* RenderTargetDisplayDynamicMaterialInstance;
	FLivRenderTargetItemPtr Item;
	float MaxDepth;

};
