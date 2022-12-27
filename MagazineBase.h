
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpActor.h"
#include "Sound/SoundBase.h"
#include "MagazineBase.generated.h"

UCLASS()
class VRWEAPONSKIT_API AMagazineBase : public AActor, public IPickUpActor
{
	GENERATED_BODY()
	
public:	
	AMagazineBase();

protected:
	virtual void BeginPlay() override;
	void PickupEvent(EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* MotionController, USceneComponent* HandMesh);
	FName SelectSocket();
public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void EndUpMagazinePickUp(bool isSuccess);
	// OnComponentBeginOverlap
	UFUNCTION()
		void InsertBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
		void InsertMagazineEvent(class AActor* OtherActor, class UPrimitiveComponent* OtherComp);
	// OnComponentEndOverlap
	UFUNCTION()
		void InsertEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// from IPickUpActor
	virtual bool InsertMagazine(class UPrimitiveComponent* OverlapeedComponent, EWeaponTypes WeaponType, UStaticMeshComponent* WeaponMesh, class AWeapon_GunBase* WeaponActor) override;
	UFUNCTION(BlueprintCallable)
		virtual EWeaponTypes Pickup(EControllerHand Hand, EGunGrabTypeEnum GripOrGrab, AVrPlayerMotionController* Actor, USceneComponent* AttachTo, bool& GunGrabbed) override;
	UFUNCTION(BlueprintCallable)
		virtual void Drop() override;

	void RemoveRound();

	virtual void OnConstruction(const FTransform& Transform) override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EWeaponTypes MagazineType = EWeaponTypes::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class AWeapon_GunBase* AttachedGunRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USoundBase* MagazineOutAudio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USoundBase* MagazineInAudio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UPrimitiveComponent* MagazineWell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UStaticMeshComponent*> CartridgeMeshes;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Magazine;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UBoxComponent* InsertCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UBoxComponent* PickUpCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UInstancedStaticMeshComponent* ISMCartridge;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Rounds;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool MagazineReadyToLoad = true;
	// MotionController
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AVrPlayerMotionController* PlayerHandRef;

};
