#include "LivProjectionMatrixTest.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LivConversions.h"

// Sets default values
ALivProjectionMatrixTest::ALivProjectionMatrixTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALivProjectionMatrixTest::BeginPlay()
{
	Super::BeginPlay();
	
}

void BuildProjMatrix(FIntPoint RenderTargetSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, float InNearClippingPlane, FMatrix& ProjectionMatrix)
{
	float const XAxisMultiplier = 1.0f;
	float const YAxisMultiplier = RenderTargetSize.X / static_cast<float>(RenderTargetSize.Y);

	static_assert(static_cast<bool>(ERHIZBuffer::IsInverted), "Is inverted.");

	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		const float OrthoWidth = InOrthoWidth / 2.0f;
		const float OrthoHeight = InOrthoWidth / 2.0f * XAxisMultiplier / YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		ProjectionMatrix = FReversedZOrthoMatrix(
			OrthoWidth,
			OrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		ProjectionMatrix = FReversedZPerspectiveMatrix(
			FOV,
			FOV,
			XAxisMultiplier,
			YAxisMultiplier,
			InNearClippingPlane,
			InNearClippingPlane
		);
	}
}

// Called every frame
void ALivProjectionMatrixTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if PLATFORM_WINDOWS

	if(!CaptureComponent)
	{
		if(SceneCaptureActor)
		{
			CaptureComponent = SceneCaptureActor->GetCaptureComponent2D();
		}
	}

	if (CaptureComponent && CaptureComponent->TextureTarget)
	{
		const float ClippingPlane = (CaptureComponent->bOverride_CustomNearClippingPlane) ? CaptureComponent->CustomNearClippingPlane : GNearClippingPlane;
		const float FOV = CaptureComponent->FOVAngle * static_cast<float>(PI) / 360.0f;

		const FIntPoint Resolution(CaptureComponent->TextureTarget->SizeX, CaptureComponent->TextureTarget->SizeY);
		BuildProjMatrix(Resolution,
			CaptureComponent->ProjectionType,
			FOV,
			CaptureComponent->OrthoWidth,
			ClippingPlane,
			ProjectionMatrix);

		ProjectionMatrixRow0 = FVector4(ProjectionMatrix.M[0][0], ProjectionMatrix.M[0][1], ProjectionMatrix.M[0][2], ProjectionMatrix.M[0][3]);
		ProjectionMatrixRow1 = FVector4(ProjectionMatrix.M[1][0], ProjectionMatrix.M[1][1], ProjectionMatrix.M[1][2], ProjectionMatrix.M[1][3]);
		ProjectionMatrixRow2 = FVector4(ProjectionMatrix.M[2][0], ProjectionMatrix.M[2][1], ProjectionMatrix.M[2][2], ProjectionMatrix.M[2][3]);
		ProjectionMatrixRow3 = FVector4(ProjectionMatrix.M[3][0], ProjectionMatrix.M[3][1], ProjectionMatrix.M[3][2], ProjectionMatrix.M[3][3]);

		const LIV_Matrix4x4 LivProjectionMatrix = Convert<FMatrix, LIV_Matrix4x4>(ProjectionMatrix);

		LivProjectionMatrixRow0 = FVector4(LivProjectionMatrix.data[0], LivProjectionMatrix.data[1], LivProjectionMatrix.data[2], LivProjectionMatrix.data[3]);
		LivProjectionMatrixRow1 = FVector4(LivProjectionMatrix.data[4], LivProjectionMatrix.data[5], LivProjectionMatrix.data[6], LivProjectionMatrix.data[7]);
		LivProjectionMatrixRow2 = FVector4(LivProjectionMatrix.data[8], LivProjectionMatrix.data[9], LivProjectionMatrix.data[10], LivProjectionMatrix.data[11]);
		LivProjectionMatrixRow3 = FVector4(LivProjectionMatrix.data[12], LivProjectionMatrix.data[13], LivProjectionMatrix.data[14], LivProjectionMatrix.data[15]);

		const float N = ClippingPlane * 100.0f;
		const float F = 1000.0f * 100.0f;
		ManualProjectionMatrixRow0 = FVector4((1.0f / FMath::Tan(FOV)), 0.0f, 0.0f, 0.0f);
		ManualProjectionMatrixRow1 = FVector4(0.0f, (Resolution.X / static_cast<float>(Resolution.Y)) / FMath::Tan(FOV), 0.0f, 0.0f);
		ManualProjectionMatrixRow2 = FVector4(0.0f, 0.0f, (F+N)/(N-F), (2*(F+N) / (N-F)));
		ManualProjectionMatrixRow3 = FVector4(0.0f, 0.0f, -1.0f, 0.0f);
	}
#endif
}

