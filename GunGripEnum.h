// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
/**
 * 
 */
UENUM(BlueprintType)
enum EGunGripEnum
{
	Open UMETA(DisplayName = "Open"),
	CanGrip UMETA(DisplayName = "CanGrip"),
	Grab UMETA(DisplayName = "Grab"),
	Engage UMETA(DisplayName = "Engage")
};
