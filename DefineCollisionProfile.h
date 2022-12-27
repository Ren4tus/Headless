// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define ECC_Magazine ECollisionChannel::ECC_GameTraceChannel1
#define ECC_GravitiyGrabObject ECollisionChannel::ECC_GameTraceChannel2
#define ECC_Weapon ECollisionChannel::ECC_GameTraceChannel3
/**
 * 
 */
class VRWEAPONSKIT_API DefineCollisionProfile
{
public:
	DefineCollisionProfile();
	~DefineCollisionProfile();
};

