// Copyright 2021 LIV Inc. - MIT License
#include "LivDebuggingTab.h"
#include "LevelEditor.h"
#include "LivEditorStyle.h"
#include "LivCaptureBase.h"
#include "LivModule.h"
#include "SGraphPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "LivBlueprintFunctionLibrary.h"
#include "LivCaptureMeshClipPlaneNoPostProcess.h"
#include "LivEditorModule.h"
#include "LivLocalPlayerSubsystem.h"
#include "LivWorldSubsystem.h"
#include "LivEditorReflectionUtils.h"
#include "LivPluginSettings.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "LivRenderTargetVisualiser"

static ULivLocalPlayerSubsystem* GetLivLocalPlayerSubsystem()
{
	for (TObjectIterator<ULivLocalPlayerSubsystem> It; It; ++It)
	{
		if (It->IsValidLowLevel())
		{
			return *It;
		}
	}

	return nullptr;
}


FDelegateHandle SLivDebuggingTab::LevelEditorTabManagerChangedHandle;

namespace LivDebuggingTab
{
	static const FName LivRenderTargetVisualiserApp = FName("LivDebuggingTab");
	static const FName LevelEditorModuleName("LevelEditor");

	TSharedRef<SDockTab> CreateLivDebuggingTab(const FSpawnTabArgs& Args)
	{
		FLivEditorModule::Get().GetAnalytics()->RecordEvent(TEXT("DEBUGGING_TAB_OPENED"));
		
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SLivDebuggingTab)
			];
	}
}


void SLivDebuggingTab::RegisterNomadTabSpawner(TSharedRef<FWorkspaceItem> InWorkspaceItem)
{
	auto RegisterTabSpawner = [InWorkspaceItem]()
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LivDebuggingTab::LevelEditorModuleName);
		TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();

		LevelEditorTabManager->RegisterTabSpawner(LivDebuggingTab::LivRenderTargetVisualiserApp,
			FOnSpawnTab::CreateStatic(&LivDebuggingTab::CreateLivDebuggingTab))
			.SetDisplayName(LOCTEXT("TabTitle", "LIV Debugging"))
			.SetTooltipText(LOCTEXT("TabTooltipText", "Display render targets used for LIV capturing for debugging"))
			.SetGroup(InWorkspaceItem)
			.SetIcon(FSlateIcon(FLivEditorStyle::GetStyleSetName(), "LIVEditor.Icons.Small"));
	};

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LivDebuggingTab::LevelEditorModuleName);
	if (LevelEditorModule.GetLevelEditorTabManager())
	{
		RegisterTabSpawner();
	}
	else
	{
		LevelEditorTabManagerChangedHandle = LevelEditorModule.OnTabManagerChanged().AddLambda(RegisterTabSpawner);
	}
}

void SLivDebuggingTab::UnregisterNomadTabSpawner()
{
	if (FSlateApplication::IsInitialized() && FModuleManager::Get().IsModuleLoaded(LivDebuggingTab::LevelEditorModuleName))
	{
		FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(LivDebuggingTab::LevelEditorModuleName);
		TSharedPtr<FTabManager> LevelEditorTabManager;
		if (LevelEditorModule)
		{
			LevelEditorTabManager = LevelEditorModule->GetLevelEditorTabManager();
			LevelEditorModule->OnTabManagerChanged().Remove(LevelEditorTabManagerChangedHandle);
		}

		if (LevelEditorTabManager.IsValid())
		{
			LevelEditorTabManager->UnregisterTabSpawner(LivDebuggingTab::LivRenderTargetVisualiserApp);
		}
	}
}

SLivDebuggingTab::SLivDebuggingTab()
	: CaptureDebugView(nullptr)
{
}

SLivDebuggingTab::~SLivDebuggingTab()
{
	if (ILivModule::IsAvailable())
	{
		ILivModule::Get().OnLivCaptureActivated().Remove(LivCaptureActivatedHandle);
		ILivModule::Get().OnLivCaptureDeactivated().Remove(LivCaptureDeactivatedHandle);
	}
}

void SLivDebuggingTab::Construct(const FArguments& InArgs)
{
	LivCaptureActivatedHandle = ILivModule::Get().OnLivCaptureActivated().AddRaw(this, &SLivDebuggingTab::OnLivCaptureActivated);
	LivCaptureDeactivatedHandle = ILivModule::Get().OnLivCaptureDeactivated().AddRaw(this, &SLivDebuggingTab::OnLivCaptureDeactivated);
}

void SLivDebuggingTab::OnLivCaptureActivated()
{
	ULivLocalPlayerSubsystem* LocalPlayerSubsystem = GetLivLocalPlayerSubsystem();
	
	if(!LocalPlayerSubsystem)
	{
		return;
	}

	ULivWorldSubsystem* WorldSubsystem = LocalPlayerSubsystem->GetLivWorldSubsystem();

	if(!WorldSubsystem)
	{
		return;
	}

	const TWeakObjectPtr<ULivCaptureBase> CaptureComponent = WorldSubsystem->GetCaptureComponent();

	if(!CaptureComponent.IsValid())
	{
		return;
	}

	SAssignNew(CaptureDebugView, SLivCaptureDebugView)
		.CaptureComponent(CaptureComponent);

	ChildSlot
	[
		CaptureDebugView.ToSharedRef()
	];
}

void SLivDebuggingTab::OnLivCaptureDeactivated()
{
	ChildSlot.DetachWidget();
	CaptureDebugView = nullptr;
}

////////////////////////////////////////////////////////////////////////////

SLivCaptureDebugView::SLivCaptureDebugView()
	: CaptureComponent(nullptr)
{
}

SLivCaptureDebugView::~SLivCaptureDebugView()
{
}

void SLivCaptureDebugView::Construct(const FArguments& InArgs)
{
	CaptureComponent = InArgs._CaptureComponent;

	CaptureClasses.Reserve(4);
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (!It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated) && It->IsChildOf<ULivCaptureBase>())
		{
			CaptureClasses.Add(MakeShared<FString>(It->GetName()));
		}
	}

	ULivPluginSettings* LivPluginSettings = GetMutableDefault<ULivPluginSettings>();

	CaptureClass = LivPluginSettings && LivPluginSettings->CaptureMethod
		? LivPluginSettings->CaptureMethod->GetClass()->GetName()
		: ULivCaptureMeshClipPlaneNoPostProcess::StaticClass()->GetName();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor::Gray)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(NSLOCTEXT("LivRenderTargetVisualiser", "CaptureMethodClassLabel", "Capture Method: "))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&CaptureClasses)
						.InitiallySelectedItem(MakeShared<FString>(CaptureClass))
						.OnGenerateWidget(this, &SLivCaptureDebugView::GenerateWidgetForCaptureClassesCombo)
						.OnSelectionChanged(this, &SLivCaptureDebugView::CaptureClassChanged)
						.Content()
						[
							SNew(STextBlock).Text(this, &SLivCaptureDebugView::GetCaptureMethodName)
						]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(InputsContainer, SHorizontalBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(IntermediatesContainer, SHorizontalBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(OutputsContainer, SHorizontalBox)
		]
	];

	const auto GetSlotBasedOnMeta = [&](const FObjectProperty& ObjectProperty) -> TSharedPtr<SHorizontalBox>
	{
		const FString* MetaValue = ObjectProperty.FindMetaData(TEXT("LivStage"));

		if(MetaValue)
		{
			if(MetaValue->Equals(TEXT("Input")))
			{
				return InputsContainer;
			}
			if(MetaValue->Equals(TEXT("Output")))
			{
				return OutputsContainer;
			}
		}
		
		return IntermediatesContainer;	
	};

	if (CaptureComponent.IsValid())
	{
		const TSubclassOf<ULivCaptureBase> CaptureComponentClass(CaptureComponent->GetClass());
		
		const auto RenderTargetObjectProperties =
			LivEditorReflectionUtils::GetObjectPropertiesOfTypeInClass<ULivCaptureBase, UTextureRenderTarget2D>(CaptureComponentClass, EFieldIteratorFlags::ExcludeSuper);

		for(auto It = RenderTargetObjectProperties.CreateConstIterator(); It; ++It)
		{
			const FObjectProperty* ObjectProperty = *It;

			FVector4 DepthSwitch(0, 0, 0, 0);
			if(const FString* DepthMeta = ObjectProperty->FindMetaData("LivDepth"))
			{
				if(DepthMeta->Equals(TEXT("R"), ESearchCase::IgnoreCase))
				{
					DepthSwitch.X = 1.0f;
				}
				else if (DepthMeta->Equals(TEXT("A"), ESearchCase::IgnoreCase))
				{
					DepthSwitch.W = 1.0f;
				}
			}

			FVector4 MaskSwitch(0, 0, 0, 0);
			if (const FString* MaskMeta = ObjectProperty->FindMetaData("LivMask"))
			{
				if (MaskMeta->Equals(TEXT("A"), ESearchCase::IgnoreCase))
				{
					DepthSwitch.W = 1.0f;
				}
			}
			
			auto RenderTargetObject = LivEditorReflectionUtils::GetObjectPropertyValue<UTextureRenderTarget2D>(*ObjectProperty, CaptureComponent.Get());
			const FLivRenderTargetItemRef Item = MakeShared<FLivRenderTargetItem>(RenderTargetObject, DepthSwitch, DepthSwitch);
	
			GetSlotBasedOnMeta(*ObjectProperty)->AddSlot()
			[
				SNew(SScaleBox)
				[
					SNew(SLivCaptureDebugViewItem, Item)
				]
			];
		}
	}
}

FText SLivCaptureDebugView::GetCaptureMethodName() const
{
	if (CaptureComponent.IsValid())
	{
		const FString ClassName = CaptureComponent->GetClass()->GetName();
		return FText::FromString(ClassName);
	}
	return FText::GetEmpty();
}

TSharedRef<SWidget> SLivCaptureDebugView::GenerateWidgetForCaptureClassesCombo(const TSharedPtr<FString> ClassName) const
{
	return SNew(STextBlock)
		.Text(TAttribute<FText>(FText::FromString(*ClassName)));
}

void SLivCaptureDebugView::CaptureClassChanged(const TSharedPtr<FString> ClassName, ESelectInfo::Type SelectInfo)
{
	if (ClassName.IsValid())
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (!It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated) && It->IsChildOf<ULivCaptureBase>())
			{
				if (It->GetName().Equals(*ClassName))
				{
					ULivPluginSettings* PluginSettings = GetMutableDefault<ULivPluginSettings>();
					PluginSettings->CaptureMethod = *It;
					PluginSettings->UpdateDefaultConfigFile();

					if (ULivLocalPlayerSubsystem* LocalPlayerSubsystem = GetLivLocalPlayerSubsystem())
					{
						LocalPlayerSubsystem->ResetCapture();
					}
					
					CaptureClass = *ClassName;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////


SLivCaptureDebugViewItem::SLivCaptureDebugViewItem()
	: RenderTargetDisplayDynamicMaterialInstance(nullptr)
	, MaxDepth(100.0f)
{
}

SLivCaptureDebugViewItem::~SLivCaptureDebugViewItem()
{
}

void SLivCaptureDebugViewItem::Construct(const FArguments& InArgs, const FLivRenderTargetItemRef& InItem)
{
	Item = InItem;

	UMaterial* RenderTargetDisplayMaterial = LoadObject<UMaterial>(nullptr, TEXT("Material'/LIV/Editor/M_RenderTargetDisplay.M_RenderTargetDisplay'"));
	UObject* EditorWorldContext = GEditor->GetEditorWorldContext().World();
	RenderTargetDisplayDynamicMaterialInstance = UMaterialInstanceDynamic::Create(RenderTargetDisplayMaterial, EditorWorldContext);

	// don't need double wide image if depth is just in R or if doesn't have a mask in alpha channel
	const float BrushWidth = Item->DepthSwitch.X == 1.0f || Item->DepthSwitch.W != 1.0f ? 256 : 512;
	Brush = MakeShareable(new FSlateImageBrush(RenderTargetDisplayDynamicMaterialInstance, FVector2D(BrushWidth, 256)));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage( FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder") )
		.BorderBackgroundColor(FLinearColor(1,1,1,0.45f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4)
			[
				// Name
				SNew(STextBlock)
					.Text(this, &SLivCaptureDebugViewItem::GetRenderTargetName)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.OnClicked(this, &SLivCaptureDebugViewItem::OnCalculateMaxDepth)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Calculate Depth")))
						]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.MinValue(100.0f)
						.MaxValue(100000.0f)
						.Value(this, &SLivCaptureDebugViewItem::GetMaxDepth)
						.OnValueChanged(this, &SLivCaptureDebugViewItem::SetMaxDepth)
						.AllowSpin(true)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFill)
				[
					SNew(SButton)
					.OnClicked(this, &SLivCaptureDebugViewItem::OnClick)
					[
						SNew(SImage)
							.Image(this, &SLivCaptureDebugViewItem::GetBrush)
					]
				]
			]
		]
	];
}

void SLivCaptureDebugViewItem::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (RenderTargetDisplayDynamicMaterialInstance)
	{
		Collector.AddReferencedObject(RenderTargetDisplayDynamicMaterialInstance);
	}
}

FReply SLivCaptureDebugViewItem::OnCalculateMaxDepth()
{
	if (Item->RenderTarget.IsValid())
	{
		UTextureRenderTarget2D* RenderTarget = Item->RenderTarget.Get();

		FRenderTarget* SrcRenderTarget = RenderTarget->GameThread_GetRenderTargetResource();

		const FIntRect Rect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY);

		// Read the render target surface data back.	
		struct FReadSurfaceContext
		{
			FRenderTarget* SrcRenderTarget;
			TArray<FLinearColor>* OutData;
			FIntRect Rect;
			FReadSurfaceDataFlags Flags;
		};

		TArray<FLinearColor> ImageData;
		FReadSurfaceContext Context =
		{
			SrcRenderTarget,
			&ImageData,
			Rect,
			FReadSurfaceDataFlags(RCM_MinMax)
		};

		ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[Context](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				*Context.OutData,
				Context.Flags
			);
		});

		FlushRenderingCommands();

		const FFloat16* Start = reinterpret_cast<FFloat16*>(ImageData.GetData());
		const FFloat16* End = Start + (RenderTarget->SizeX * RenderTarget->SizeY);

		FFloat16 Max = 0;
		for (const FFloat16* It = Start; It < End; ++It)
		{
			if (*It > Max)
			{
				Max = *It;
			}
		}

		MaxDepth = static_cast<float>(Max);
	}

	return FReply::Handled();
}

bool SLivCaptureDebugViewItem::IsDepth() const
{
	if (Item->RenderTarget.IsValid())
	{
		return Item->RenderTarget->RenderTargetFormat == ETextureRenderTargetFormat::RTF_R16f;
	}
	return false;
}

TOptional<float> SLivCaptureDebugViewItem::GetMaxDepth() const
{
	return MaxDepth;
}

void SLivCaptureDebugViewItem::SetMaxDepth(float NewMaxDepth)
{
	MaxDepth = NewMaxDepth;
}

const FSlateBrush* SLivCaptureDebugViewItem::GetBrush() const
{
	if (Item->RenderTarget.IsValid())
	{
		RenderTargetDisplayDynamicMaterialInstance->SetTextureParameterValue("RenderTarget", Item->RenderTarget.Get());
		const FVector4& DepthSwitch = Item->DepthSwitch;
		RenderTargetDisplayDynamicMaterialInstance->SetVectorParameterValue("DepthSwitch", FLinearColor(DepthSwitch));
		const FVector4& MaskSwitch = Item->MaskSwitch;
		RenderTargetDisplayDynamicMaterialInstance->SetVectorParameterValue("MaskSwitch", FLinearColor(MaskSwitch));
		if(Item->DepthSwitch.SizeSquared() > 0)
		{
			RenderTargetDisplayDynamicMaterialInstance->SetScalarParameterValue("InverseMaxDepth", 1.0f / MaxDepth);
		}
	}
	return Brush.Get();
}

FText SLivCaptureDebugViewItem::GetRenderTargetName() const
{
	if (Item->RenderTarget.IsValid())
	{
		const FString Name = IsDepth() ? Item->RenderTarget->GetName() + FString(TEXT(" (Depth)")) : Item->RenderTarget->GetName();
		return FText::FromString(Name);
	}
	return FText::GetEmpty();
}

FReply SLivCaptureDebugViewItem::OnClick() const
{
	UTextureRenderTarget2D* RenderTarget = Item->RenderTarget.Get();
	if (RenderTarget)
	{
		GEditor->EditObject(RenderTarget);
	}

	return FReply::Handled();
}

////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE