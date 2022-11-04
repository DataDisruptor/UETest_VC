// Copyright 2021 LIV Inc. - MIT License
#pragma once

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include <dxgi.h>
#include "LIV_BridgeDatastruct.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "Kismet/KismetMathLibrary.h"
#include "PixelFormat.h"

/**
 * Copied from D3D11Viewport.h:45
 * (Not sure I want this plugin to list D3D11RHI as a module dependency).
 */
DXGI_FORMAT GetRenderTargetFormat(const EPixelFormat PixelFormat);

template<typename FromType, typename ToType>
inline ToType Convert(const FromType& From) {}

/**
 * Convert Unity/Liv space matrix to UE4 space matrix
 */
template<>
inline FMatrix Convert<LIV_Matrix4x4, FMatrix>(const LIV_Matrix4x4& LivMatrix)
{
	FMatrix M(EForceInit::ForceInitToZero);

	M.M[0][0] = LivMatrix.m22;
	M.M[0][1] = LivMatrix.m02;
	M.M[0][2] = LivMatrix.m12;
	M.M[0][3] = LivMatrix.m30;

	M.M[1][0] = LivMatrix.m20;
	M.M[1][1] = LivMatrix.m00;
	M.M[1][2] = LivMatrix.m10;
	M.M[1][3] = LivMatrix.m31;

	M.M[2][0] = LivMatrix.m21;
	M.M[2][1] = LivMatrix.m01;
	M.M[2][2] = LivMatrix.m11;
	M.M[2][3] = LivMatrix.m32;

	M.M[3][0] = LivMatrix.m23 * 100.0f;
	M.M[3][1] = LivMatrix.m03 * 100.0f;
	M.M[3][2] = LivMatrix.m13 * 100.0f;
	M.M[3][3] = LivMatrix.m33;

	return M;
}

/**
 * Convert UE4 space matrix to Unity/Liv space matrix
 */
template<>
inline LIV_Matrix4x4 Convert<FMatrix, LIV_Matrix4x4>(const FMatrix& Matrix)
{
	LIV_Matrix4x4 M;

	M.m22 = Matrix.M[0][0];
	M.m02 = Matrix.M[0][1];
	M.m12 = Matrix.M[0][2];
	M.m30 = Matrix.M[0][3];

	M.m20 = Matrix.M[1][0];
	M.m00 = Matrix.M[1][1];
	M.m10 = Matrix.M[1][2];
	M.m31 = Matrix.M[1][3];

	M.m21 = Matrix.M[2][0];
	M.m01 = Matrix.M[2][1];
	M.m11 = Matrix.M[2][2];
	M.m32 = Matrix.M[2][3];

	M.m23 = Matrix.M[3][0] * (1.0f / 100.0f);
	M.m03 = Matrix.M[3][1] * (1.0f / 100.0f);
	M.m13 = Matrix.M[3][2] * (1.0f / 100.0f);
	M.m33 = Matrix.M[3][3];

	return M;
}

/**
 * Convert Unity/Liv space quaternion to UE4 space quaternion
 */
template<>
inline FQuat Convert<LIV_Quaternion, FQuat>(const LIV_Quaternion& LivQuaternion)
{
	return FQuat(LivQuaternion.z, LivQuaternion.x, LivQuaternion.y, LivQuaternion.w);
}

/**
 * Convert UE4 space quaternion to LIV space quaternion
 */
template<>
inline LIV_Quaternion Convert<FQuat, LIV_Quaternion>(const FQuat& Quaternion)
{
	return LIV_Quaternion{ { Quaternion.Y, Quaternion.Z, Quaternion.X, Quaternion.W } };
}

template<typename FromType, typename ToType>
inline ToType ConvertPosition(const FromType& From) {}

/**
 * Convert Unity/Liv space position to UE4 space position
 * UE4				Unity
 * X		=		Z * 100
 * Y		=		X * 100
 * Z		=		Y * 100
 */
template<>
inline FVector ConvertPosition<LIV_Vector3, FVector>(const LIV_Vector3& LivVector)
{
	const float Scale = 100.0f;
	return FVector(LivVector.z * Scale, LivVector.x * Scale, LivVector.y * Scale);
}

/**
 * Convert UE4 space position to Unity/Liv space position
 * Unity			UE4
 * X		=		Y / 100
 * Y		=		Z / 100
 * Z		=		X / 100
 */
template<>
inline LIV_Vector3 ConvertPosition<FVector, LIV_Vector3>(const FVector& Vector)
{
	const float Scale = 1.0 / 100.0f;
	return LIV_Vector3{{Vector.Y * Scale, Vector.Z * Scale, Vector.X * Scale }};
}

static inline float AspectRatio(const float Width, const float Height)
{
	return Width / Height;
}

/**
 * Convert Vertical Field of View (FOV) to Horizontal FOV given an aspect ratio.
 */
static inline float ConvertVerticalFOVToHorizontalFOV(const float VerticalFOV, const float AspectRatio)
{
	return 2.0f * UKismetMathLibrary::DegAtan(AspectRatio * UKismetMathLibrary::DegTan(VerticalFOV * 0.5f));
}

/**
 * Convert Vertical Field of View (FOV) to Horizontal FOV given a width and height.
 */
static inline float ConvertVerticalFOVToHorizontalFOV(const float VerticalFOV, const float Width, const float Height)
{
	return ConvertVerticalFOVToHorizontalFOV(VerticalFOV, Width / Height);
}

/**
 * Convert Horizontal Field of View (FOV) to Vertical FOV given an aspect ratio.
 */
static inline float ConvertHorizontalFOVToVerticalFOV(const float HorizontalFOV, const float AspectRatio)
{
	return 2.0f * UKismetMathLibrary::DegAtan((1.0f / AspectRatio) * UKismetMathLibrary::DegTan(HorizontalFOV * 0.5f));
}

/**
 * Convert Horizontal Field of View (FOV) to Vertical FOV given a width and height.
 */
static inline float ConvertHorizontalFOVToVerticalFOV(const float HorizontalFOV, const float Width, const float Height)
{
	return 2.0f * UKismetMathLibrary::DegAtan((Height / Width) * UKismetMathLibrary::DegTan(HorizontalFOV * 0.5f));
}

static LIV_Matrix4x4 CreateLivProjectionMatrix(const FIntPoint Resolution, const float FOVAngle, const float Near, const float Far = 1000.0f)
{
	const float FOV = FOVAngle * PI / 360.0f;
	LIV_Matrix4x4 LivProjectionMatrix;
	LivProjectionMatrix.data[0] = (1.0f / FMath::Tan(FOV));
	LivProjectionMatrix.data[1] = 0.0f;
	LivProjectionMatrix.data[2] = 0.0f;
	LivProjectionMatrix.data[3] = 0.0f;
	LivProjectionMatrix.data[4] = 0.0f;
	LivProjectionMatrix.data[5] = (Resolution.X / static_cast<float>(Resolution.Y)) / FMath::Tan(FOV);
	LivProjectionMatrix.data[6] = 0.0f;
	LivProjectionMatrix.data[7] = 0.0f;
	LivProjectionMatrix.data[8] = 0.0f;
	LivProjectionMatrix.data[9] = 0.0f;
	LivProjectionMatrix.data[10] = (Far + Near) / (Near - Far);
	LivProjectionMatrix.data[11] = (2 * (Far + Near) / (Near - Far));
	LivProjectionMatrix.data[12] = 0.0f;
	LivProjectionMatrix.data[13] = 0.0f;
	LivProjectionMatrix.data[14] = -1.0f;
	LivProjectionMatrix.data[15] = 0.0f;
	return LivProjectionMatrix;
}

#endif