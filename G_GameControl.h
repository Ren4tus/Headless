// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "G_GameControl.generated.h"

/**
 * 
 */
UCLASS()
class VRWEAPONSKIT_API UG_GameControl : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool IsDebugMode = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UHapticFeedbackEffect_Base* HapticEffectSingle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UHapticFeedbackEffect_Base* HapticEffectAuto;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ECollisionChannel> Weapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ETraceTypeQuery> EnemyBoneTrace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ETraceTypeQuery> Visibility;
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsScreenDebuging();
	
	
};
