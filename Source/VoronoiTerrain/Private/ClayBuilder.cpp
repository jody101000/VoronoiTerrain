// Fill out your copyright notice in the Description page of Project Settings.


#include "ClayBuilder.h"
#include "ProceduralMeshComponent.h"

// Sets default values
UClayBuilder::UClayBuilder()
{
	PrimaryComponentTick.bCanEverTick = true;
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
}

void UClayBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
	    ProceduralMeshComponent->AttachToComponent(Owner->GetRootComponent(), 
	        FAttachmentTransformRules::KeepWorldTransform);
	}
	
}

void UClayBuilder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UClayBuilder::StartBuildClay()
{
	FVector MousePosition;
	if (GetMouseWorldPosition(MousePosition))
	{
		// Build mesh at MousePositions
		
		UE_LOG(LogTemp, Warning, TEXT("Clay forming at (%.03f, %.03f, %.03f)"), MousePosition.X, MousePosition.Y, MousePosition.Z );
	}
}

bool UClayBuilder::GetMouseWorldPosition(FVector& MouseWorldPosition) const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);
    
	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
	
	FHitResult HitResult;
	FVector TraceEnd = WorldLocation + (WorldDirection * 10000.0f);
    
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		MouseWorldPosition = HitResult.Location;
	}
	else // Not hit any object, return a position with max distance
	{
		MouseWorldPosition = WorldLocation + (WorldDirection * 1000.0f);
	}
    
	return true;
}

void UClayBuilder::CreateBasicCubeAtPosition(const FVector& Position)
{
	// Append Vertices, triangles, ... 
}


//
// **TerrainBuilder.h**
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "Components/ActorComponent.h"
// #include "Components/InstancedStaticMeshComponent.h"
// #include "Engine/World.h"
// #include "TerrainBuilder.generated.h"
//
// UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
// class YOURGAME_API UTerrainBuilder : public UActorComponent
// {
//     GENERATED_BODY()
//
// public:
//     UTerrainBuilder();
//
// protected:
//     virtual void BeginPlay() override;
//     virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//     // Terrain block mesh
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
//     UStaticMesh* TerrainBlockMesh;
//
//     // Size of each terrain block
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
//     float BlockSize = 100.0f;
//
//     // Building speed when holding mouse
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
//     float BuildSpeed = 5.0f;
//
//     // Maximum build distance
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
//     float MaxBuildDistance = 2000.0f;
//
// private:
//     // Instanced mesh component for terrain blocks
//     UPROPERTY()
//     UInstancedStaticMeshComponent* InstancedMeshComponent;
//
//     // Track built positions to avoid duplicates
//     TSet<FIntVector> BuiltPositions;
//
//     // Mouse input tracking
//     bool bIsBuilding = false;
//     FVector LastBuildPosition;
//     FVector PlayerPosition;
//     float BuildTimer = 0.0f;
//
//     // Input functions
//     void HandleMouseClick();
//     void HandleMouseHold();
//     void HandleMouseRelease();
//
//     // Building functions
//     bool BuildTerrainBlock(const FVector& WorldPosition);
//     FVector SnapToGrid(const FVector& Position);
//     FIntVector WorldToGridCoords(const FVector& WorldPosition);
//     bool GetMouseWorldPosition(FVector& OutWorldPosition);
//     void BuildTowardsPlayer();
//
// public:
//     // Called by player controller
//     void StartBuilding();
//     void StopBuilding();
//     void TickBuilding();
// };
//
// **TerrainBuilder.cpp**
//
// #include "TerrainBuilder.h"
// #include "Engine/Engine.h"
// #include "Engine/World.h"
// #include "GameFramework/PlayerController.h"
// #include "GameFramework/Pawn.h"
// #include "Camera/CameraComponent.h"
// #include "DrawDebugHelpers.h"
//
// UTerrainBuilder::UTerrainBuilder()
// {
//     PrimaryComponentTick.bCanEverTick = true;
//     
//     // Create instanced mesh component
//     InstancedMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMeshComponent"));
// }
//
// void UTerrainBuilder::BeginPlay()
// {
//     Super::BeginPlay();
//     
//     // Attach instanced mesh to owner's root component
//     if (AActor* Owner = GetOwner())
//     {
//         InstancedMeshComponent->AttachToComponent(Owner->GetRootComponent(), 
//             FAttachmentTransformRules::KeepWorldTransform);
//             
//         // Set the terrain block mesh
//         if (TerrainBlockMesh)
//         {
//             InstancedMeshComponent->SetStaticMesh(TerrainBlockMesh);
//         }
//     }
// }
//
// void UTerrainBuilder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
// {
//     Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//     
//     if (bIsBuilding)
//     {
//         BuildTimer += DeltaTime;
//         if (BuildTimer >= 1.0f / BuildSpeed)
//         {
//             BuildTowardsPlayer();
//             BuildTimer = 0.0f;
//         }
//     }
// }
//
// void UTerrainBuilder::StartBuilding()
// {
//     FVector MouseWorldPos;
//     if (GetMouseWorldPosition(MouseWorldPos))
//     {
//         // Build initial block
//         if (BuildTerrainBlock(MouseWorldPos))
//         {
//             LastBuildPosition = SnapToGrid(MouseWorldPos);
//             bIsBuilding = true;
//             BuildTimer = 0.0f;
//             
//             // Store player position
//             if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
//             {
//                 if (APawn* PlayerPawn = PC->GetPawn())
//                 {
//                     PlayerPosition = PlayerPawn->GetActorLocation();
//                 }
//             }
//         }
//     }
// }
//
// void UTerrainBuilder::StopBuilding()
// {
//     bIsBuilding = false;
//     BuildTimer = 0.0f;
// }
//
// void UTerrainBuilder::BuildTowardsPlayer()
// {
//     // Calculate direction from last build position to player
//     FVector Direction = (PlayerPosition - LastBuildPosition).GetSafeNormal();
//     FVector NextPosition = LastBuildPosition + (Direction * BlockSize);
//     
//     // Check if we're within build distance
//     float DistanceToPlayer = FVector::Dist(NextPosition, PlayerPosition);
//     if (DistanceToPlayer > MaxBuildDistance)
//     {
//         return;
//     }
//     
//     // Build the next block
//     if (BuildTerrainBlock(NextPosition))
//     {
//         LastBuildPosition = SnapToGrid(NextPosition);
//     }
// }
//
// bool UTerrainBuilder::BuildTerrainBlock(const FVector& WorldPosition)
// {
//     FVector SnappedPosition = SnapToGrid(WorldPosition);
//     FIntVector GridCoords = WorldToGridCoords(SnappedPosition);
//     
//     // Check if position is already built
//     if (BuiltPositions.Contains(GridCoords))
//     {
//         return false;
//     }
//     
//     // Add to built positions
//     BuiltPositions.Add(GridCoords);
//     
//     // Create transform for the new instance
//     FTransform InstanceTransform;
//     InstanceTransform.SetLocation(SnappedPosition);
//     InstanceTransform.SetRotation(FQuat::Identity);
//     InstanceTransform.SetScale3D(FVector::OneVector);
//     
//     // Add instance to the mesh component
//     InstancedMeshComponent->AddInstance(InstanceTransform);
//     
//     return true;
// }
//
// FVector UTerrainBuilder::SnapToGrid(const FVector& Position)
// {
//     FVector SnappedPos;
//     SnappedPos.X = FMath::RoundToFloat(Position.X / BlockSize) * BlockSize;
//     SnappedPos.Y = FMath::RoundToFloat(Position.Y / BlockSize) * BlockSize;
//     SnappedPos.Z = FMath::RoundToFloat(Position.Z / BlockSize) * BlockSize;
//     return SnappedPos;
// }
//
// FIntVector UTerrainBuilder::WorldToGridCoords(const FVector& WorldPosition)
// {
//     return FIntVector(
//         FMath::RoundToInt(WorldPosition.X / BlockSize),
//         FMath::RoundToInt(WorldPosition.Y / BlockSize),
//         FMath::RoundToInt(WorldPosition.Z / BlockSize)
//     );
// }
//
// bool UTerrainBuilder::GetMouseWorldPosition(FVector& OutWorldPosition)
// {
//     APlayerController* PC = GetWorld()->GetFirstPlayerController();
//     if (!PC)
//     {
//         return false;
//     }
//     
//     // Get mouse position
//     float MouseX, MouseY;
//     PC->GetMousePosition(MouseX, MouseY);
//     
//     // Convert to world space
//     FVector WorldLocation, WorldDirection;
//     PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
//     
//     // Perform line trace
//     FHitResult HitResult;
//     FVector TraceEnd = WorldLocation + (WorldDirection * 10000.0f);
//     
//     FCollisionQueryParams QueryParams;
//     QueryParams.AddIgnoredActor(GetOwner());
//     
//     // Trace against world static objects, or if no hit, use a point in space
//     if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_WorldStatic, QueryParams))
//     {
//         OutWorldPosition = HitResult.Location;
//     }
//     else
//     {
//         // If no hit, place at a fixed distance from camera
//         OutWorldPosition = WorldLocation + (WorldDirection * 1000.0f);
//     }
//     
//     return true;
// }
//
// **BuilderPlayerController.h**
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "GameFramework/PlayerController.h"
// #include "TerrainBuilder.h"
// #include "BuilderPlayerController.generated.h"
//
// UCLASS()
// class YOURGAME_API ABuilderPlayerController : public APlayerController
// {
//     GENERATED_BODY()
//
// public:
//     ABuilderPlayerController();
//
// protected:
//     virtual void BeginPlay() override;
//     virtual void SetupInputComponent() override;
//
//     // Input functions
//     void OnLeftMousePressed();
//     void OnLeftMouseReleased();
//
// private:
//     // Reference to terrain builder component
//     UPROPERTY()
//     UTerrainBuilder* TerrainBuilder;
// };
//
// **BuilderPlayerController.cpp**
//
// #include "BuilderPlayerController.h"
// #include "Engine/World.h"
// #include "GameFramework/Pawn.h"
//
// ABuilderPlayerController::ABuilderPlayerController()
// {
//     bShowMouseCursor = true;
//     bEnableClickEvents = true;
//     bEnableMouseOverEvents = true;
// }
//
// void ABuilderPlayerController::BeginPlay()
// {
//     Super::BeginPlay();
//     
//     // Find terrain builder component on possessed pawn
//     if (APawn* ControlledPawn = GetPawn())
//     {
//         TerrainBuilder = ControlledPawn->FindComponentByClass<UTerrainBuilder>();
//     }
// }
//
// void ABuilderPlayerController::SetupInputComponent()
// {
//     Super::SetupInputComponent();
//     
//     // Bind mouse input
//     InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &ABuilderPlayerController::OnLeftMousePressed);
//     InputComponent->BindAction("LeftMouseButton", IE_Released, this, &ABuilderPlayerController::OnLeftMouseReleased);
// }
//
// void ABuilderPlayerController::OnLeftMousePressed()
// {
//     if (TerrainBuilder)
//     {
//         TerrainBuilder->StartBuilding();
//     }
// }
//
// void ABuilderPlayerController::OnLeftMouseReleased()
// {
//     if (TerrainBuilder)
//     {
//         TerrainBuilder->StopBuilding();
//     }
// }
