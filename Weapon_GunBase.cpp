
#include "Weapon_GunBase.h"
#include "GunGripEnum.h"
#include "GunGrabType.h"
#include "DefineCollisionProfile.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "G_GameControl.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/DecalComponent.h"
#include "DamageInterface.h"
AWeapon_GunBase::AWeapon_GunBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	EjectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EjectorMesh"));
	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	SafetyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafetyMesh"));
	PistolGripCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PistolGripCollision"));
	HandGuardCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("HandGuardCollision"));
	MagazineWellCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("MagazineWellCollision"));
	SilencerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("SilencerCollision"));
	HandGuardDummyHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandGuardDummyHand"));
	ChamberRound = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChamberRound"));
	ChargingHandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChargingHandleMesh"));
	ChargingHandleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ChargingHandleCollision"));
	ChargingHandleDummyHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharingHandleDummyHand"));
	TriggerTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TriggerTimeline"));

	GunMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	EjectorMesh->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	TriggerMesh->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	SafetyMesh->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	PistolGripCollision->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	HandGuardCollision->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	MagazineWellCollision->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	SilencerCollision->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	HandGuardDummyHand->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	ChamberRound->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	ChargingHandleMesh->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepWorldTransform);
	ChargingHandleCollision->AttachToComponent(ChargingHandleMesh, FAttachmentTransformRules::KeepWorldTransform);
	ChargingHandleDummyHand->AttachToComponent(ChargingHandleMesh, FAttachmentTransformRules::KeepWorldTransform);


	ChargingHandleCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon_GunBase::ChargingHandleBeginOverlap);
	ChargingHandleCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon_GunBase::ChargingHandleEndOverlap);
	HandGuardCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon_GunBase::HandGuardEndOverlap);

	TriggerTimelineFunction.BindUFunction(this, FName("TriggerTimelineInterpReturn"));
	TriggerTimeline->AddInterpFloat(TriggerTimelineCurve, TriggerTimelineFunction);
	TriggerTimeline->SetTimelineLength(0.07f);

	TriggerTimeline->SetTimelineLength(0.22f);

	ChargingHandleStartLocation = 0.0f;
	AimingSocket = TEXT("None");
}

void AWeapon_GunBase::ChargingHandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(ChargingHandleController))
	{
		ChargingHandleController = Cast<AVrPlayerMotionController>(OtherActor);
		CharginghandleOverlapped = true;
	}
}

void AWeapon_GunBase::ChargingHandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ChargingHandleController == OtherActor && CharginghandleOverlapped)
	{
		FTimerHandle DelayHandle;
		float DelayTime = 0.2f;

		// Delay
		GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([&]()
			{
				if (!OverlappedComponent->IsOverlappingActor(ChargingHandleController))
				{
					ChargingHandleReleased();
					ChargingHandleOverlapEnd.Broadcast();
				}
				
				// TimerHandle �ʱ�ȭ
				GetWorld()->GetTimerManager().ClearTimer(DelayHandle);
			}), DelayTime, false);	// �ݺ��Ϸ��� false�� true�� ����

	}
}

void AWeapon_GunBase::BeginPlay()
{
	Super::BeginPlay();

	MotionControllerPawn = Cast<AVrPlayerMotionControllerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
}

FVector AWeapon_GunBase::HandRelativeLocaion()
{
	// ��ġ�� ������ Ʈ�������� ������ ��ȯ
	return UKismetMathLibrary::InverseTransformLocation
	(
		GunMesh->GetComponentTransform(),
		ChargingHandleController->GrabSphere->GetComponentLocation()
	);
}

float AWeapon_GunBase::ChargingHandleDistance(bool isDebug)
{
	// X ���� ���� �̵� ����(����/����)
	// X ���� 0�������� ������ �� �ְ� �����̴��� ó�� ��������� ��ġ�� �� ��
	float ChargingHandleDistance = HandRelativeLocaion().X - ChargingHandleStartLocation;

	// �����
	if (isDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChargingHandleDistance = %f"), ChargingHandleDistance);
		if (Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->IsScreenDebuging())
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1, 5.f, FColor::Cyan, FString::Printf(TEXT("ChargingHandleDistance = %f"), ChargingHandleDistance)
			);
		}
	}

	return ChargingHandleDistance;
}

bool AWeapon_GunBase::IsChargingHandleGrabbed()
{
	// grab ������ ��
	bool isGrabed = (ChargingHandleController->GripState == EGunGripEnum::Grab);
	// grip action�� ��
	bool isGripping = (ChargingHandleController->GripMode == EGunGrabTypeEnum::Gripping);
	// ���� �ݸ����� ��Ʈ�ѷ��� �浹���� ���
	bool isOverlapped = ChargingHandleCollision->IsOverlappingActor(ChargingHandleController);
	return (isGrabed && isGripping && isOverlapped);
}

bool AWeapon_GunBase::IsChargingHandleReset()
{
	return UKismetMathLibrary::EqualEqual_VectorVector(ChargingHandleMesh->GetRelativeLocation(), FVector(0.0f, 0.0f, 0.0f), 0.01f);
}

void AWeapon_GunBase::RelaodingGrab(bool isReleased)
{
	// Grab
	if (!isReleased)
	{
		if (NeedToShowHandMeshWhenHandleIsGrabbed)
		{
			NeedToShowHandMeshWhenHandleIsGrabbed = false;
			ChargingHandleMoving = true;
			ToggleChargingHandleDummyHand(true);
			UseChargingHandleGripsInBlendspace(true);

			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChargingHandleReleasedSoundEffect, ChargingHandleCollision->GetComponentLocation());
			
			if (IsValid(ChargingHandleController))
			{
				ChargingHandleController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.3f, false);
				ChargingHandleController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectAuto, 0.2f, true);
				ChargingHandleOGrabbed.Broadcast();
			}
		}
		NeedToStopVibrationWhenHandleIsReleased = true;
	}
	// Release
	else
	{
		if (NeedToStopVibrationWhenHandleIsReleased)
		{
			NeedToStopVibrationWhenHandleIsReleased = false;
			ToggleChargingHandleDummyHand(false);
			UseChargingHandleGripsInBlendspace(false);
			
			EControllerHand WhichHand = (PrimaryHand == EControllerHand::Left) ? EControllerHand::Right : EControllerHand::Left;
			UGameplayStatics::GetPlayerController(GetWorld(), 0)->StopHapticEffect(WhichHand);
		}
		NeedToShowHandMeshWhenHandleIsGrabbed = true;
	}
}

void AWeapon_GunBase::RelaodingPull(bool isReleased)
{
	// Grab
	if (!isReleased)
	{
		// ��Ʈ�ѷ��� �����̴��� ��Ҵ��� Ȯ��
		if (ChargingHandleMoving)
		{
			// ���� �ѹ� �����̴� ������ ����
			SetChargingHandleStartLocation();

			// �ִ�� ������ �˻�
			if (ChargingHandleDistance(false) > ChargingHandleMaxDistance)
			{
				float HandleXLocation = FMath::Clamp(ChargingHandleDistance(false), ChargingHandleMaxDistance, 0.0f);
				SetActorRelativeLocation(FVector(HandleXLocation, 0, 0));
			}
			else 
			{
				FullyExtended();
				RelaodingRelease();
			}
		}
	}
	// Release
	else
	{
		NeedToSetChargingHandleStartLocation = true;
	}
}

void AWeapon_GunBase::RelaodingRelease()
{
	
	if (IsChargingHandleReset())
	{
		ChargingHandleMoving = false;
		if (IsValid(ChargingHandleController))
		{
			ChargingHandleController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.4f, false);
		}
		NeedToResetSliderLocation = true;
	}
	else
	{
		FVector NewChargingHandlePostion(UKismetMathLibrary::FInterpTo_Constant(ChargingHandleMesh->GetRelativeLocation().X, 0.0f, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 75.0f), 0.0f, 0.0f);
		// �׸��� ������ ���� ��ġ�� �ٽ� �����̵�
		ChargingHandleMesh->SetRelativeLocation(NewChargingHandlePostion);
	}
	
}

void AWeapon_GunBase::FullyExtended()
{
	if (NeedToResetSliderLocation)
	{
		NeedToResetSliderLocation = false;

		// ���� & ��ƽ �ǵ��
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChargingHandleReleasedSoundEffect, ChargingHandleCollision->GetComponentLocation());
		ChargingHandleController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.9f, false);
		GrabbingMotionController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.7f, false);

		// ���� ź �غ�
		if (!WeaponCooked)
		{
			LoadNextRoundIntoChamber();
			// �ε� ������ ��� ���� ����
			if (OneInTheChamber)
			{
				WeaponCooked = true;
				Magazine->RemoveRound();
			}
		}
	}

}

void AWeapon_GunBase::SetChargingHandleStartLocation()
{
	if (NeedToSetChargingHandleStartLocation)
	{
		ChargingHandleStartLocation = HandRelativeLocaion().X - ChargingHandleMesh->GetRelativeLocation().X;
		NeedToSetChargingHandleStartLocation = false;
	}
}

void AWeapon_GunBase::ChargingHandleReleased()
{
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->StopHapticEffect(ChargingHandleController->Hand);
	ChargingHandleController->ChargingHandleGripped = false;
	CharginghandleOverlapped = false;
	if (ChargingHandleMoving)
	{
		ToggleChargingHandleDummyHand(false);
	}

	ChargingHandleController = nullptr;
}

void AWeapon_GunBase::ToggleChargingHandleDummyHand(bool Visibility)
{
	if (UKismetSystemLibrary::IsValid(ChargingHandleDummyHand->SkeletalMesh))
	{
		ChargingHandleController->HandMesh->SetVisibility(!Visibility, true);
		ChargingHandleController->SecondaryDummyGrippingHand = (Visibility ? ChargingHandleDummyHand : nullptr);
	}

}

void AWeapon_GunBase::LoadNextRoundIntoChamber()
{
	if (UKismetSystemLibrary::IsValid(Magazine))
	{
		// �ּ� �ѹ��̶� �������� �غ� �Ϸ�
		if (Magazine->Rounds > 0)
		{
			OneInTheChamber = true;
			WeaponCooked = true;
			ChamberRound->SetVisibility(true);
		}
	}
}

void AWeapon_GunBase::MagRelesing()
{
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle,this,&AWeapon_GunBase::ResetMagCollision, 0.5f, false);	// �ݺ��Ϸ��� false�� true�� ����
}

void AWeapon_GunBase::ResetMagCollision()
{
	if (IsValid(Magazine))
	{
		Magazine->Magazine->SetCollisionResponseToChannel(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->Weapon, ECollisionResponse::ECR_Block);
		Magazine->MagazineReadyToLoad = true;
		Magazine->PlayerHandRef = nullptr;
		Magazine = nullptr;
	}

}

bool AWeapon_GunBase::ReadyToFire()
{
	if (OneInTheChamber && WeaponCooked)
	{
		MagazineReady = true;
		WeaponCooked = false;
		ChamberRound->SetVisibility(false);
		OneInTheChamber = false;
		if (UKismetSystemLibrary::IsValid(Magazine) && SemiAutomatic)
		{
			LoadNextRoundIntoChamber();
			if (OneInTheChamber)
			{
				Magazine->RemoveRound();
			}
		}
	}

	return MagazineReady;
}

void AWeapon_GunBase::UseChargingHandleGripsInBlendspace(bool ChargingHandleGripped)
{
	ChargingHandleController->ChargingHandleGripped = ChargingHandleGripped;
	ChargingHandleController->Weapon = ChargingHandleGripped ? Weapon : EWeaponTypes::None;
	
}

void AWeapon_GunBase::TriggerPulled()
{
	WeaponTriggerPulled.Broadcast();
	TriggerTimeline->Play();
}

void AWeapon_GunBase::SingleShot()
{
	WeaponFired.Broadcast();
}

void AWeapon_GunBase::SingleShotSoundEffects()
{
	if (IsValid(SingleShotSoundEffect))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),SingleShotSoundEffect, GunMesh->GetSocketLocation(FName("Barrel")));
	}
}

void AWeapon_GunBase::GunAiming()
{
	// FindLookatRotation�� ����Ͽ� �� ������ ������ �� ��� ��Ʈ�ѷ��� ȸ���� ����
	if (HandguardGripped && HasHandguard && IsValid(GrabbingMotionController))
	{
		GrabbingMotionController->Dummy->SetWorldRotation(MotionControllerLookAtRotation());
	}
}

FRotator AWeapon_GunBase::MotionControllerLookAtRotation()
{
	// �ٸ� ������ ����ϴ� ��� �Ļ� ��ü���� AimingSocket ������
	FName CurrentSocketName = AimingSocket == TEXT("None") ? TEXT("DefaultHandguardGrip") : AimingSocket;
	
	// ������κ��� ȸ�� �� ����
	FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(
		GrabbingMotionController->MotionController->GetComponentLocation(),
		SecondaryMotionController->HandMesh->GetSocketLocation(CurrentSocketName)
	);

	return FRotator(
		GrabbingMotionController->MotionController->GetComponentRotation().Vector().X, 
		LookAt.Vector().Y - GrabbingMotionController->HandMesh->GetRelativeRotation().Vector().Y, 
		LookAt.Vector().Z
	);
}

void AWeapon_GunBase::HandGuardRelase(UObject* Hand)
{
	if (Hand == SecondaryMotionController && IsValid(SecondaryMotionController))
	{
		SecondaryMotionController->Weapon = EWeaponTypes::None;
		SecondaryMotionController->HandguardGripped = false;
		// ���߰� �ִ� ���� �� ����ȭ
		SecondaryMotionController->HandMesh->SetVisibility(true);

		// ���� �յ��� ��Ȱ��ȭ
		HandGuardDummyHand->SetVisibility(false);
		ChargingHandleDummyHand->SetVisibility(false);

		UGameplayStatics::GetPlayerController(GetWorld(), 0)->StopHapticEffect(SecondaryMotionController->Hand);
		ResetHandRotation();

		// ���۷��� ����
		SecondaryMotionController = nullptr;
		HandguardGripped = false;
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("hand guard released"), true, true, FLinearColor::Green, 2.0f);
	}
	else
		UE_LOG(LogTemp, Log, TEXT("secondary motion controller is not valid"));
}

void AWeapon_GunBase::ResetHandRotation()
{
	if (IsValid(GrabbingMotionController))
	{
		GrabbingMotionController->Dummy->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	}
}

void AWeapon_GunBase::ReportNoiseEventToAI()
{

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GunMesh->GetSocketLocation(FName("Barrel")), 1.0f, this, MaxAudibleRange, FName("Weapon"));
}

void AWeapon_GunBase::ParticleEffects()
{
	if (IsValid(MuzzleFlashEmitter))
	{
		FTransform BarrelSocketTransform = GunMesh->GetSocketTransform(FName("Barrel"));
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleFlashEmitter,
			GunMesh,
			FName(""),
			BarrelSocketTransform.GetLocation(),
			BarrelSocketTransform.GetRotation().Rotator(),
			FVector(1.0f, 1.0f, 1.0f)
		);
	}


}

void AWeapon_GunBase::RecoilEvent()
{
	RandomiseRecoil();
	PlayRecoilAndEjectorTimeline();
}

FHitResult AWeapon_GunBase::LineTraceFromBarrel()
{
	FVector BarrelStartVector = GunMesh->GetSocketLocation(FName("Barrel"));
	FVector BarrelEndVector = BarrelStartVector + (UKismetMathLibrary::GetForwardVector(GunMesh->GetSocketRotation(FName("Barrel"))) * 200000.0f);
	
	FHitResult OutHit;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		BarrelStartVector,
		BarrelEndVector,
		Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->EnemyBoneTrace,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	BulletRicocheted = CalculateRicochetAngle(0.35f, 600.0f, OutHit.Distance, BarrelEndVector, OutHit.Location, OutHit.ImpactNormal, 0.35f);
	if (BulletRicocheted)
	{
		return OutHit;
	}
	else
	{
		UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			BarrelStartVector,
			BarrelEndVector,
			Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->Visibility,
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::None,
			OutHit,
			true
		);
		BulletRicocheted = CalculateRicochetAngle(0.35f, 600.0f, OutHit.Distance, BarrelEndVector, OutHit.Location, OutHit.ImpactNormal, 0.35f);
		return OutHit;
	}
}

bool AWeapon_GunBase::CalculateRicochetAngle(float MaxAngleForRicochet, float MinDistanceForRicochet, float Distance, FVector TraceDirection, FVector HitLocation, FVector ImpactNormal, float LikeihoodOfRicochet)
{

	return
		(MaxAngleForRicochet < UKismetMathLibrary::Abs(UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::Normal(ImpactNormal, 0.0001f), UKismetMathLibrary::Normal(TraceDirection, 0.0001f)))) &&
		(Distance > MinDistanceForRicochet) &&
		UKismetMathLibrary::RandomBoolWithWeight(LikeihoodOfRicochet);
}

void AWeapon_GunBase::BulletDecals(FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		UDecalComponent* BulletDecalComponent =
			UGameplayStatics::SpawnDecalAttached(
				BulletDecal,
				FVector(2.0f, 2.0f, 2.0f),
				Hit.Component.Get(),
				FName("None"),
				Hit.ImpactPoint,
				UKismetMathLibrary::Conv_VectorToRotator(Hit.Normal),
				EAttachLocation::KeepRelativeOffset,
				10.0f
			);
		if (BulletDecalComponent)
		{
			BulletDecalComponent->SetFadeScreenSize(0.001f);
			BulletDecalComponent->SetFadeIn(0.0f, 0.01f);
			BulletDecalComponent->SetFadeOut(15.0f, 15.0f, false);
		}
	}
}

void AWeapon_GunBase::BulletImpactSoundAndParticles(FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		UGameplayStatics::SpawnEmitterAttached(
			ImpactParticle,
			Hit.Component.Get(),
			FName("None"),
			Hit.ImpactPoint,
			UKismetMathLibrary::Conv_VectorToRotator(Hit.Normal),
			FVector(1.0f, 1.0f, 1.0f),
			EAttachLocation::KeepRelativeOffset
		);
		UGameplayStatics::SpawnSoundAtLocation
		(
			GetWorld(),
			ImpactSound,
			Hit.ImpactPoint,
			FRotator(0.0f,0.0f,0.0f),
			1.0f,
			1.0f,
			0.0f
			);
	}
}

void AWeapon_GunBase::ApplyPhysicsAndDamage(FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		if (UKismetSystemLibrary::DoesImplementInterface(Hit.Actor.Get(), UDamageInterface::StaticClass()))
		{
			Cast<IDamageInterface>(Hit.Actor.Get())->BulletDamage(Hit.BoneName, Hit.ImpactNormal * (ImpactForce / 2.0f), Damage, Dismember, this);
		}
		else
		{
			if (Hit.Component.Get()->IsSimulatingPhysics())
			{
				Hit.Component.Get()->AddImpulse(Hit.ImpactNormal * ImpactForce, FName("None"), false);
			}
		}
		
	}
}

void AWeapon_GunBase::RandomiseRecoil()
{
	// ������� �����ϸ� �ݵ� ����
	if (HandguardGripped)
	{
		RandomRecoilY = UKismetMathLibrary::RandomFloatInRange(-0.25f, 0.25f);
		RandomRecoilZ = UKismetMathLibrary::RandomFloatInRange(0.0f, 0.25f);
	}
	else 
	{
		RandomRecoilY = UKismetMathLibrary::RandomFloatInRange(-0.65, 0.65f);
		RandomRecoilZ = UKismetMathLibrary::RandomFloatInRange(0.0f, 0.75f);
	}

}

void AWeapon_GunBase::ReleaseMagazine()
{
	if (IsValid(Magazine))
	{
		Magazine->MagazineReadyToLoad = false;
		UE_LOG(LogTemp, Warning, TEXT("mag release"));
		if (Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->IsScreenDebuging())
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1, 5.f, FColor::Cyan, FString::Printf(TEXT("mag release"))
			);
		}

		Magazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// �ε巴�� ���������� ��� �ݸ��� ����
		Magazine->Magazine->SetCollisionResponseToChannel(ECC_Weapon,ECollisionResponse::ECR_Ignore);
		// �ݸ����� �ٽ� �۵��ϵ��� Ÿ�̸� ����
		MagRelesing();
		Magazine->Magazine->SetSimulatePhysics(true);

		if (IsValid(GrabbingMotionController))
		{
			GrabbingMotionController->RumbleController(Cast<UG_GameControl>(UGameplayStatics::GetGameInstance(GetWorld()))->HapticEffectSingle, 0.7f, false);
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), MagOut, GunMesh->GetComponentLocation());
		}
	}
}

void AWeapon_GunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool isReleased = false;
	// ��Ʈ�ѷ��� ���� �ݸ����� ���� ������
	if (IsValid(ChargingHandleController))
	{
		if (IsChargingHandleGrabbed() && !HandguardGripped)
		{
			// Grabbed
			isReleased = false;
		}
		else 
		{
			// Released
			isReleased = true;
		}
		RelaodingGrab(isReleased);
		RelaodingPull(isReleased);
	}
	else 
	{
		if (ChargingHandleMoving || !IsChargingHandleReset())
		{
			RelaodingRelease();
		}
	}
}

void AWeapon_GunBase::TriggerTimelineInterpReturn(float Value)
{ 
	TriggerMesh->SetRelativeRotation(FRotator(0.0f, UKismetMathLibrary::Lerp(0.0f, -15.0f, Value), 0.0f));
}

void AWeapon_GunBase::PlayRecoilAnimationAndEjectorAnimation(float SemiAutoRecoil, float SingleShotRecoil, float EjectorPosition)
{
	float CurrentDistance = HandguardGripped ? RecoilDistanceSingleGrip : RecoilDistanceDualGrip;
	if (UKismetSystemLibrary::IsValid(PrimaryHandMesh) && CurrentDistance > 0.0f)
	{
		// �ݵ� ȿ���� ���� ���� X�� �������� ������
		FVector Axis =
			UKismetMathLibrary::VLerp(
				MotionControllerLocation,
				FVector
				(
					MotionControllerLocation.X - CurrentDistance, 
					MotionControllerLocation.Y + RandomRecoilY, 
					MotionControllerLocation.Z + RandomRecoilZ
				),
				(SemiAutomatic ? SemiAutoRecoil : SingleShotRecoil)
			);
		PrimaryHandMesh->SetRelativeLocation(Axis);

		PrimaryHandMesh->SetRelativeRotation(
			FRotator(
			MotionControllerRotation.Vector().X,
			UKismetMathLibrary::Lerp(
				MotionControllerRotation.Vector().Y,
				(HandguardGripped ? RecoilDistanceDualGrip : RecoilDistanceSingleGrip) + MotionControllerRotation.Vector().Y,
				(SemiAutomatic ? SemiAutoRecoil : SingleShotRecoil)
				),
			MotionControllerRotation.Vector().Z
			)
		);
	}

	if (UKismetSystemLibrary::IsValid(EjectorMesh) && SemiAutomatic)
	{
		EjectorMesh->SetRelativeLocation
		(
			FVector
			(
				UKismetMathLibrary::Lerp(0.0f, EjectorDistance, EjectorPosition),
				0.0f,
				0.0f
			)
		);
	}

}

void AWeapon_GunBase::HandGuardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HandguardGripped)
	{
		HandGuardRelase(OtherActor);
	}
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("handguard not gripped"), true, true, FLinearColor::Green, 2.0f);
	}
}
