#pragma once
#include "VrPlayerMotionController.h"
#include "Components/BoxComponent.h"
#include "Delegates/Delegate.h"
#include "VrPlayerMotionControllerPawn.h"
#include "Components/TimelineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagazineBase.h"
#include "Weapon_GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FNormalDelegate);

UCLASS()
class VRWEAPONSKIT_API AWeapon_GunBase : public AActor
{
	GENERATED_BODY()

public:
	AWeapon_GunBase();

	UFUNCTION()
		void ChargingHandleBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void ChargingHandleEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	FNormalDelegate ChargingHandleOverlapEnd;
	FNormalDelegate ChargingHandleOGrabbed;
	FNormalDelegate WeaponTriggerPulled;
	FNormalDelegate WeaponFired;

protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector HandRelativeLocaion();
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float ChargingHandleDistance(bool isDebug);
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsChargingHandleGrabbed();
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsChargingHandleReset();

	void RelaodingGrab(bool isReleased);
	void RelaodingPull(bool isReleased);
	void RelaodingRelease();
	void FullyExtended();
	void SetChargingHandleStartLocation();
	void ChargingHandleReleased();
	void ToggleChargingHandleDummyHand(bool Visibility);
	void LoadNextRoundIntoChamber();
	void MagRelesing();
	void ResetMagCollision();
	bool ReadyToFire();
	void UseChargingHandleGripsInBlendspace(bool ChargingHandleGripped);
	void TriggerPulled();
	void SingleShot();
	void SingleShotSoundEffects();
	void GunAiming();
	FRotator MotionControllerLookAtRotation();
	void HandGuardRelase(UObject* Hand);
	void ResetHandRotation();
	void ParticleEffects();
	void RecoilEvent();
	FHitResult LineTraceFromBarrel();
	bool CalculateRicochetAngle(float MaxAngleForRicochet,float MinDistanceForRicochet,float Distance,FVector TraceDirection,FVector HitLocation,FVector ImpactNormal,float LikeihoodOfRicochet);
	void BulletDecals(FHitResult Hit);
	void BulletImpactSoundAndParticles(FHitResult Hit);
	void ApplyPhysicsAndDamage(FHitResult Hit);
	void RandomiseRecoil();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void PlayRecoilAndEjectorTimeline();
	UFUNCTION(BlueprintCallable)
		void PlayRecoilAnimationAndEjectorAnimation(float SemiAutoRecoil, float SingleShotRecoil, float EjectorPosition);
	UFUNCTION()
		void HandGuardEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	virtual void Tick(float DeltaTime) override;
	void ReportNoiseEventToAI();
	void ReleaseMagazine();
protected:

	// Root Component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* GunMesh;
	// - Child Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* EjectorMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* TriggerMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* SafetyMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UCapsuleComponent* PistolGripCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* HandGuardCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* MagazineWellCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* SilencerCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* HandGuardDummyHand;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* ChamberRound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* ChargingHandleMesh;
	// - Child Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* ChargingHandleCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* ChargingHandleDummyHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float ChargingHandleStartLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool HandguardGripped;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool ChargingHandleMoving;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float ChargingHandleMaxDistance;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float MaxAudibleRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool CharginghandleOverlapped;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USoundBase* ChargingHandleReleasedSoundEffect;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USoundBase* SingleShotSoundEffect;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UParticleSystem* MuzzleFlashEmitter;

	// Timeline --
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UCurveFloat* TriggerTimelineCurve;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UTimelineComponent* TriggerTimeline;
	UFUNCTION()
		void TriggerTimelineInterpReturn(float Value);
	FOnTimelineFloat TriggerTimelineFunction;
	// -- Timeline

	float RandomRecoilY;
	float RandomRecoilZ;
	float RecoilDistanceSingleGrip = 2.0f;
	float RecoilDistanceDualGrip = 0.75f;
	float EjectorDistance = -4.0f;
	bool BulletRicocheted = false;
	float ImpactForce = -3000.0f;
	float Damage = 2.0f;
	bool Dismember = false;
	bool NeedToSetChargingHandleStartLocation = true;
	bool NeedToResetSliderLocation = true;
	bool NeedToShowHandMeshWhenHandleIsGrabbed = true;
	bool NeedToStopVibrationWhenHandleIsReleased = true;
	bool WeaponCooked = false;
	bool OneInTheChamber = false;
	bool MagazineReady = false;
	bool SemiAutomatic = false;
	bool HasHandguard = true;
	FName AimingSocket;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AVrPlayerMotionController* ChargingHandleController;
	// Primary Motion Controller
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AVrPlayerMotionController* GrabbingMotionController;
	// Secondary Motion Controller
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AVrPlayerMotionController* SecondaryMotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AMagazineBase* Magazine;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EWeaponTypes Weapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EControllerHand PrimaryHand = EControllerHand::Right;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AVrPlayerMotionControllerPawn* MotionControllerPawn;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* PrimaryHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UMaterialInstance* BulletDecal;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UParticleSystem* ImpactParticle;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USoundBase* ImpactSound;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USoundBase* MagOut;

	FVector MotionControllerLocation;
	FRotator MotionControllerRotation;
};
