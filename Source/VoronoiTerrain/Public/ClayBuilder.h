// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClayBuilder.generated.h"

class UDynamicMeshComponent;
class UProceduralMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class VORONOITERRAIN_API UClayBuilder : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UClayBuilder();
	
	void StartBuildClay();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 100.0f, ClampMax = 2000.0f))
	float MaxBuildDistance = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* Material;
	
	bool GetMouseWorldPosition(FVector& MouseWorldPosition) const;

	UPROPERTY()
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UDynamicMeshComponent* DynamicMeshComponent;
	
private:
	void CreateBasicCubeAtPosition(const FVector& Position);
	// void AppendBasicCubeAtPosition();

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> Colors;

};
