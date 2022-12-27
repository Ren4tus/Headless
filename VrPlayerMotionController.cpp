
#include "VrPlayerMotionController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon_GunBase.h"
#include "PickUpActor.h"
#include "Components/SceneComponent.h"
#include "NavigationSystem.h"
#include <VRWeaponsKit/G_GameControl.h>
AVrPlayerMotionController::AVrPlayerMotionController()
{
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Controller"));
	GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Grab Sphere"));
	Dummy = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy"));
	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand Mesh"));
	GravityGrabPointer = CreateDefaultSubobject<USplineMeshComponent>(TEXT("GravityGrab Pointer"));
	TeleportCylinder = CreateDefaultSubobject<USplineMeshComponent>(TEXT("TeleportCylinder"));
	ArcDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ArcDirection"));
	ArcSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ArcSpline"));
	Ring = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Ring"));
	Arrow = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Arrow"));
	RoomScaleMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("RoomScaleMesh"));
	SteamVRChaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>(TEXT("SteamVRChaperone"));

	Scene->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	MotionController->AttachToComponent(Scene, FAttachmentTransformRules::KeepWorldTransform);
	Dummy->AttachToComponent(MotionController, FAttachmentTransformRules::KeepWorldTransform);
	HandMesh->AttachToComponent(Dummy, FAttachmentTransformRules::KeepWorldTransform);
	GrabSphere->AttachToComponent(HandMesh, FAttachmentTransformRules::KeepWorldTransform);
	GravityGrabPointer->AttachToComponent(HandMesh, FAttachmentTransformRules::KeepWorldTransform);
	TeleportCylinder->AttachToComponent(MotionController, FAttachmentTransformRules::KeepWorldTransform);
	ArcDirection->AttachToComponent(HandMesh, FAttachmentTransformRules::KeepWorldTransform);
	ArcSpline->AttachToComponent(HandMesh, FAttachmentTransformRules::KeepWorldTransform);
	Ring->AttachToComponent(TeleportCylinder, FAttachmentTransformRules::KeepWorldTransform);
	Arrow->AttachToComponent(TeleportCylinder, FAttachmentTransformRules::KeepWorldTransform);
	RoomScaleMesh->AttachToComponent(Arrow, FAttachmentTransformRules::KeepWorldTransform);

	//Create the actor as a UChildActorComponent
	FutureWatch = CreateDefaultSubobject<UChildActorComponent>(TEXT("FutureWatch"));
	FutureWatch->SetChildActorClass(FutureWatchClass);
	FutureWatch->CreateChildActor();
}

void AVrPlayerMotionController::BeginPlay()
{
	Super::BeginPlay();
	SetupRoomScaleOutline();

	// Hide until activation of teleporter
	TeleportCylinder->SetVisibility(false, true);
	RoomScaleMesh->SetVisibility(false);

	switch (Hand)
	{
	case EControllerHand::Left:
		Dummy->SetWorldScale3D(FVector(1.0f, -1.0f, 1.0f));
		break;
	case EControllerHand::Right:
		FutureWatch->SetHiddenInGame(true);
		break;
	}
}

void AVrPlayerMotionController::ReleaseActor()
{
	WantsToGrab = false;
	if (IsValid(AttachedActor))
	{
		if (AttachedActor->K2_GetRootComponent()->GetAttachParent() == HandMesh)
		{
			Cast<IPickUpActor>(AttachedActor)->Drop();
			RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.2f, false);
		}
		AttachedActor = nullptr;
		Weapon = EWeaponTypes::None;
		GunGrabbed = false;
	}
}

void AVrPlayerMotionController::ResetHandRotation()
{
	HandMesh->SetRelativeRotation(HandTransform.GetRotation());
	HandMesh->SetRelativeLocation(HandTransform.GetLocation());
}

void AVrPlayerMotionController::ClearArc()
{
	//Checks if it is valid at index zero because index zero is always going to exist if the array has any elements
	if (SplineMeshes.IsValidIndex(0))
	{
		while (SplineMeshes.IsValidIndex(0))
		{
			//This destroys the component but it does not remove it from the array.
			SplineMeshes[0]->DestroyComponent();
			SplineMeshes.RemoveAt(0);
		}
	}

	ArcSpline->ClearSplinePoints(true);
}

bool AVrPlayerMotionController::TraceTeleportDestination(TArray<FVector> TracePoints, FVector NavMeshLocation, FVector TraceLocation)
{
	// 텔레포트 도착 지점 계산
	//UGameplayStatics::Blueprint_PredictProjectilePath_ByObjectType
	FHitResult OutHit;
	TArray<FVector> OutPathPositions;
	FVector OutLastTraceDestination;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	const TArray<AActor*> ActorsToIgnore;
	bool HitResult = UGameplayStatics::Blueprint_PredictProjectilePath_ByObjectType(
		GetWorld(),
		OutHit,
		OutPathPositions,
		OutLastTraceDestination,
		ArcDirection->GetComponentLocation(),
		ArcDirection->GetForwardVector() * TeleportLaunchVelocity,
		true,
		0.0f,
		ObjectTypes,
		true,
		ActorsToIgnore,
		EDrawDebugTrace::Type::ForOneFrame,
		0.0f,
		30.0f,
		2.0f,
		0.0f
	);
	TracePoints = OutPathPositions;
	TraceLocation = OutHit.Location;
	TSubclassOf<UNavigationQueryFilter> FilterClass;
	bool NavResult = UNavigationSystemV1::K2_ProjectPointToNavigation(
		GetWorld(),
		OutHit.Location,
		NavMeshLocation,
		nullptr,
		FilterClass,
		UKismetMathLibrary::MakeVector(ProjectNavExtends, ProjectNavExtends, ProjectNavExtends)
	);
	
	return (HitResult && NavResult);
}

void AVrPlayerMotionController::UpdateArcSpline(bool FoundValidLocation, TArray<FVector> SplinePoints)
{
	if (!FoundValidLocation)
	{
		// Create Small Stub line when we failed to find a teleport location
		SplinePoints.Empty();
		SplinePoints.Add(ArcDirection->GetComponentLocation());
		SplinePoints.Add(ArcDirection->GetComponentLocation() + ArcDirection->GetForwardVector() * 20.0f);
	}
	for (FVector element : SplinePoints)
	{
		ArcSpline->AddSplinePoint(element, ESplineCoordinateSpace::Type::Local, true);
	}
	// Update the point type to create the curve
	ArcSpline->SetSplinePointType(SplinePoints.Num() - 1, ESplinePointType::CurveClamped, true);
	for (int i = 0; i < ArcSpline->GetNumberOfSplinePoints() - 2; i++)
	{
		AActor* Spline = AddSplineBeamMesh();
		Cast<USplineMeshComponent>(Spline)->SetStartAndEnd(
			SplinePoints[i],
			ArcSpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
			SplinePoints[i + 1],
			ArcSpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local)
		);
	}
	
}

void AVrPlayerMotionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 손 애니메이션 갱신

	// 무언가를 잡으려 하는 경우
	if (WantsToGrab)
	{
		// 지금 뭔가를 잡고 있는 경우
		if (AttachedActor != nullptr)
		{
			if (Engage)
			{
				GripState = EGunGripEnum::Engage;
			}
			else
			{
				GripState = EGunGripEnum::Grab;
			}
		}
		else
		{
			GripState = EGunGripEnum::Grab;
		}
	}
	else
	{
		if (GetActorNearHand(TSubclassOf<AActor>()) != nullptr)
		{
			switch (Weapon)
			{
			case EWeaponTypes::None:
			case EWeaponTypes::Flashbang:
				GripState = EGunGripEnum::CanGrip;
				break;
			case EWeaponTypes::Handgun:
			case EWeaponTypes::Assault_Rifle:
			case EWeaponTypes::Shotgun:
			case EWeaponTypes::Sniper_Rifle:
				if (Engage)
				{
					GripState = EGunGripEnum::Engage;
				}
				else
				{
					GripState = EGunGripEnum::CanGrip;
				}
				break;

			}
		}
		else
		{
			GripState = EGunGripEnum::Open;
		}
	}
	UpdateAnimationStateOfHanMesh();

	ClearArc();
	if (IsTeleporterActive)
	{
		TArray<FVector> TracePoints; 
		FVector NavMeshLocation;
		FVector TraceLocation;
		IsValidTeleportDestination = TraceTeleportDestination(TracePoints, NavMeshLocation, TraceLocation);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
		const TArray<AActor*> ActorsToIgnore;
		TeleportCylinder->SetVisibility(IsValidTeleportDestination, true);
		FHitResult OutHit;
		FHitResult* SweepHitResult = nullptr;
		UKismetSystemLibrary::LineTraceSingleForObjects(
			GetWorld(),
			NavMeshLocation,
			NavMeshLocation - FVector(0.0f, 0.0f, -200.0f),
			ObjectTypes,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::Type::None,
			OutHit,
			true);
		TeleportCylinder->SetWorldLocation(
			UKismetMathLibrary::SelectVector(OutHit.ImpactPoint, NavMeshLocation,OutHit.bBlockingHit),
			false,
			SweepHitResult
		);

		// Rumble controller when a valid teleport location was found
		if ((IsValidTeleportDestination && !bLastFrameValidDestination) || (!IsValidTeleportDestination && bLastFrameValidDestination))
		{
			RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.3f, false);
		}
		bLastFrameValidDestination = IsValidTeleportDestination;

		//UpdateArcSpline();
	}
	
}

void AVrPlayerMotionController::RumbleController(UHapticFeedbackEffect_Base* HapticEffect, float Intensity, bool Loop)
{
	UGameplayStatics::GetPlayerController(GetWorld(),0)->PlayHapticEffect(HapticEffect, Hand, Intensity, Loop);
}

void AVrPlayerMotionController::EngageHand(EGunGrabTypeEnum GripOrGrab)
{
	GripMode = GripOrGrab;

	switch (GripMode)
	{
	case EGunGrabTypeEnum::Grabbing:
		// 손 그립 애니메이션 변경
		Engage = true;
		// 방아쇠를 당겼다는 메세지를 전송
		Cast<IPickUpActor>(AttachedActor)->PullTrigger();
		break;
	case EGunGrabTypeEnum::Gripping:
		USkeletalMeshComponent* HandguardDummyHand = nullptr;
		if (Cast<IPickUpActor>(GetActorNearHand(TSubclassOf<AWeapon_GunBase>()))->GripHandGuard(this, HandguardDummyHand))
		{
			SecondaryDummyGrippingHand = HandguardDummyHand;
			HandguardGripped = true;
		}
		else
		{
			GrabActor(TSubclassOf<AWeapon_GunBase>(), EGunGrabTypeEnum::Gripping);
		}
		//GripHandGuard();
		break;
	}
}

void AVrPlayerMotionController::SetupRoomScaleOutline()
{
	FVector OutRectCenter;
	FRotator OutRectRotation;
	float OutSideLengthX, OutSideLengthY;
	UKismetMathLibrary::MinimumAreaRectangle(GetWorld(), SteamVRChaperone->GetBounds(), FVector(0.0f, 0.0f, 1.0f), OutRectCenter, OutRectRotation, OutSideLengthX, OutSideLengthY);

	// Measure Chaperone (Defaults to 100x100 if  roomscale isn't used)
	// Checks if both are not 100.
	IsRoomScale = !(UKismetMathLibrary::NearlyEqual_FloatFloat(OutSideLengthX, 100.0f, 0.01f) && UKismetMathLibrary::NearlyEqual_FloatFloat(OutSideLengthY, 100.0f, 0.01f));
	if (IsRoomScale)
	{
		RoomScaleMesh->SetWorldScale3D(FVector(OutSideLengthX, OutSideLengthY, ChaperoneMeshHeight));
		RoomScaleMesh->SetRelativeRotation(OutRectRotation);
	}
}

void AVrPlayerMotionController::GrabActor(TSubclassOf<AActor> ClassFilter, EGunGrabTypeEnum GripOrGrab)
{
	WantsToGrab = true;
	AActor* NearestMesh = GetActorNearHand(ClassFilter);
	if (IsValid(NearestMesh))
	{
		AttachedActor = NearestMesh;
		
		Weapon = Cast<IPickUpActor>(NearestMesh)->Pickup(Hand, GripOrGrab, this, HandMesh, GunGrabbed);

		RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.75f, false);
		if (GunGrabbed)
		{
			// 손을 회전해서 조준할 때 손목이 편하게 되도록 수정
			HandMesh->SetRelativeRotation(FRotator(HandTransform.GetRotation().X, WeaponGripRotation, HandTransform.GetRotation().Z));
			HandMesh->SetRelativeLocation(HandTransform.GetLocation());

			Cast<AWeapon_GunBase>(AttachedActor)->MotionControllerLocation = HandMesh->GetRelativeLocation();
			Cast<AWeapon_GunBase>(AttachedActor)->MotionControllerRotation = HandMesh->GetRelativeRotation();
		}
		else
		{
			//door
		}
	}
}

void AVrPlayerMotionController::DisengageHand(EGunGrabTypeEnum GripOrGrab)
{
	switch (GripMode)
	{
	case EGunGrabTypeEnum::Grabbing:
		if (GunGrabbed)
		{
			Engage = false;
			Cast<IPickUpActor>(AttachedActor)->ReleaseTrigger();
		}
		else 
		{
			ReleaseActor();
			Engage = false;
			ResetHandRotation();
		}
		break;
	case EGunGrabTypeEnum::Gripping:
		if (GunGrabbed)
		{
			ReleaseActor();
			Engage = false;
			ResetHandRotation();
		}
		else
		{
			if (HandguardGripped)
			{
				Cast<IPickUpActor>(GetActorNearHand(TSubclassOf<AWeapon_GunBase>()))->ReleaseHandGuard(this);
				HandguardGripped = false;
			}
			// 문고리 잡기 해제
			else
			{

			}
		}
		break;
	}
}

AActor* AVrPlayerMotionController::GetActorNearHand(TSubclassOf<AActor> ClassFilter)
{
	TArray<AActor*> OverlappingActors;
	GrabSphere->GetOverlappingActors(OverlappingActors, ClassFilter);

	AActor* NearestOverlappingActor = nullptr;
	float NearestOverlap = 0.0f;
	for (AActor* element : OverlappingActors)
	{
		// 집을 수 있는 액터로 필터
		if (UKismetSystemLibrary::DoesImplementInterface(element, UPickUpActor::StaticClass()))
		{
			float CurrentOverlap = UKismetMathLibrary::VSize(element->GetActorLocation() - GrabSphere->GetComponentLocation());
			if (CurrentOverlap < NearestOverlap)
			{
				NearestOverlappingActor = element;
				NearestOverlap = CurrentOverlap;
			}
		}
	}

	return NearestOverlappingActor;
}

void AVrPlayerMotionController::MagRelease()
{
	Cast<AWeapon_GunBase>(AttachedActor)->ReleaseMagazine();
}
