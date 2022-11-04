// Copyright 2021 LIV Inc. - MIT License
#include "LivTests.h"

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include "LIV.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "LivConversions.h"

#include "EngineGlobals.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

static constexpr auto GAutomationFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLivMatrixConversion, "LIV.Math.Matrix Conversion", GAutomationFlags)

/**
 * Test conversion of Liv matrix to a UE4 matrix.
 */
bool FLivMatrixConversion::RunTest(const FString& Parameters)
{
	const LIV_Matrix4x4 LivMatrix
	{
		0.866f, 0.5f,	0.0f, 5.0f,
		-0.5f,	0.866f, 0.0f, 2.0f,
		0.0f,	0.0f,	1.0f, 10.0f,
		0.0f,	0.0f,	0.0f, 1.0f
	};

	FMatrix UnrealMatrix = FMatrix::Identity;

	// Row 0
	UnrealMatrix.M[0][0] = 1.0f; 
	UnrealMatrix.M[0][1] = 0.0f;
	UnrealMatrix.M[0][2] = 0.0f;
	UnrealMatrix.M[0][3] = 0.0f;

	// Row 1
	UnrealMatrix.M[1][0] = 0.0f;
	UnrealMatrix.M[1][1] = 0.866f;
	UnrealMatrix.M[1][2] = -0.5f;
	UnrealMatrix.M[1][3] = 0.0f;

	// Row 2
	UnrealMatrix.M[2][0] = 0.0f;
	UnrealMatrix.M[2][1] = 0.5f;
	UnrealMatrix.M[2][2] = 0.866f;
	UnrealMatrix.M[2][3] = 0.0f;

	// Row 3
	UnrealMatrix.M[3][0] = 1000.0f;
	UnrealMatrix.M[3][1] = 500.0f;
	UnrealMatrix.M[3][2] = 200.0f;
	UnrealMatrix.M[3][3] = 1.0f;

	// Convert from Liv space (Unity) to UE4
	const FMatrix ConvertedMatrix = Convert<LIV_Matrix4x4, FMatrix>(LivMatrix);

	// Test equality of converted matrix - element by element
	for (int32 Row{ 0 }; Row < 4; Row++)
	{
		for (int32 Col{ 0 }; Col < 4; Col++)
		{
			const FString What = FString::Printf(TEXT("[%d,%d]"), Row, Col);
			TestEqual<float>(What, ConvertedMatrix.M[Row][Col], UnrealMatrix.M[Row][Col]);
		}
	}

	// Convert from UE4 space back to Liv space (Unity)
	const LIV_Matrix4x4 ConvertedBackMatrix = Convert<FMatrix, LIV_Matrix4x4>(ConvertedMatrix);

	for(int32 Idx{0}; Idx < 16; ++Idx)
	{
		const FString What = FString::Printf(TEXT("[%d]"), Idx);
		TestEqual<float>(What, ConvertedBackMatrix.data[Idx], LivMatrix.data[Idx]);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLivLocationConversion, "LIV.Math.Location Conversion", GAutomationFlags)

/**
 * Test conversion between Liv space locations and UE4 space locations.
 */
bool FLivLocationConversion::RunTest(const FString& Parameters)
{
	
	const LIV_Vector3 LivLocation{ {3.0f, 2.0f, 9.0f }};
	const FVector UnrealLocation(900.0f, 300.0f, 200.0f);

	const FVector ConvertedLocation = ConvertPosition<LIV_Vector3, FVector>(LivLocation);

	TestEqual<float>(TEXT("Pos X"), ConvertedLocation.X, UnrealLocation.X);
	TestEqual<float>(TEXT("Pos Y"), ConvertedLocation.Y, UnrealLocation.Y);
	TestEqual<float>(TEXT("Pos Z"), ConvertedLocation.Z, UnrealLocation.Z);

	const LIV_Vector3 ConvertedBackLocation = ConvertPosition<FVector, LIV_Vector3>(ConvertedLocation);

	TestEqual<float>(TEXT("Pos X"), ConvertedBackLocation.x, LivLocation.x);
	TestEqual<float>(TEXT("Pos Y"), ConvertedBackLocation.y, LivLocation.y);
	TestEqual<float>(TEXT("Pos Z"), ConvertedBackLocation.z, LivLocation.z);
	
	
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLivQuaternionConversion, "LIV.Math.Quaternion Conversion", GAutomationFlags)

/**
 * Test conversion between UE4 and LIV quaternions.
 */
bool FLivQuaternionConversion::RunTest(const FString& Parameters)
{
	const LIV_Quaternion LivQuaternion {{ 0.394430071, -0.236777350, 0.105940290, 0.881554365 }};
	const FQuat UnrealQuaternion (0.105940290f, 0.394430071f, -0.236777350f, 0.881554365);

	const FQuat ConvertedQuaternion = Convert<LIV_Quaternion, FQuat>(LivQuaternion);

	TestEqual<float>(TEXT("Quat X"), ConvertedQuaternion.X, UnrealQuaternion.X);
	TestEqual<float>(TEXT("Quat Y"), ConvertedQuaternion.Y, UnrealQuaternion.Y);
	TestEqual<float>(TEXT("Quat Z"), ConvertedQuaternion.Z, UnrealQuaternion.Z);
	TestEqual<float>(TEXT("Quat W"), ConvertedQuaternion.W, UnrealQuaternion.W);

	const LIV_Quaternion ConvertedBackQuaternion = Convert<FQuat, LIV_Quaternion>(ConvertedQuaternion);

	TestEqual<float>(TEXT("Quat X"), ConvertedBackQuaternion.x, LivQuaternion.x);
	TestEqual<float>(TEXT("Quat Y"), ConvertedBackQuaternion.y, LivQuaternion.y);
	TestEqual<float>(TEXT("Quat Z"), ConvertedBackQuaternion.z, LivQuaternion.z);
	TestEqual<float>(TEXT("Quat W"), ConvertedBackQuaternion.w, LivQuaternion.w);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldOfViewConversion, "LIV.Math.FOV Conversion", GAutomationFlags)

/**
 * Test conversions between vertical field of view and horizontal field of view.
 */
bool FFieldOfViewConversion::RunTest(const FString& Parameters)
{
	// 16:9 aspect ratio
	const float Width = 16.0f;
	const float Height = 9.0f;
	const float AspectRatio = Width / Height;
	const float VerticalFOV = 59.0f;
	const float HorizontalFOV = 90.0f;

	//////////////////////////////////////////////////////////////////////////

	const float ActualHorizontalFOVFromRatio = FMath::RoundToFloat(ConvertVerticalFOVToHorizontalFOV(VerticalFOV, AspectRatio));
	TestEqual<float>(TEXT("Horizontal FOV from Aspect Ratio"), ActualHorizontalFOVFromRatio, HorizontalFOV);

	const float ActualHorizontalFOVFromWidthHeight = FMath::RoundToFloat(ConvertVerticalFOVToHorizontalFOV(VerticalFOV, Width, Height));
	TestEqual<float>(TEXT("Horizontal FOV from Width and Height"), ActualHorizontalFOVFromWidthHeight, HorizontalFOV);

	//////////////////////////////////////////////////////////////////////////

	const float ActualVerticalFOVFromRatio = FMath::RoundToFloat(ConvertHorizontalFOVToVerticalFOV(HorizontalFOV, AspectRatio));
	TestEqual<float>(TEXT("Vertical FOV from Aspect Ratio"), ActualVerticalFOVFromRatio, VerticalFOV);

	const float ActualVerticalFOVFromWidthHeight = FMath::RoundToFloat(ConvertHorizontalFOVToVerticalFOV(HorizontalFOV, Width, Height));
	TestEqual<float>(TEXT("Vertical FOV from Width and Height"), ActualVerticalFOVFromWidthHeight, VerticalFOV);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
#endif // PLATFORM_WINDOWS