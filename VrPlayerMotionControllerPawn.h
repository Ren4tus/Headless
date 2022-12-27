// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "VrPlayerMotionControllerPawn.generated.h"

UCLASS()
class VRWEAPONSKIT_API AVrPlayerMotionControllerPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AVrPlayerMotionControllerPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		UStaticMeshComponent* DistortionSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		USphereComponent* DetectionSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		UPostProcessComponent* PostProcessComp;

};
