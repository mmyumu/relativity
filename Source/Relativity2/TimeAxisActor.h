#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeAxisActor.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class RELATIVITY2_API ATimeAxisActor : public AActor
{
	GENERATED_BODY()

public:
	ATimeAxisActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* AxisLine;

	UPROPERTY(EditAnywhere, Category = "Axis")
	AActor* FollowActor;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float TickSpacing = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	int32 VisibleTickCount = 31;

	UPROPERTY(EditAnywhere, Category = "Axis")
	FString UnitLabel = TEXT("ans");

	UPROPERTY(EditAnywhere, Category = "Axis")
	float AxisLength = 6000.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float AxisThickness = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float TickLength = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float TickThickness = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float LabelOffsetX = 110.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	float LabelWorldSize = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Axis")
	FRotator LabelRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Axis")
	bool bClampAtZero = true;

private:
	UPROPERTY()
	UStaticMesh* CubeMesh;

	UPROPERTY()
	TArray<UStaticMeshComponent*> TickMarks;

	UPROPERTY()
	TArray<UTextRenderComponent*> TickLabels;

	int32 LastStartTickIndex = 0;
	int32 BufferStart = 0;

	void CreateTicks();
	void PlaceTick(int32 SlotIndex, int32 TickValue);
	void UpdateAxis();
};