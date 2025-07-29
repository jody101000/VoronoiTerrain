// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatformComponent.h"
#include "Components/BoxComponent.h"
#include "InteractivePlatform.generated.h"

USTRUCT()
struct FActorOBB
{
	GENERATED_BODY()
	
	FVector Center;
 
	FVector Forward;
	FVector Right;
	FVector Up;
 
	FActorOBB()
		: Center(0.0f, 0.0f, 0.0f),
		  Forward(0.0f, 0.0f, 0.0f),
		  Right(0.0f, 0.0f, 0.0f),
		  Up(0.0f, 0.0f, 0.0f)
	{
	}
};

UCLASS()
class VORONOITERRAIN_API AInteractivePlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractivePlatform();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	virtual void Tick(float DeltaTime) override;


private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlatformComponent;

	UPROPERTY(EditAnywhere)
	UBoxComponent* PlatformTriggerVolume;

	UPROPERTY(EditAnywhere)
	UStaticMesh* InactiveMesh;

	UPROPERTY(EditAnywhere)
	UStaticMesh* ActiveMesh;

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FActorOBB GetPlatformOBB();

	UPROPERTY()
	FBox PlatformAABB;

	UPROPERTY()
	FActorOBB PlatformOBB;

	
};
