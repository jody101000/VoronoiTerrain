// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractivePlatform.h"
#include "../VoronoiTerrainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

AInteractivePlatform::AInteractivePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Mesh"));

	PlatformTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Platform Trigger Volumne"));
	PlatformTriggerVolume->SetupAttachment(PlatformComponent);
}


void AInteractivePlatform::OnConstruction(const FTransform& Transform)
{
	FlushPersistentDebugLines(GetWorld());
	if (PlatformComponent && PlatformComponent->GetStaticMesh())
	{
		PlatformAABB = PlatformComponent->GetStaticMesh()->GetBoundingBox();
		// PlatformAABB = GetComponentsBoundingBox();
		// PlatformAABB = CalculateComponentsBoundingBoxInLocalSpace();
		PlatformOBB = GetPlatformOBB();
		
		FTransform BoxTransform;
		BoxTransform.SetLocation(PlatformOBB.Center);
		FMatrix RotationMatrix(PlatformOBB.Forward, PlatformOBB.Right, PlatformOBB.Up, FVector::ZeroVector);
		FRotator MyRotator = RotationMatrix.Rotator();
		BoxTransform.SetRotation(FQuat::MakeFromRotator(MyRotator));
    
		// Set scale based on OBB extents (multiply by 2 for full extent)
		BoxTransform.SetScale3D(FVector(PlatformOBB.Forward.Length() * 2, PlatformOBB.Right.Length() * 2, PlatformOBB.Up.Length() * 2));

		PlatformTriggerVolume->SetRelativeTransform(BoxTransform);

		FVector p1 = PlatformOBB.Center + PlatformOBB.Up + PlatformOBB.Right + PlatformOBB.Forward;
		FVector p2 = PlatformOBB.Center + PlatformOBB.Up + PlatformOBB.Right - PlatformOBB.Forward;
		DrawDebugLine(GetWorld(), p1 + GetActorLocation(), p2 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);

		FVector p3 = PlatformOBB.Center + PlatformOBB.Up - PlatformOBB.Right + PlatformOBB.Forward;
		FVector p4 = PlatformOBB.Center + PlatformOBB.Up - PlatformOBB.Right - PlatformOBB.Forward;
		DrawDebugLine(GetWorld(), p3 + GetActorLocation(), p4 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p1 + GetActorLocation(), p3 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p2 + GetActorLocation(), p4 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		
		FVector p5 = PlatformOBB.Center - PlatformOBB.Up + PlatformOBB.Right + PlatformOBB.Forward;
		FVector p6 = PlatformOBB.Center - PlatformOBB.Up + PlatformOBB.Right - PlatformOBB.Forward;
		DrawDebugLine(GetWorld(), p5 + GetActorLocation(), p6 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);

		FVector p7 = PlatformOBB.Center - PlatformOBB.Up - PlatformOBB.Right + PlatformOBB.Forward;
		FVector p8 = PlatformOBB.Center - PlatformOBB.Up - PlatformOBB.Right - PlatformOBB.Forward;
		DrawDebugLine(GetWorld(), p7 + GetActorLocation(), p8 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p5 + GetActorLocation(), p7 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p6 + GetActorLocation(), p8 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);

		DrawDebugLine(GetWorld(), p3 + GetActorLocation(), p7 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p4 + GetActorLocation(), p8 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p1 + GetActorLocation(), p5 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);
		DrawDebugLine(GetWorld(), p2 + GetActorLocation(), p6 + GetActorLocation(), FColor::Blue, true, -1, 0, 2);

		DrawDebugBox(GetWorld(), PlatformAABB.GetCenter(), PlatformAABB.GetExtent(), FColor::Orange, true, -1, 0, 2);
	}
}


void AInteractivePlatform::BeginPlay()
{
	Super::BeginPlay();

	PlatformComponent->SetStaticMesh(ActiveMesh);

	PlatformTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractivePlatform::OnComponentBeginOverlap);
	PlatformTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractivePlatform::OnComponentEndOverlap);	
}

void AInteractivePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractivePlatform::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AVoronoiTerrainCharacter* Char = Cast<AVoronoiTerrainCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Char->GetCapsuleComponent())
		{
			PlatformComponent->SetStaticMesh(ActiveMesh);
			Char->GetCharacterMovement()->JumpZVelocity *= 1.2;
		}
	}
}

void AInteractivePlatform::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AVoronoiTerrainCharacter* Char = Cast<AVoronoiTerrainCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Char->GetCapsuleComponent())
		{
			PlatformComponent->SetStaticMesh(InactiveMesh);
			Char->GetCharacterMovement()->JumpZVelocity /= 1.2;
		}
	}
}

FActorOBB AInteractivePlatform::GetPlatformOBB()
{
	const auto Transform = GetTransform();
 
	// Get World space Location.
	const FVector Center = Transform.TransformPosition(PlatformAABB.GetCenter());
 
	// And World space extent
	const FVector Extent = PlatformAABB.GetExtent();
	const FVector Forward = Transform.TransformVector(FVector::ForwardVector * Extent.X);
	const FVector Right = Transform.TransformVector(FVector::RightVector * Extent.Y);
	const FVector Up = Transform.TransformVector(FVector::UpVector * Extent.Z);
 
	// Now you have an oriented bounding box represented by a `Center` and three extent vectors.
	FActorOBB OrientedBox;
	OrientedBox.Center = Center;
	OrientedBox.Forward = Forward;
	OrientedBox.Right = Right;
	OrientedBox.Up = Up;
 
	return OrientedBox;
}


// FOrientedBox GetOrientedBoxByOcclusionActor()
// {
// 	auto TransformTargetPts = [&](FBox& LocalAABBox,const FMatrix& ActorRotationMatrix,FOrientedBox& OBB) {
// 		FVector LocalOrigin, LocalExtent;
// 		LocalAABBox.GetCenterAndExtents(LocalOrigin, LocalExtent);
// 		OBB.Center = ActorRotationMatrix.TransformPosition(LocalOrigin);
// 		OBB.ExtentX = ActorRotationMatrix.TransformVector(FVector(LocalExtent.X, 0, 0)).Size();
// 		OBB.ExtentY = ActorRotationMatrix.TransformVector(FVector(0, LocalExtent.Y, 0)).Size();
// 		OBB.ExtentZ = ActorRotationMatrix.TransformVector(FVector(0, 0, LocalExtent.Z)).Size();
//
// 		// Set the Axis
// 		OBB.AxisX = ActorRotationMatrix.GetScaledAxis(EAxis::X).GetSafeNormal();
// 		OBB.AxisY = ActorRotationMatrix.GetScaledAxis(EAxis::Y).GetSafeNormal();
// 		OBB.AxisZ = ActorRotationMatrix.GetScaledAxis(EAxis::Z).GetSafeNormal();
// 	};
//
// 	AActor* Actor = Cast<AActor>(OcclusionActor.OcclusionActor);
// 	if(Actor)
// 	{
// 		auto LocalAABBox =  Actor->CalculateComponentsBoundingBoxInLocalSpace(true);
// 		FMatrix ActorRotationMatrix = Actor->GetActorTransform().ToMatrixWithScale();
// 		FOrientedBox OBB;
// 		TransformTargetPts(LocalAABBox,ActorRotationMatrix,OBB);
// 		return OBB;
// 	}
// 	if (OcclusionActor.InstanceIndex!=INDEX_NONE)
// 	{
// 		UStaticMesh* SM = nullptr;
// 		if(OcclusionActor.OcclusionActor->IsA(UStaticMesh::StaticClass()))
// 		{
// 			SM = Cast<UStaticMesh>(OcclusionActor.OcclusionActor);
// 		}
// 		else if(OcclusionActor.OcclusionActor->IsA(UFoliageType::StaticClass()))
// 		{
// 			UFoliageType* FT = Cast<UFoliageType>(OcclusionActor.OcclusionActor);
// 			SM =  Cast<UStaticMesh>(FT->GetSource());
// 		}
// 		if(IsValid(SM))
// 		{
// 			
// 			FBox StaticMeshBoundingBox = SM->GetBoundingBox();
// 			FMatrix ActorRotationMatrix = OcclusionActor.Transform.ToMatrixWithScale();
// 			FOrientedBox OBB;
// 			TransformTargetPts(StaticMeshBoundingBox,ActorRotationMatrix,OBB);
// 			return OBB;
// 				
// 		}
// 	}
// 	
// 	return FOrientedBox();
// }