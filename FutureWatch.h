// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "FutureWatch.generated.h"

/**
 * 
 */
UCLASS()
class VRWEAPONSKIT_API AFutureWatch : public AStaticMeshActor
{
	GENERATED_BODY()
		
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* RoomScaleMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Laser;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UWidgetComponent* SkillUI;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UWidgetComponent* HP;

	AFutureWatch();
};
