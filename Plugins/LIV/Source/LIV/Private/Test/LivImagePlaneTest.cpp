// Copyright 2021 LIV Inc. - MIT License

#include "Test/LivImagePlaneTest.h"

#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"

ALivImagePlaneTest::ALivImagePlaneTest()
	: bDrawSphere(false)
	, Eye(ELivEye::Center)
{
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SetupAttachment(RootComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA
}


static void DeprojectScreenToWorld(
	const FVector2D& ScreenPos, 
	const FIntRect& ViewRect, 
	const FMatrix& InvViewProjMatrix,
	FVector& OutWorldNear,
	FVector& OutWorldFar)
{
	const int32 PixelX = FMath::TruncToInt(ScreenPos.X);
	const int32 PixelY = FMath::TruncToInt(ScreenPos.Y);

	// Get the eye position and direction of the mouse cursor in two stages (inverse transform projection, then inverse transform view).
	// This avoids the numerical instability that occurs when a view matrix with large translation is composed with a projection matrix

	// Get the pixel coordinates into 0..1 normalized coordinates within the constrained view rectangle
	const float NormalizedX = (PixelX - ViewRect.Min.X) / static_cast<float>(ViewRect.Width());
	const float NormalizedY = (PixelY - ViewRect.Min.Y) / static_cast<float>(ViewRect.Height());

	// Get the pixel coordinates into -1..1 projection space
	const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
	const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;

	// The start of the ray trace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
	// To get the direction of the ray trace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const FVector4 NearProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
	const FVector4 FarProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

	// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
	// by the projection matrix should use homogeneous coordinates (i.e. FPlane).
	const FVector4 HGNearViewSpace = InvViewProjMatrix.TransformFVector4(NearProjectionSpace);
	const FVector4 HGFarViewSpace = InvViewProjMatrix.TransformFVector4(FarProjectionSpace);
	FVector NearViewSpace(HGNearViewSpace.X, HGNearViewSpace.Y, HGNearViewSpace.Z);
	FVector FarViewSpace(HGFarViewSpace.X, HGFarViewSpace.Y, HGFarViewSpace.Z);
	// divide vectors by W to undo any projection and get the 3-space coordinate
	if (HGNearViewSpace.W != 0.0f)
	{
		NearViewSpace /= HGNearViewSpace.W;
	}
	if (HGFarViewSpace.W != 0.0f)
	{
		FarViewSpace /= HGFarViewSpace.W;
	}
	OutWorldNear = NearViewSpace;
	OutWorldFar = FarViewSpace;
}

static TArray<FVector> GetImagePlaneCornerWorldPositions(const FMatrix& InvViewProjMatrix)
{
	const TArray<FVector2D> ProjectionSpaceImagePlaneCorners
	{
		FVector2D(-1.0f, -1.0f),
		FVector2D(-1.0f, 1.0f),
		FVector2D(1.0f, 1.0f),
		FVector2D(1.0f, -1.0f),
	};

	TArray<FVector> Positions;
	Positions.Reserve(4);

	for(const auto& CornerPosition : ProjectionSpaceImagePlaneCorners)
	{
		// The start of the ray trace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
		// To get the direction of the ray trace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.5)
		const FVector4 NearProjectionSpace = FVector4(CornerPosition.X, CornerPosition.Y, 1.0f, 1.0f);

		// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
		// by the projection matrix should use homogeneous coordinates (i.e. FPlane).
		const FVector4 HGNearViewSpace = InvViewProjMatrix.TransformFVector4(NearProjectionSpace);

		FVector NearViewSpace(HGNearViewSpace.X, HGNearViewSpace.Y, HGNearViewSpace.Z);

		// divide vectors by W to undo any projection and get the 3-space coordinate
		if (HGNearViewSpace.W != 0.0f)
		{
			NearViewSpace /= HGNearViewSpace.W;
		}

		Positions.Add(NearViewSpace);
	}

	return Positions;
}

static bool RayIntersectsImagePlane(const FMatrix& InvViewProjMatrix)
{
	const auto Project = [&InvViewProjMatrix](const FVector2D& ScreenPosition)
	{
		const FVector4 ProjectionSpace = FVector4(ScreenPosition.X, ScreenPosition.Y, 1.0f, 1.0f);
		const FVector4 HGViewSpace = InvViewProjMatrix.TransformFVector4(ProjectionSpace);

		FVector ViewSpace(HGViewSpace.X, HGViewSpace.Y, HGViewSpace.Z);

		// divide vectors by W to undo any projection and get the 3-space coordinate
		if (HGViewSpace.W != 0.0f)
		{
			ViewSpace /= HGViewSpace.W;
		}

		return ViewSpace;
	};



	return false;
}

static bool SegmentTriangleIntersection(
	const FVector& StartPoint, 
	const FVector& EndPoint, 
	const FVector& A, 
	const FVector& B, 
	const FVector& C)
{
	FVector Edge1(B - A);
	Edge1.Normalize();
	FVector Edge2(C - A);
	Edge2.Normalize();
	FVector TriNormal = Edge2 ^ Edge1;
	TriNormal.Normalize();

	FVector IntersectPoint;

	const bool bCollide = FMath::SegmentPlaneIntersection(StartPoint, EndPoint, FPlane(A, TriNormal), IntersectPoint);
	if (!bCollide)
	{
		return false;
	}

	const FVector BaryCentric = FMath::ComputeBaryCentric2D(IntersectPoint, A, B, C);
	if (BaryCentric.X > 0.0f && BaryCentric.Y > 0.0f && BaryCentric.Z > 0.0f)
	{
		return true;
	}
	return false;
}

static bool SegmentRectangleIntersection(
	const FVector& StartPoint,
	const FVector& EndPoint,
	const FVector& A,
	const FVector& B,
	const FVector& C,
	const FVector& D)
{
	FVector Edge1(B - A);
	Edge1.Normalize();
	FVector Edge2(C - A);
	Edge2.Normalize();
	FVector TriNormal = Edge2 ^ Edge1;
	TriNormal.Normalize();

	FVector IntersectPoint;

	const bool bCollide = FMath::SegmentPlaneIntersection(StartPoint, EndPoint, FPlane(A, TriNormal), IntersectPoint);
	if (!bCollide)
	{
		return false;
	}

	const FVector BaryCentric = FMath::ComputeBaryCentric2D(IntersectPoint, A, B, C);
	if (BaryCentric.X > 0.0f && BaryCentric.Y > 0.0f && BaryCentric.Z > 0.0f)
	{
		return true;
	}
	return false;
}

void ALivImagePlaneTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	if(!LocalPlayer)
	{
		return;
	}

	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();

	if(!ViewportClient)
	{
		return;
	}

	auto TranslateEye = [](ELivEye LivEye)
	{
		switch (LivEye)
		{
		case ELivEye::Left: return eSSP_LEFT_EYE;
		case ELivEye::Right: return eSSP_RIGHT_EYE;
		default: return eSSP_FULL;
		}
	};


	const auto StereoPass = TranslateEye(Eye);

	FSceneViewProjectionData ProjectionData;
	const bool bValid = LocalPlayer->GetProjectionData(ViewportClient->Viewport, StereoPass, ProjectionData);

	if(!bValid)
	{
		return;
	}

	SetActorLocation(ProjectionData.ViewOrigin);

	if (bDrawSphere)
	{
		DrawDebugSphere(
			GetWorld(),
			ProjectionData.ViewOrigin,
			10.0f,
			16,
			FColor::Magenta.WithAlpha(2),
			false
		);
	}

	// see: UGameplayStatics::DeprojectScreenToWorld

	FMatrix const InvViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix().InverseFast();

	const TArray<FVector2D> ScreenPositionCorners
	{
		FVector2D(0.0f, 0.0f),
		FVector2D(0.0f, 1.0f),
		FVector2D(1.0f, 1.0f),
		FVector2D(1.0f, 0.0f)
	};

	FVector2D ViewportSize;
	ViewportClient->GetViewportSize(ViewportSize);

	for (const FVector2D&  ScreenPosition : ScreenPositionCorners)
	{
		FVector Near, Far;
		DeprojectScreenToWorld(ScreenPosition * ViewportSize, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, Near, Far);

		DrawDebugLine(
			GetWorld(),
			Near,
			Far,
			FColor(2, 2, 2, 2)
		);
	}

	const auto Corners = GetImagePlaneCornerWorldPositions(InvViewProjMatrix);


	for (int32 Idx{ 0 }; Idx < Corners.Num(); ++Idx)
	{
		const auto& CurrentCorner = Corners[Idx];
		const auto& NextCorner = Corners[(Idx + 1) % Corners.Num()];

		DrawDebugLine(
			GetWorld(),
			CurrentCorner,
			NextCorner,
			FColor::Black
		);
	}

	const bool bIntersectsImagePlane =
			SegmentTriangleIntersection(RayStart, RayEnd, Corners[0], Corners[1], Corners[3])
		||	SegmentTriangleIntersection(RayStart, RayEnd, Corners[1], Corners[3], Corners[2]);

	DrawDebugDirectionalArrow(
		GetWorld(),
		RayStart,
		RayEnd,
		2.0f,
		bIntersectsImagePlane ? FColor::Green : FColor::Red
	);


	///
	///

	//DrawDebugDirectionalArrow(
	//	GetWorld(),
	//	RayStart,
	//	RayEnd,
	//	2.0f,
	//	FColor::Yellow
	//);

	//const auto Corners = GetImagePlaneCornerWorldPositions(InvViewProjMatrix);

	//FVector Intersection, Normal;
	//FMath::SegmentIntersection2D(RayStart, RayEnd, Corners[0], Corners[1], Corners[2], Corners[3], Intersection);
}

