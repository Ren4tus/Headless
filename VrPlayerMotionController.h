
#pragma once


#include "CoreMinimal.h"
#include "GunGripEnum.h"
#include "GunGrabType.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplay.h"
#include "FutureWatch.h"
#include "SteamVRChaperoneComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include <Runtime/Engine/Classes/Haptics/HapticFeedbackEffect_Curve.h>
#include "WeaponTypes.h"
#include "VrPlayerMotionController.generated.h"

UCLASS()
class VRWEAPONSKIT_API AVrPlayerMotionController : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere)
		TSubclassOf<AFutureWatch> FutureWatchClass;
public:	
	AVrPlayerMotionController();

protected:
	virtual void BeginPlay() override;
	void ReleaseActor();
	void ResetHandRotation();
	void ClearArc();
	bool TraceTeleportDestination(TArray<FVector> TracePoints, FVector NavMeshLocation, FVector TraceLocation);
	void UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void UpdateAnimationStateOfHanMesh();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		AActor* AddSplineBeamMesh();
public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActionModule")
		class AGravityGrabModule* GravityModule;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<USplineMeshComponent*> SplineMeshes;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USphereComponent* GrabSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TEnumAsByte<EGunGrabTypeEnum> GripMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TEnumAsByte<EGunGripEnum> GripState;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TEnumAsByte<EWeaponTypes> HandleingWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USplineMeshComponent* GravityGrabPointer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* SecondaryDummyGrippingHand;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool Engage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AActor* AttachedActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EControllerHand Hand;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool ChargingHandleGripped;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EWeaponTypes Weapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UMotionControllerComponent* MotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UChildActorComponent* FutureWatch;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USceneComponent* Dummy;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* HandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USplineComponent* ArcSpline;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UArrowComponent* ArcDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* TeleportCylinder;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Ring;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Arrow;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* RoomScaleMesh;
	UPROPERTY(BlueprintReadOnly)
		class USteamVRChaperoneComponent* SteamVRChaperone;

	float ChaperoneMeshHeight = 70.0f;
	FTransform HandTransform;
	float WeaponGripRotation = -10.0f;
	float TeleportLaunchVelocity = 900.0f;
	float ProjectNavExtends = 500.0f;
	bool IsRoomScale = false;
	bool HandguardGripped = false;
	bool WantsToGrab = false;
	bool GunGrabbed = false;
	bool IsTeleporterActive = false;
	bool IsValidTeleportDestination = false;
	bool bLastFrameValidDestination = false;
public:
	UFUNCTION(BlueprintCallable)
		void RumbleController(UHapticFeedbackEffect_Base* HapticEffect ,float Intensity, bool Loop);
	UFUNCTION(BlueprintCallable)
		void EngageHand(EGunGrabTypeEnum GripOrGrab);
	UFUNCTION(BlueprintCallable)
		AActor* GetActorNearHand(TSubclassOf<AActor> ClassFilter);
	void MagRelease();
	void SetupRoomScaleOutline();
	void GrabActor(TSubclassOf<AActor> ClassFilter, EGunGrabTypeEnum GripOrGrab);
	void DisengageHand(EGunGrabTypeEnum GripOrGrab);
};
