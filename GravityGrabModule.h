#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VrPlayerMotionController.h"
#include "GravityGrabModule.generated.h"

UCLASS()
class VRWEAPONSKIT_API AGravityGrabModule : public AActor
{
	GENERATED_BODY()
	
public:	
	AGravityGrabModule();

protected:
	virtual void BeginPlay() override;
	void CheckGravityGrab();
	void AbortGravityGrab();
	void DoGravityGrab();
	FVector TraceGravityGrabPath(AActor* GrabbedActor, FVector HandLocation);
public:	
	virtual void Tick(float DeltaTime) override;

protected:
	AVrPlayerMotionController* CharacterRef;
	bool PeformingGravityGrab = false;
	bool HasGravityGrabTarget = false;
	bool HasAmmoSteelTarget = false;
	AActor* GravityGrabActor;
	//AActor* AttachedActor;
	float GravityGrabExitTimer = 0.0f;
	float GravityGrabExitTime = 0.1f;
	float GravityGrabRange = 2000.0f;
	float GrabSphereRadiusGravityGrab = 25.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPrimitiveComponent* GravityGrabPrimitive;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName TraceStartSocketName; // Set On BP
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<TEnumAsByte<EObjectTypeQuery>>ObjectTypes;
public:
	bool IsGravityOn = false;
};
