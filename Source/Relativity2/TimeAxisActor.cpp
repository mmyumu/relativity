#include "TimeAxisActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

ATimeAxisActor::ATimeAxisActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AxisLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AxisLine"));
	AxisLine->SetupAttachment(Root);
	AxisLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMeshFinder.Succeeded())
	{
		CubeMesh = CubeMeshFinder.Object;
		AxisLine->SetStaticMesh(CubeMesh);
	}

	AxisLine->SetRelativeLocation(FVector::ZeroVector);
	AxisLine->SetRelativeScale3D(FVector(
		AxisThickness / 100.0f,
		AxisThickness / 100.0f,
		AxisLength / 100.0f
	));
}

void ATimeAxisActor::BeginPlay()
{
	Super::BeginPlay();

	CreateTicks();
	UpdateAxis();
}

void ATimeAxisActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAxis();
}

void ATimeAxisActor::CreateTicks()
{
	if (!CubeMesh || VisibleTickCount <= 0)
	{
		return;
	}

	TickMarks.Empty();
	TickLabels.Empty();

	for (int32 i = 0; i < VisibleTickCount; ++i)
	{
		UStaticMeshComponent* TickMark = NewObject<UStaticMeshComponent>(
			this,
			*FString::Printf(TEXT("TickMark_%d"), i)
		);

		TickMark->RegisterComponent();
		TickMark->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		TickMark->SetStaticMesh(CubeMesh);
		TickMark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TickMark->SetRelativeScale3D(FVector(
			TickLength / 100.0f,
			TickThickness / 100.0f,
			TickThickness / 100.0f
		));

		TickMarks.Add(TickMark);

		UTextRenderComponent* Label = NewObject<UTextRenderComponent>(
			this,
			*FString::Printf(TEXT("TickLabel_%d"), i)
		);

		Label->RegisterComponent();
		Label->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		Label->SetWorldSize(LabelWorldSize);
		Label->SetHorizontalAlignment(EHTA_Left);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetTextRenderColor(FColor::White);
		Label->SetRelativeRotation(LabelRotation);

		TickLabels.Add(Label);
	}

	const float FollowZ = FollowActor ? FollowActor->GetActorLocation().Z : GetActorLocation().Z;
	const int32 CenterTickIndex = FMath::FloorToInt(FollowZ / TickSpacing);
	LastStartTickIndex = CenterTickIndex - (VisibleTickCount / 2);
	BufferStart = 0;

	for (int32 i = 0; i < VisibleTickCount; ++i)
	{
		PlaceTick(i, LastStartTickIndex + i);
	}
}

void ATimeAxisActor::PlaceTick(int32 SlotIndex, int32 TickValue)
{
	const bool bVisible = !bClampAtZero || TickValue >= 0;
	const FVector ActorLoc = GetActorLocation();
	const float WorldZ = TickValue * TickSpacing;

	UStaticMeshComponent* Mark = TickMarks.IsValidIndex(SlotIndex) ? TickMarks[SlotIndex] : nullptr;
	UTextRenderComponent* Label = TickLabels.IsValidIndex(SlotIndex) ? TickLabels[SlotIndex] : nullptr;

	if (Mark)
	{
		Mark->SetVisibility(bVisible);
		if (bVisible)
		{
			Mark->SetWorldLocation(FVector(ActorLoc.X - TickLength * 0.5f, ActorLoc.Y, WorldZ));
		}
	}

	if (Label)
	{
		Label->SetVisibility(bVisible);
		if (bVisible)
		{
			Label->SetWorldLocation(FVector(ActorLoc.X - LabelOffsetX, ActorLoc.Y, WorldZ));
			Label->SetText(FText::FromString(FString::Printf(TEXT("%d %s"), TickValue, *UnitLabel)));
			Label->SetWorldSize(LabelWorldSize);
			Label->SetRelativeRotation(LabelRotation);
		}
	}
}

void ATimeAxisActor::UpdateAxis()
{
	if (TickSpacing <= KINDA_SMALL_NUMBER || TickMarks.Num() == 0)
	{
		return;
	}

	const float FollowZ = FollowActor ? FollowActor->GetActorLocation().Z : GetActorLocation().Z;

	// Only the axis line follows the player — not the whole actor
	if (AxisLine)
	{
		const FVector ActorLoc = GetActorLocation();
		AxisLine->SetWorldLocation(FVector(ActorLoc.X, ActorLoc.Y, FollowZ));
	}

	const int32 CenterTickIndex = FMath::FloorToInt(FollowZ / TickSpacing);
	const int32 StartTickIndex = CenterTickIndex - (VisibleTickCount / 2);
	const int32 Delta = StartTickIndex - LastStartTickIndex;

	if (FMath::Abs(Delta) >= VisibleTickCount)
	{
		// Large jump: reinitialize all slots in circular order
		for (int32 i = 0; i < VisibleTickCount; ++i)
		{
			PlaceTick((BufferStart + i) % VisibleTickCount, StartTickIndex + i);
		}
	}
	else if (Delta > 0)
	{
		// Moved forward: recycle Delta bottom slots to the top
		for (int32 d = 0; d < Delta; ++d)
		{
			const int32 SlotIndex = (BufferStart + d) % VisibleTickCount;
			PlaceTick(SlotIndex, LastStartTickIndex + VisibleTickCount + d);
		}
		BufferStart = (BufferStart + Delta) % VisibleTickCount;
	}
	else if (Delta < 0)
	{
		// Moved backward: recycle -Delta top slots to the bottom
		for (int32 d = 0; d < -Delta; ++d)
		{
			const int32 SlotIndex = ((BufferStart - 1 - d) % VisibleTickCount + VisibleTickCount) % VisibleTickCount;
			PlaceTick(SlotIndex, LastStartTickIndex - 1 - d);
		}
		BufferStart = ((BufferStart + Delta) % VisibleTickCount + VisibleTickCount) % VisibleTickCount;
	}

	LastStartTickIndex = StartTickIndex;
}