// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerControllerHandType.h"
#include "GunGrabType.h"
#include "PickUpActor.generated.h"
class AVrPlayerMotionController;
UINTERFACE(MinimalAPI)
class UPickUpActor : public UInterface
{
	GENERATED_BODY()
};

class VRWEAPONSKIT_API IPickUpActor
{
	GENERATED_BODY()

public:
	virtual void ReleaseTrigger();
	virtual EWeaponTypes Pickup(EControllerHand Hand, EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* Actor, USceneComponent* AttachTo, bool& GunGrabbed);
	virtual void Drop();
	virtual bool PullTrigger();
	virtual bool InsertMagazine(class UPrimitiveComponent* OverlapeedComponent, EWeaponTypes WeaponType, UStaticMeshComponent* WeaponMesh, class AWeapon_GunBase* WeaponActor);
	virtual bool GripHandGuard(AVrPlayerMotionController* MotionController, class USkeletalMeshComponent* HandGuardDummy);
	virtual void ReleaseHandGuard(AVrPlayerMotionController* MotionController);
};
