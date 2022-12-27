// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	None,
	Flashbang,
	Handgun,
	Assault_Rifle,
	Sniper_Rifle,
	Shotgun
};