// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpActor.h"
void IPickUpActor::ReleaseTrigger()
{
}

EWeaponTypes IPickUpActor::Pickup(EControllerHand Hand, EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* Actor, USceneComponent* AttachTo, bool& GunGrabbed)
{
	return EWeaponTypes::None;
}

void IPickUpActor::Drop()
{
}

bool IPickUpActor::PullTrigger()
{
	return false;
}

bool IPickUpActor::InsertMagazine(class UPrimitiveComponent* OverlapeedComponent, EWeaponTypes WeaponType, UStaticMeshComponent* WeaponMesh, AWeapon_GunBase* WeaponActor)
{
	return false;
}

bool IPickUpActor::GripHandGuard(AVrPlayerMotionController* MotionController, USkeletalMeshComponent* HandGuardDummy)
{
	return false;
}

void IPickUpActor::ReleaseHandGuard(AVrPlayerMotionController* MotionController)
{
}
