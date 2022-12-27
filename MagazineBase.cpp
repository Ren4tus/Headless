
#include "MagazineBase.h"
#include "Components/BoxComponent.h"
#include "Weapon_GunBase.h"
#include "Kismet/GameplayStatics.h"
#include "VrPlayerMotionController.h"
#include <VRWeaponsKit/G_GameControl.h>
AMagazineBase::AMagazineBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Magazine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Magazine"));
	InsertCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Insert Collision"));
	PickUpCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Pickup Collision"));
	
	Magazine->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	InsertCollision->AttachToComponent(Magazine, FAttachmentTransformRules::KeepWorldTransform);
	PickUpCollision->AttachToComponent(Magazine, FAttachmentTransformRules::KeepWorldTransform);

	InsertCollision->OnComponentBeginOverlap.AddDynamic(this, &AMagazineBase::InsertBeginOverlap);
	InsertCollision->OnComponentEndOverlap.AddDynamic(this, &AMagazineBase::InsertEndOverlap);
}

void AMagazineBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMagazineBase::PickupEvent(EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* MotionController, USceneComponent* HandMesh)
{
	if (EGunGrabTypeEnum::Grabbing == GripOrGrab)
	{
		PlayerHandRef = MotionController;
		PlayerHandRef->HandleingWeapon = MagazineType;
		PlayerHandRef->Engage = true;

		Magazine->SetSimulatePhysics(false);
		bool isAttachSuccess = RootComponent->K2_AttachToComponent(
			HandMesh,
			SelectSocket(),
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative,
			false
		);
		MagazineReadyToLoad = false;

		// 0.25초 후에 MagazineReadyToLoad 재설정
		// 총에 달려있는 탄창을 집은 경우 - 사운드 재생, 레퍼런스 리셋 
		// BP에서 실행
		EndUpMagazinePickUp(isAttachSuccess);
	}
}

FName AMagazineBase::SelectSocket()
{
	switch (MagazineType)
	{
	case EWeaponTypes::None:
		return FName(TEXT("None"));
		break;
	case EWeaponTypes::Flashbang:
		return FName(TEXT("None"));
		break;
	case EWeaponTypes::Handgun:
		return FName(TEXT("Handgun_MagazineGripSocket"));
		break;
	case EWeaponTypes::Assault_Rifle:
		return FName(TEXT("Assault_rifle_MagazineGripSocket"));
		break;
	case EWeaponTypes::Sniper_Rifle:
		return FName(TEXT("Sniper_rifle_MagazineGripSocket")); 
		break;
	case EWeaponTypes::Shotgun:
		return FName(TEXT("None"));
		break;
	default:
		return FName(TEXT("None"));
		break;
	}

}

void AMagazineBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagazineBase::InsertBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FTimerHandle WaitHandle;
	float WaitTime = 0.1f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			InsertMagazineEvent(OtherActor, OtherComp);

		}), WaitTime, false);
}

void AMagazineBase::InsertMagazineEvent(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	// InsertMagazine안에서 초기화됨
	UStaticMeshComponent* WeaponMesh = nullptr;
	bool IsMagazineCompatible = Cast<IPickUpActor>(OtherActor)->InsertMagazine(OtherComp,MagazineType, WeaponMesh, AttachedGunRef);
	if (IsMagazineCompatible && MagazineReadyToLoad)
	{
		if (IsValid(AttachedGunRef->GrabbingMotionController))
		{
			AttachedGunRef->GrabbingMotionController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.5f, false);
		}
		AttachedGunRef->Magazine = this;
		MagazineWell = OtherComp;

		// 손에서 해제
		DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
	
		// 총에 부착
		bool IsAttachSuccess = Magazine->K2_AttachToComponent(
			WeaponMesh, 
			FName(TEXT("Magazine")), 
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, 
			true);

		if (IsAttachSuccess)
		{
			MagazineReadyToLoad = false;
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), MagazineInAudio, Magazine->GetComponentLocation());
			PlayerHandRef->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.5f, false);
		}
	}
}

void AMagazineBase::InsertEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp == MagazineWell)
	{
		FTimerHandle WaitHandle;
		float WaitTime = 0.25f;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				MagazineReadyToLoad = true;

			}), WaitTime, false);
	}
}

bool AMagazineBase::InsertMagazine(UPrimitiveComponent* OtherComp, EWeaponTypes WeaponType, UStaticMeshComponent* WeaponMesh, AWeapon_GunBase* WeaponActor)
{
	return false;
}

EWeaponTypes AMagazineBase::Pickup(EControllerHand Hand, EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* Actor, USceneComponent* AttachTo, bool& GunGrabbed)
{
	PickupEvent(GripOrGrab, Actor, AttachTo);
	return MagazineType;
}

void AMagazineBase::Drop()
{
	PlayerHandRef->HandleingWeapon = EWeaponTypes::None;
	PlayerHandRef->Engage = false;

	Magazine->SetSimulatePhysics(true);
	DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));


}

void AMagazineBase::RemoveRound()
{
	Rounds--;
	if (Rounds <= ISMCartridge->GetInstanceCount())
	{
		ISMCartridge->RemoveInstance(Rounds);
	}
}

void AMagazineBase::OnConstruction(const FTransform& Transform)
{
	PickUpCollision->SetHiddenInGame(true);
	InsertCollision->SetHiddenInGame(true);

	// 인스턴스 매쉬로 교체
	for (int i = 0; i < CartridgeMeshes.Num(); i++)
	{
		UStaticMeshComponent* Cartridge = CartridgeMeshes[i];
		if (IsValid(Cartridge))
		{
			if (i < Rounds)
			{
				FTransform InstanceTransform = Cartridge->GetRelativeTransform();
				InstanceTransform.GetScale3D() = FVector(1.0f, 1.0f, 1.0f);

				ISMCartridge->AddInstance(InstanceTransform);
			}
			Cartridge->DestroyComponent();
		}
	}
	
}

