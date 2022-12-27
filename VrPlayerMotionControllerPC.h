// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VrPlayerMotionControllerPC.generated.h"

/**
 * 
 */
UCLASS()
class VRWEAPONSKIT_API AVrPlayerMotionControllerPC : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable)
		void ControllerInit();

public:
	bool Initialized;
	APawn* CharacterRef;
};
