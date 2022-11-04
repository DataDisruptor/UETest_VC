// Copyright 2021 LIV Inc. - MIT License
#include "LivShaders.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "RenderGraphBuilder.h"
#include "ShaderCompilerCore.h"

IMPLEMENT_SHADER_TYPE(, FLivSingleTextureVertexShader, TEXT("/Plugin/Liv/LivSingleTextureVS.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLivInvertAlphaPixelShader, TEXT("/Plugin/Liv/LivInvertAlphaPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivCopyPixelShader, TEXT("/Plugin/Liv/LivCopyPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivCombineAlphaShader, TEXT("/Plugin/Liv/LivCombineAlphaPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivForegroundSegmentationPixelShader, TEXT("/Plugin/Liv/LivForegroundSegmentationPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivForegroundSegmentationPostProcessedPixelShader, TEXT("/Plugin/Liv/LivForegroundSegmentationPostProcessedPS.usf"), TEXT("MainPS"), SF_Pixel)

// RDG

IMPLEMENT_SHADER_TYPE(, FLivRDGScreenPassVS, TEXT("/Plugin/Liv/LivRDGScreenPassVS.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLivRDGInvertAlphaPS, TEXT("/Plugin/Liv/LivRDGInvertAlphaPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGCombineAlphaPS, TEXT("/Plugin/Liv/LivRDGCombineAlphaPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGForegroundSegmentationAndCopyPS, TEXT("/Plugin/Liv/LivRDGForegroundSegmentationAndCopyPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGForegroundSegmentationPPAndCopyPS, TEXT("/Plugin/Liv/LivRDGForegroundSegmentationPPAndCopyPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGCopySceneColorDepthPS, TEXT("/Plugin/Liv/LivRDGCopySceneColorDepthPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGCopySceneColorAndDepthPS, TEXT("/Plugin/Liv/LivRDGCopySceneColorAndDepthPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGCopyDepthPS, TEXT("/Plugin/Liv/LivRDGCopyDepthPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGSegmentByDepthPS, TEXT("/Plugin/Liv/LivRDGSegmentByDepthPS.usf"), TEXT("MainPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLivRDGCopyFullSceneColorPS, TEXT("/Plugin/Liv/LivRDGCopyFullSceneColorPS.usf"), TEXT("MainPS"), SF_Pixel)

IMPLEMENT_SHADER_TYPE(, FLivApplyEyeAdaptationPS, TEXT("/Plugin/Liv/LivEyeAdaptation.usf"), TEXT("MainPS"), SF_Pixel)


//IMPLEMENT_SHADER_TYPE(, FLivRDGSegmentPS, TEXT("/Plugin/Liv/LivRDGSegmentPS.usf"), TEXT("MainPS"), SF_Pixel)

IMPLEMENT_TYPE_LAYOUT(FLivRDGSegmentPS);

#define IMPLEMENT_SEGMENT_SHADER(PostProcessing)\
	typedef TLivRDGSegmentPS<PostProcessing> FLivRDGSegmentPS_##PostProcessing;\
	IMPLEMENT_SHADER_TYPE4_WITH_TEMPLATE_PREFIX(template<>, LIVRENDERING_API, FLivRDGSegmentPS_##PostProcessing, SF_Pixel);

IMPLEMENT_SEGMENT_SHADER(false);
IMPLEMENT_SEGMENT_SHADER(true);


IMPLEMENT_MATERIAL_SHADER_TYPE(, FLivClipPlaneVS, TEXT("/Plugin/Liv/LivClipPlane.usf"), TEXT("VSMain"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLivClipPlanePS, TEXT("/Plugin/Liv/LivClipPlane.usf"), TEXT("PSMain"), SF_Pixel);

// CS

IMPLEMENT_TYPE_LAYOUT(FLivRDGCopy2DCS);


#define IMPLEMENT_COPY_RESOURCE_SHADER(ValueType)\
	typedef TLivRDGCopy2DCS<ECopyTextureValueType::ValueType, 4> FLivRDGCopy2DCS_##ValueType##4;\
	IMPLEMENT_SHADER_TYPE4_WITH_TEMPLATE_PREFIX(template<>, LIVRENDERING_API, FLivRDGCopy2DCS_##ValueType##4, SF_Compute);

IMPLEMENT_COPY_RESOURCE_SHADER(Float);
IMPLEMENT_COPY_RESOURCE_SHADER(Int32);
IMPLEMENT_COPY_RESOURCE_SHADER(Uint32);


//////////////////////////////////////////////////////////////////////////

void FLivInvertAlphaPixelShader::SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InInputTexture)
{
	SetTextureParameter(InRHICmdList, 
		InRHICmdList.GetBoundPixelShader(), 
		InputTexture, 
		InputTextureSampler, 
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
		InInputTexture->TextureRHI);
}

void FLivInvertAlphaPixelShader::SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture)
{
	SetTextureParameter(InRHICmdList, 
		InRHICmdList.GetBoundPixelShader(), 
		OutputTexture,
		OutputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), 
		InOutputTexture->TextureRHI);
}

void FLivInvertAlphaPixelShader::BindParams(const FShaderParameterMap& ParameterMap)
{
	InputTexture.Bind(ParameterMap, TEXT("InputTexture"));
	InputTextureSampler.Bind(ParameterMap, TEXT("InputTextureSampler"));

	OutputTexture.Bind(ParameterMap, TEXT("OutputTexture"));
	OutputTextureSampler.Bind(ParameterMap, TEXT("OutputTextureSampler"));
}

//////////////////////////////////////////////////////////////////////////


void FLivCopyPixelShader::SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InInputTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputTexture,
		InputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputTexture->TextureRHI);
}


void FLivCopyPixelShader::SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		OutputTexture,
		OutputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InOutputTexture->TextureRHI);
}

void FLivCopyPixelShader::BindParams(const FShaderParameterMap& ParameterMap)
{
	InputTexture.Bind(ParameterMap, TEXT("InputTexture"));
	InputTextureSampler.Bind(ParameterMap, TEXT("InputTextureSampler"));

	OutputTexture.Bind(ParameterMap, TEXT("OutputTexture"));
	OutputTextureSampler.Bind(ParameterMap, TEXT("OutputTextureSampler"));
}

//////////////////////////////////////////////////////////////////////////

void FLivCombineAlphaShader::SetInputParameters(FRHICommandList& InRHICmdList, FTextureResource* InColorTexture, FTextureResource* InAlphaTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputColorTexture,
		InputColorTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InColorTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputAlphaTexture,
		InputAlphaTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InAlphaTexture->TextureRHI);
}


void FLivCombineAlphaShader::SetOutputParameters(FRHICommandList& InRHICmdList, FTextureResource* InOutputTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		OutputTexture,
		OutputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InOutputTexture->TextureRHI);
}

void FLivCombineAlphaShader::BindParams(const FShaderParameterMap& ParameterMap)
{
	InputColorTexture.Bind(ParameterMap, TEXT("InputColorTexture"));
	InputColorTextureSampler.Bind(ParameterMap, TEXT("InputColorTextureSampler"));

	InputAlphaTexture.Bind(ParameterMap, TEXT("InputAlphaTexture"));
	InputAlphaTextureSampler.Bind(ParameterMap, TEXT("InputAlphaTextureSampler"));

	OutputTexture.Bind(ParameterMap, TEXT("OutputTexture"));
	OutputTextureSampler.Bind(ParameterMap, TEXT("OutputTextureSampler"));
}

//////////////////////////////////////////////////////////////////////////

void FLivForegroundSegmentationPixelShader::SetParameters(FRHICommandList& InRHICmdList, 
	FTextureResource* InInputBackgroundTexture, 
	FTextureResource* InInputForegroundTexture,
	FTextureResource* InOutputTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputBackgroundTexture,
		InputBackgroundTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputBackgroundTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputForegroundTexture,
		InputForegroundTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputForegroundTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		OutputTexture,
		OutputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InOutputTexture->TextureRHI);
}

void FLivForegroundSegmentationPixelShader::BindParams(const FShaderParameterMap& ParameterMap)
{
	InputBackgroundTexture.Bind(ParameterMap, TEXT("InputBackgroundTexture"));
	InputBackgroundTextureSampler.Bind(ParameterMap, TEXT("InputBackgroundTextureSampler"));

	InputForegroundTexture.Bind(ParameterMap, TEXT("InputForegroundTexture"));
	InputForegroundTextureSampler.Bind(ParameterMap, TEXT("InputForegroundTextureSampler"));

	OutputTexture.Bind(ParameterMap, TEXT("OutputTexture"));
	OutputTextureSampler.Bind(ParameterMap, TEXT("OutputTextureSampler"));
}

//////////////////////////////////////////////////////////////////////////

void FLivForegroundSegmentationPostProcessedPixelShader::SetParameters(FRHICommandList& InRHICmdList, 
	FTextureResource* InInputSceneColorTexture, 
	FTextureResource* InInputBackgroundDepthTexture, 
	FTextureResource* InInputForegroundDepthTexture, 
	FTextureResource* InOutputTexture)
{
	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputSceneColorTexture,
		InputSceneColorTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputSceneColorTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputBackgroundDepthTexture,
		InputBackgroundDepthTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputBackgroundDepthTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		InputForegroundDepthTexture,
		InputForegroundDepthTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InInputForegroundDepthTexture->TextureRHI);

	SetTextureParameter(InRHICmdList,
		InRHICmdList.GetBoundPixelShader(),
		OutputTexture,
		OutputTextureSampler,
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
		InOutputTexture->TextureRHI);
}

void FLivForegroundSegmentationPostProcessedPixelShader::BindParams(const FShaderParameterMap& ParameterMap)
{
	InputSceneColorTexture.Bind(ParameterMap, TEXT("InputSceneColorTexture"));
	InputSceneColorTextureSampler.Bind(ParameterMap, TEXT("InputSceneColorTextureSampler"));

	InputBackgroundDepthTexture.Bind(ParameterMap, TEXT("InputBackgroundDepthTexture"));
	InputBackgroundDepthTextureSampler.Bind(ParameterMap, TEXT("InputBackgroundDepthTextureSampler"));

	InputForegroundDepthTexture.Bind(ParameterMap, TEXT("InputForegroundDepthTexture"));
	InputForegroundDepthTextureSampler.Bind(ParameterMap, TEXT("InputForegroundDepthTextureSampler"));

	OutputTexture.Bind(ParameterMap, TEXT("OutputTexture"));
	OutputTextureSampler.Bind(ParameterMap, TEXT("OutputTextureSampler"));
}


//////////////////////////////////////////////////////////////////////////


namespace
{

	class FLivHistogramCS : public FGlobalShader
	{
	public:
		// Changing these numbers requires Histogram.usf to be recompiled.
		static constexpr uint32 ThreadGroupSizeX = 8;
		static constexpr uint32 ThreadGroupSizeY = 4;
		static constexpr uint32 LoopCountX = 8;
		static constexpr uint32 LoopCountY = 8;
		static constexpr uint32 HistogramSize = 64;

		// /4 as we store 4 buckets in one ARGB texel.
		static constexpr uint32 HistogramTexelCount = HistogramSize / 4;

		// The number of texels on each axis processed by a single thread group.
		static const FIntPoint TexelsPerThreadGroup;

		DECLARE_GLOBAL_SHADER(FLivHistogramCS);
		SHADER_USE_PARAMETER_STRUCT(FLivHistogramCS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
			SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Input)
			SHADER_PARAMETER_STRUCT(FLivEyeAdaptationParameters, EyeAdaptation)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
			SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, HistogramRWTexture)
			SHADER_PARAMETER(FIntPoint, ThreadGroupCount)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
			OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEX"), ThreadGroupSizeX);
			OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEY"), ThreadGroupSizeY);
			OutEnvironment.SetDefine(TEXT("LOOP_SIZEX"), LoopCountX);
			OutEnvironment.SetDefine(TEXT("LOOP_SIZEY"), LoopCountY);
			OutEnvironment.SetDefine(TEXT("HISTOGRAM_SIZE"), HistogramSize);
			OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
		}

		// One ThreadGroup processes LoopCountX*LoopCountY blocks of size ThreadGroupSizeX*ThreadGroupSizeY
		static FIntPoint GetThreadGroupCount(FIntPoint InputExtent)
		{
			return FIntPoint::DivideAndRoundUp(InputExtent, TexelsPerThreadGroup);
		}
	};

	const FIntPoint FLivHistogramCS::TexelsPerThreadGroup(ThreadGroupSizeX* LoopCountX, ThreadGroupSizeY* LoopCountY);

	IMPLEMENT_GLOBAL_SHADER(FLivHistogramCS, "/Engine/Private/PostProcessHistogram.usf", "MainCS", SF_Compute);

	class FLivHistogramReducePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLivHistogramReducePS);
		SHADER_USE_PARAMETER_STRUCT(FLivHistogramReducePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Input)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, EyeAdaptationTexture)
			SHADER_PARAMETER(uint32, LoopSize)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		// Uses full float4 to get best quality for smooth eye adaptation transitions.
		static constexpr EPixelFormat OutputFormat = PF_A32B32G32R32F;

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
			OutEnvironment.SetRenderTargetOutputFormat(0, OutputFormat);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(FLivHistogramReducePS, "/Engine/Private/PostProcessHistogramReduce.usf", "MainPS", SF_Pixel);

} //! namespace


FRDGTextureRef AddLivHistogramPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const FLivEyeAdaptationParameters& EyeAdaptationParameters,
	FScreenPassTexture SceneColor,
	FRDGTextureRef EyeAdaptationTexture)
{
	check(SceneColor.IsValid());
	check(EyeAdaptationTexture);

	const FIntPoint HistogramThreadGroupCount = FIntPoint::DivideAndRoundUp(SceneColor.ViewRect.Size(), FLivHistogramCS::TexelsPerThreadGroup);

	const uint32 HistogramThreadGroupCountTotal = HistogramThreadGroupCount.X * HistogramThreadGroupCount.Y;

	FRDGTextureRef HistogramTexture = nullptr;

	RDG_EVENT_SCOPE(GraphBuilder, "Histogram");

	// First pass outputs one flattened histogram per group.
	{
		const FIntPoint TextureExtent = FIntPoint(FLivHistogramCS::HistogramTexelCount, HistogramThreadGroupCountTotal);

		const FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
			TextureExtent,
			PF_FloatRGBA,
			FClearValueBinding::None,
			/*GFastVRamConfig.Histogram*/ TexCreate_FastVRAM | TexCreate_RenderTargetable | TexCreate_UAV | TexCreate_ShaderResource);

		// @NOTE/@TODO look into FastVRAM implications

		HistogramTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("Histogram"));

		FLivHistogramCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLivHistogramCS::FParameters>();
		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->Input = GetScreenPassTextureViewportParameters(FScreenPassTextureViewport(SceneColor));
		PassParameters->InputTexture = SceneColor.Texture;
		PassParameters->HistogramRWTexture = GraphBuilder.CreateUAV(HistogramTexture);
		PassParameters->ThreadGroupCount = HistogramThreadGroupCount;
		PassParameters->EyeAdaptation = EyeAdaptationParameters;

		TShaderMapRef<FLivHistogramCS> ComputeShader(View.ShaderMap);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("Liv Histogram %dx%d (CS)", SceneColor.ViewRect.Width(), SceneColor.ViewRect.Height()),
			ComputeShader,
			PassParameters,
			FIntVector(HistogramThreadGroupCount.X, HistogramThreadGroupCount.Y, 1));
	}

	FRDGTextureRef HistogramReduceTexture = nullptr;

	// Second pass further reduces the histogram to a single line. The second line contains the eye adaptation value (two line texture).
	{
		const FIntPoint TextureExtent = FIntPoint(FLivHistogramCS::HistogramTexelCount, 2);

		const FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create2D(
			TextureExtent,
			FLivHistogramReducePS::OutputFormat,
			FClearValueBinding::None,
			/*GFastVRamConfig.HistogramReduce*/ TexCreate_FastVRAM | TexCreate_RenderTargetable | TexCreate_ShaderResource);

		HistogramReduceTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("LivHistogramReduce"));

		const FScreenPassTextureViewport InputViewport(HistogramTexture);
		const FScreenPassTextureViewport OutputViewport(HistogramReduceTexture);

		FLivHistogramReducePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLivHistogramReducePS::FParameters>();
		PassParameters->Input = GetScreenPassTextureViewportParameters(InputViewport);
		PassParameters->InputTexture = HistogramTexture;
		PassParameters->InputSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->LoopSize = HistogramThreadGroupCountTotal;
		PassParameters->EyeAdaptationTexture = EyeAdaptationTexture;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(HistogramReduceTexture, ERenderTargetLoadAction::ENoAction);

		TShaderMapRef<FLivHistogramReducePS> PixelShader(View.ShaderMap);

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("LivHistogramReduce %dx%d (PS)", InputViewport.Extent.X, InputViewport.Extent.Y),
			View,
			OutputViewport,
			InputViewport,
			PixelShader,
			PassParameters);
	}

	return HistogramReduceTexture;
}


//////////////////////////



class FLivEyeAdaptationShader : public FGlobalShader
{
public:
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FLivEyeAdaptationParameters, EyeAdaptation)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistogramTexture)
	END_SHADER_PARAMETER_STRUCT()

	static constexpr EPixelFormat OutputFormat = PF_A32B32G32R32F;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters & Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters & Parameters, FShaderCompilerEnvironment & OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetRenderTargetOutputFormat(0, OutputFormat);
	}

	FLivEyeAdaptationShader() = default;
	FLivEyeAdaptationShader(const CompiledShaderInitializerType & Initializer)
		: FGlobalShader(Initializer)
	{}
};

class FLivEyeAdaptationPS : public FLivEyeAdaptationShader
{
	using Super = FLivEyeAdaptationShader;
public:
	DECLARE_GLOBAL_SHADER(FLivEyeAdaptationPS);
	SHADER_USE_PARAMETER_STRUCT(FLivEyeAdaptationPS, Super);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(Super::FParameters, Base)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_GLOBAL_SHADER(FLivEyeAdaptationPS, "/Engine/Private/PostProcessEyeAdaptation.usf", "EyeAdaptationPS", SF_Pixel);

class FLivEyeAdaptationCS : public FLivEyeAdaptationShader
{
	using Super = FLivEyeAdaptationShader;
public:
	DECLARE_GLOBAL_SHADER(FLivEyeAdaptationCS);
	SHADER_USE_PARAMETER_STRUCT(FLivEyeAdaptationCS, Super);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(Super::FParameters, Base)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, RWEyeAdaptationTexture)
	END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_GLOBAL_SHADER(FLivEyeAdaptationCS, "/Engine/Private/PostProcessEyeAdaptation.usf", "EyeAdaptationCS", SF_Compute);

float LivLuminanceMaxFromLensAttenuation()
{
	const bool bExtendedLuminanceRange = true; // IsExtendLuminanceRangeEnabled();

	//IConsoleVariable* CVarEyeAdaptationLensAttenuation = IConsoleManager::Get().FindConsoleVariable(TEXT("r.EyeAdaptation.LensAttenuation"));
	//float LensAttenuation = CVarEyeAdaptationLensAttenuation->GetValueOnRenderThread();
	float LensAttenuation = 0.78f;

	// 78 is defined in the ISO 12232:2006 standard.
	const float kISOSaturationSpeedConstant = 0.78f;

	const float LuminanceMax = kISOSaturationSpeedConstant / FMath::Max<float>(LensAttenuation, .01f);

	// if we do not have luminance range extended, the math is hardcoded to 1.0 scale.
	return bExtendedLuminanceRange ? LuminanceMax : 1.0f;
}

float LivGetAutoExposureCompensationFromSettings(const FViewInfo& View)
{
	const FPostProcessSettings& Settings = View.FinalPostProcessSettings;

	// This scales the average luminance AFTER it gets clamped, affecting the exposure value directly.
	float AutoExposureBias = Settings.AutoExposureBias;

	// AutoExposureBias need to minus 1 if it is used for mobile LDR, because we don't go through the postprocess eye adaptation pass. 
	if (IsMobilePlatform(View.GetShaderPlatform()) && !IsMobileHDR())
	{
		AutoExposureBias = AutoExposureBias - 1.0f;
	}

	return FMath::Pow(2.0f, AutoExposureBias);
}

FLivEyeAdaptationParameters GetLivEyeAdaptationParameters(const FViewInfo& View, ERHIFeatureLevel::Type MinFeatureLevel)
{
	// @TODO: implement
	const bool bExtendedLuminanceRange = true; // IsExtendLuminanceRangeEnabled();

	const FPostProcessSettings& Settings = View.FinalPostProcessSettings;

	// const FEngineShowFlags& EngineShowFlags = View.Family->EngineShowFlags;

	// @TODO: something else
	const EAutoExposureMethod AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;// GetAutoExposureMethod(View);

	const float LuminanceMax = bExtendedLuminanceRange ? LivLuminanceMaxFromLensAttenuation() : 1.0f;

	const float PercentToScale = 0.01f;

	const float ExposureHighPercent = FMath::Clamp(Settings.AutoExposureHighPercent, 1.0f, 99.0f) * PercentToScale;
	const float ExposureLowPercent = FMath::Min(FMath::Clamp(Settings.AutoExposureLowPercent, 1.0f, 99.0f) * PercentToScale, ExposureHighPercent);

	const float HistogramLogMax = bExtendedLuminanceRange ? EV100ToLog2(LuminanceMax, Settings.HistogramLogMax) : Settings.HistogramLogMax;
	const float HistogramLogMin = FMath::Min(bExtendedLuminanceRange ? EV100ToLog2(LuminanceMax, Settings.HistogramLogMin) : Settings.HistogramLogMin, HistogramLogMax - 1);

	// These clamp the average luminance computed from the scene color. We are going to calculate the white point first, and then
	// figure out the average grey point later. I.e. if the white point is 1.0, the middle grey point should be 0.18.
	float MinWhitePointLuminance = 1.0f;
	float MaxWhitePointLuminance = 1.0f;

	// Get the exposure compensation from the post process volume settings (everything except the curve)
	float ExposureCompensationSettings = LivGetAutoExposureCompensationFromSettings(View);

	// Get the exposure compensation from the curve
	// @TODO: re-implement?
	float ExposureCompensationCurve = 1.0f;// GetAutoExposureCompensationFromCurve(View);
	const float BlackHistogramBucketInfluence = 0.0f; // CVarEyeAdaptationBlackHistogramBucketInfluence.GetValueOnRenderThread();

	const float kMiddleGrey = 0.18f;

	// AEM_Histogram and AEM_Basic adjust their ExposureCompensation to middle grey (0.18). AEM_Manual ExposureCompensation is already calibrated to 1.0.
	const float GreyMult = (AutoExposureMethod == AEM_Manual) ? 1.0f : kMiddleGrey;

	const bool IsDebugViewMode = false; // IsAutoExposureDebugMode(View);

	if (IsDebugViewMode)
	{
		ExposureCompensationSettings = 1.0f;
		ExposureCompensationCurve = 1.0f;
	}
	// Fixed exposure override in effect.
	/*
	 * @NOTE: ignore for now
	else if (View.Family->ExposureSettings.bFixed)
	{
		ExposureCompensationSettings = 1.0f;
		ExposureCompensationCurve = 1.0f;

		// ignores bExtendedLuminanceRange
		MinWhitePointLuminance = MaxWhitePointLuminance = CalculateFixedAutoExposure(View);
	}
	*/
	/*
	 * @NOTE: EyeAdptation will be off, we're pretending it isn't
	else if (!EngineShowFlags.EyeAdaptation)
	{
		// if eye adaptation is off, then set everything to 1.0
		ExposureCompensationSettings = 1.0f;
		ExposureCompensationCurve = 1.0f;

		// GetAutoExposureMethod() should return Manual in this case.
		check(AutoExposureMethod == AEM_Manual);

		// just lock to 1.0, it's not possible to guess a reasonable value using the min and max.
		MinWhitePointLuminance = MaxWhitePointLuminance = 1.0;
	}
	*/
	// This should always be true now that it works on mobile
	else if (View.GetFeatureLevel() >= MinFeatureLevel)
	{
		if (AutoExposureMethod == EAutoExposureMethod::AEM_Manual)
		{
			// ignores bExtendedLuminanceRange
			// @NOTE: TODO if manual implemented
			// MinWhitePointLuminance = MaxWhitePointLuminance = CalculateManualAutoExposure(View, false);
		}
		else
		{
			if (bExtendedLuminanceRange)
			{
				MinWhitePointLuminance = EV100ToLuminance(LuminanceMax, Settings.AutoExposureMinBrightness);
				MaxWhitePointLuminance = EV100ToLuminance(LuminanceMax, Settings.AutoExposureMaxBrightness);
			}
			else
			{
				MinWhitePointLuminance = Settings.AutoExposureMinBrightness;
				MaxWhitePointLuminance = Settings.AutoExposureMaxBrightness;
			}
		}
	}

	MinWhitePointLuminance = FMath::Min(MinWhitePointLuminance, MaxWhitePointLuminance);

	const float HistogramLogDelta = HistogramLogMax - HistogramLogMin;
	const float HistogramScale = 1.0f / HistogramLogDelta;
	const float HistogramBias = -HistogramLogMin * HistogramScale;

	// If we are in histogram mode, then we want to set the minimum to the bottom end of the histogram. But if we are in basic mode,
	// we want to simply use a small epsilon to keep true black values from returning a NaN and/or a very low value. Also, basic
	// mode does the calculation in pre-exposure space, which is why we need to multiply by View.PreExposure.
	const float LuminanceMin = (AutoExposureMethod == AEM_Basic) ? 0.0001f : FMath::Exp2(HistogramLogMin);

	//AutoExposureMeterMask
	const FTextureRHIRef MeterMask = Settings.AutoExposureMeterMask ?
		Settings.AutoExposureMeterMask->Resource->TextureRHI :
		GWhiteTexture->TextureRHI;

	// The distance at which we switch from linear to exponential. I.e. at StartDistance=1.5, when linear is 1.5 f-stops away from hitting the 
	// target, we switch to exponential.
	// @TODO: re-implement (find CVar)
	const float StartDistance = 1.5f; // CVarEyeAdaptationExponentialTransitionDistance.GetValueOnRenderThread();
	const float StartTimeUp = StartDistance / FMath::Max(Settings.AutoExposureSpeedUp, 0.001f);
	const float StartTimeDown = StartDistance / FMath::Max(Settings.AutoExposureSpeedDown, 0.001f);

	// We want to ensure that at time=StartT, that the derivative of the exponential curve is the same as the derivative of the linear curve.
	// For the linear curve, the step will be AdaptationSpeed * FrameTime.
	// For the exponential curve, the step will be at t=StartT, M is slope modifier:
	//      slope(t) = M * (1.0f - exp2(-FrameTime * AdaptionSpeed)) * AdaptionSpeed * StartT
	//      AdaptionSpeed * FrameTime = M * (1.0f - exp2(-FrameTime * AdaptionSpeed)) * AdaptationSpeed * StartT
	//      M = FrameTime / (1.0f - exp2(-FrameTime * AdaptionSpeed)) * StartT
	//
	// To be technically correct, we should take the limit as FrameTime->0, but for simplicity we can make FrameTime a small number. So:
	const float kFrameTimeEps = 1.0f / 60.0f;
	const float ExponentialUpM = kFrameTimeEps / ((1.0f - exp2(-kFrameTimeEps * Settings.AutoExposureSpeedUp)) * StartTimeUp);
	const float ExponentialDownM = kFrameTimeEps / ((1.0f - exp2(-kFrameTimeEps * Settings.AutoExposureSpeedDown)) * StartTimeDown);

	// If the white point luminance is 1.0, then the middle grey luminance should be 0.18.
	const float MinAverageLuminance = MinWhitePointLuminance * kMiddleGrey;
	const float MaxAverageLuminance = MaxWhitePointLuminance * kMiddleGrey;

	const bool bValidRange = View.FinalPostProcessSettings.AutoExposureMinBrightness < View.FinalPostProcessSettings.AutoExposureMaxBrightness;

	// if it is a camera cut we force the exposure to go all the way to the target exposure without blending.
	// if it is manual mode, we also force the exposure to hit the target, which matters for HDR Visualization
	// if we don't have a valid range (AutoExposureMinBrightness == AutoExposureMaxBrightness) then force it like Manual as well.
	const float ForceTarget = (View.bCameraCut || AutoExposureMethod == EAutoExposureMethod::AEM_Manual || !bValidRange) ? 1.0f : 0.0f;

	FLivEyeAdaptationParameters Parameters;
	Parameters.ExposureLowPercent = ExposureLowPercent;
	Parameters.ExposureHighPercent = ExposureHighPercent;
	Parameters.MinAverageLuminance = MinAverageLuminance;
	Parameters.MaxAverageLuminance = MaxAverageLuminance;
	Parameters.ExposureCompensationSettings = ExposureCompensationSettings;
	Parameters.ExposureCompensationCurve = ExposureCompensationCurve;
	Parameters.DeltaWorldTime = View.Family->DeltaWorldTime;
	Parameters.ExposureSpeedUp = Settings.AutoExposureSpeedUp;
	Parameters.ExposureSpeedDown = Settings.AutoExposureSpeedDown;
	Parameters.HistogramScale = HistogramScale;
	Parameters.HistogramBias = HistogramBias;
	Parameters.LuminanceMin = LuminanceMin;
	Parameters.BlackHistogramBucketInfluence = BlackHistogramBucketInfluence; // no calibration constant because it is now baked into ExposureCompensation
	Parameters.GreyMult = GreyMult;
	Parameters.ExponentialDownM = ExponentialDownM;
	Parameters.ExponentialUpM = ExponentialUpM;
	Parameters.StartDistance = StartDistance;
	Parameters.LuminanceMax = LuminanceMax;
	Parameters.ForceTarget = ForceTarget;
	Parameters.VisualizeDebugType = 0;// CVarEyeAdaptationVisualizeDebugType.GetValueOnRenderThread();
	Parameters.MeterMaskTexture = MeterMask;
	Parameters.MeterMaskSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	return Parameters;
}


void AddLivHistogramEyeAdaptationPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& View,
	const FLivEyeAdaptationParameters& EyeAdaptationParameters,
	FRDGTextureRef HistogramTexture,
	FRDGTextureRef OutputTexture)
{
	// @NOTE: should be fine to just pass in the eye adaptation texture that we create in parent 
	// View.SwapEyeAdaptationTextures(GraphBuilder);
	// FRDGTextureRef OutputTexture = GraphBuilder.RegisterExternalTexture(View.GetEyeAdaptationTexture(GraphBuilder.RHICmdList), ERenderTargetTexture::Targetable, ERDGTextureFlags::MultiFrame);

	FLivEyeAdaptationShader::FParameters PassBaseParameters;
	PassBaseParameters.EyeAdaptation = GetLivEyeAdaptationParameters(View, ERHIFeatureLevel::SM5);
	PassBaseParameters.HistogramTexture = HistogramTexture;

	if (View.bUseComputePasses)
	{
		FLivEyeAdaptationCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLivEyeAdaptationCS::FParameters>();
		PassParameters->Base = PassBaseParameters;
		PassParameters->RWEyeAdaptationTexture = GraphBuilder.CreateUAV(OutputTexture);

		TShaderMapRef<FLivEyeAdaptationCS> ComputeShader(View.ShaderMap);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("LivHistogramEyeAdaptation (CS)"),
			ComputeShader,
			PassParameters,
			FIntVector(1, 1, 1));
	}
	else
	{
		FLivEyeAdaptationPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLivEyeAdaptationPS::FParameters>();
		PassParameters->Base = PassBaseParameters;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ENoAction);

		TShaderMapRef<FLivEyeAdaptationPS> PixelShader(View.ShaderMap);

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("LivHistogramEyeAdaptation (PS)"),
			View,
			FScreenPassTextureViewport(OutputTexture),
			FScreenPassTextureViewport(HistogramTexture),
			PixelShader,
			PassParameters);
	}

	//return OutputTexture;
}
