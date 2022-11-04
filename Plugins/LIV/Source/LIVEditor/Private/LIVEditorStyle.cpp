// Copyright 2021 LIV Inc. - MIT License
#include "LivEditorStyle.h"
#include "LivEditorModule.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FLivEditorStyle::StyleInstance = NULL;

void FLivEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLivEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLivEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LivEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D BackgroundSize(1920.0f, 1080.0f);

TSharedRef< FSlateStyleSet > FLivEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LIVEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LIV")->GetBaseDir() / TEXT("Resources"));

	Style->Set("LIVEditor.OpenPluginWindow", new IMAGE_BRUSH(TEXT("ButtonIcon_64x"), Icon40x40));
	Style->Set("LIVEditor.OpenWizard", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));

	Style->Set("LIVEditor.Icons.Small", new IMAGE_BRUSH(TEXT("ButtonIcon_16x"), Icon16x16));
	Style->Set("LIVEditor.Background", new IMAGE_BRUSH(TEXT("LIV_UI_Background"), BackgroundSize));
	Style->Set("LIVEditor.HeaderLogo", new IMAGE_BRUSH(TEXT("LIV"), FVector2D(36.0f, 20.0f)));
	Style->Set("LIVEditor.HeaderLogoLarge", new IMAGE_BRUSH(TEXT("LIV"), FVector2D(70.0f, 36.0f)));
	Style->Set("LIVEditor.LandingPage", new IMAGE_BRUSH(TEXT("LandingPage"), FVector2D(1920.0f, 1080.0f)));

	Style->Set("LIVEditor.DarkGroupBorder", new BOX_BRUSH("LivDarkGroupBorder", FMargin(4.0f / 16.0f)));

	{
		Style->Set("LIVEditor.Throbber.Chunk", new IMAGE_BRUSH("Throbber", FVector2D(16, 16)));
		Style->Set("LIVEditor.Throbber.CircleChunk", new IMAGE_BRUSH("Throbber", FVector2D(8, 8)));
	}

	Style->Set("LIVEditor.Check", new IMAGE_BRUSH("Check", FVector2D(48, 48)));
	Style->Set("LIVEditor.Cross", new IMAGE_BRUSH("Cross", FVector2D(48, 48)));

	Style->Set("LIVEditor.Discord", new IMAGE_BRUSH("Discord", FVector2D(237, 64)));

	
	const FTextBlockStyle WizardPageTitle = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("Wizard.PageTitle");

	Style->Set("LivEditor.Home.SubHeader", FTextBlockStyle(WizardPageTitle)
		.SetFontSize(16)
	);

	const FTextBlockStyle NormalText = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText");
	Style->Set("LIVEditor.Wizard.StatusText", FTextBlockStyle(NormalText).SetFontSize(11));

	constexpr FLinearColor LivGreen(0, 0.57758f, 0.0865f);
	Style->Set("LIVEditor.Colors.Green", LivGreen);

	const FButtonStyle FlatButton = FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Success");

	struct ButtonColor
	{
	public:
		FName Name;
		FLinearColor Normal;
		FLinearColor Hovered;
		FLinearColor Pressed;

		ButtonColor(const FName& InName, const FLinearColor& Color) : Name(InName)
		{
			Normal = Color * 0.8f;
			Normal.A = Color.A;
			Hovered = Color * 1.0f;
			Hovered.A = Color.A;
			Pressed = Color * 0.6f;
			Pressed.A = Color.A;
		}
	};

	TArray< ButtonColor > FlatButtons;
	//FlatButtons.Add(ButtonColor("LIVEditor.FlatButton.Primary", FLinearColor(0.02899, 0.19752, 0.48195)));
	FlatButtons.Add(ButtonColor("LIVEditor.FlatButton.Success", LivGreen));
	//FlatButtons.Add(ButtonColor("LIVEditor.FlatButton.Info", FLinearColor(0.10363, 0.53564, 0.7372)));
	//FlatButtons.Add(ButtonColor("LIVEditor.FlatButton.Warning", FLinearColor(0.87514, 0.42591, 0.07383)));
	//FlatButtons.Add(ButtonColor("LIVEditor.FlatButton.Danger", FLinearColor(0.70117, 0.08464, 0.07593)));

	const auto CopyAndOverrideTint = [](FSlateBrush Brush, const FLinearColor Tint)
	{
		Brush.TintColor = Tint;
		return Brush;
	};

	for (const ButtonColor& Entry : FlatButtons)
	{
		Style->Set(Entry.Name, FButtonStyle(FlatButton)
			.SetNormal(CopyAndOverrideTint(FlatButton.Normal, Entry.Normal))
			.SetHovered(CopyAndOverrideTint(FlatButton.Hovered, Entry.Hovered))
			.SetPressed(CopyAndOverrideTint(FlatButton.Pressed, Entry.Pressed))
		);
	}

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FLivEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLivEditorStyle::Get()
{
	return *StyleInstance;
}
