

#include "GravityGrabModule.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "IPickUpOutline.h"
#include "Kismet/KismetSystemLibrary.h"
AGravityGrabModule::AGravityGrabModule()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGravityGrabModule::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGravityGrabModule::CheckGravityGrab()
{
	if (PeformingGravityGrab)
	{
		if (IsValid(GravityGrabActor))
		{
			// ��� ���� �ӵ� Ȯ��
			if (UKismetMathLibrary::VSizeSquared(GravityGrabActor->GetVelocity()) < 100.0f)
			{
				// ���� ȿ���� �Ϸ�Ǿ����� ����
				GravityGrabExitTimer += UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

				if (GravityGrabExitTimer > GravityGrabExitTime)
				{
					Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, false);
					PeformingGravityGrab = false;
					GravityGrabActor = nullptr;
					GravityGrabExitTimer = 0.0f;
				}
			}
			else 
			{
				GravityGrabExitTimer = 0.0f;
			}
			
		}
		// ��� ���Ͱ� ��ȿ���� ������ ����
		else 
		{
			PeformingGravityGrab = false;
			GravityGrabExitTimer = 0.0f;
			return;
		}
	}
	// ���� �� �ִ� ��ü���� ���Ǿ� �ݸ������� �˻�
	else 
	{
		FTransform SocketTransform = CharacterRef->HandMesh->GetSocketTransform(TraceStartSocketName, ERelativeTransformSpace::RTS_World);
		
		// Start
		FVector StartVector = SocketTransform.GetLocation();
		// End
		FVector EndVector = SocketTransform.GetLocation() + CharacterRef->GravityGrabPointer->GetForwardVector() * GravityGrabRange;

		TArray<AActor*> IgnoredActor;
		FHitResult OutHit;
		bool success = UKismetSystemLibrary::SphereTraceSingleForObjects(
			GetWorld(), 
			StartVector, 
			EndVector, 
			5.0f, 
			ObjectTypes, 
			true, 
			IgnoredActor,
			EDrawDebugTrace::ForOneFrame, 
			OutHit,
			true
		);

		if (success)
		{
			// �������̽� ���͸� ���� ���� �� �ִ� ��ü�� �˻�
			if (UKismetSystemLibrary::DoesImplementInterface(OutHit.GetActor(), UIPickUpOutline::StaticClass()))
			{
				// ��ȣ�ۿ� ������ ������Ʈ�� ������ ��� ���Ϳ� ����
				if (IsValid(GravityGrabActor))
				{
					Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, false);
				}
				GravityGrabActor = OutHit.GetActor();

				// ����� �̹� �տ� ��� �ִ� ��� ����, �ƴϸ� ��� ��ü ���̶���Ʈ
				if (IsValid(CharacterRef->AttachedActor))
				{
					bool Highlight = CharacterRef->AttachedActor == GravityGrabActor ? true : false;
					Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, Highlight);
					HasGravityGrabTarget = Highlight;
					PeformingGravityGrab = Highlight;
					GravityGrabActor = nullptr;
				}
				else
				{
					HasGravityGrabTarget = true;
					Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, true);
				}
			}
			// ���� �� �ִ� ��ü�� ã�� �� ������ �ߴ�
			else 
			{
				AbortGravityGrab();
			}
			
		}
		else 
		{
			AbortGravityGrab();
		}
	}
}

void AGravityGrabModule::AbortGravityGrab()
{
	HasGravityGrabTarget = false;
	if (IsValid(GravityGrabActor))
	{
		Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, false);
		GravityGrabActor = nullptr;
	}
}

void AGravityGrabModule::DoGravityGrab()
{
	if (HasGravityGrabTarget)
	{
		// ��ü�� ��� ������ ��� ���� Ȯ��
		PeformingGravityGrab = true;
		CharacterRef->GrabSphere->SetSphereRadius(GrabSphereRadiusGravityGrab);

		if (IsValid(GravityGrabActor))
		{
			// ��� ��ü�� ��ȿ�� ��� ��ü �߻�
			FVector LaunchVelocity = TraceGravityGrabPath(GravityGrabActor, CharacterRef->HandMesh->GetSocketLocation(TraceStartSocketName));
			if (IsValid(GravityGrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass())))
			{
				GravityGrabPrimitive = Cast<UPrimitiveComponent>(GravityGrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
				GravityGrabPrimitive->SetAllPhysicsLinearVelocity(LaunchVelocity);
				GravityGrabPrimitive->SetLinearDamping(0.0f);
				FName BoneName = TEXT("None");
				// ������鼭 �ٸ� ��ü�� �о���� ������ �����ٰ� ������� ����
				GravityGrabPrimitive->SetMassScale(BoneName, 10000.0f);

				FTimerHandle DelayHandle;
				float DelayTime = 0.2f;

				// Delay
				GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([&]()
					{
						if(IsValid(GravityGrabPrimitive))
							GravityGrabPrimitive->SetMassScale(BoneName, 1.0f);
						// TimerHandle �ʱ�ȭ
						GetWorld()->GetTimerManager().ClearTimer(DelayHandle);
					}), DelayTime, false);	// �ݺ��Ϸ��� false�� true�� ����
			}
		}
	}

}

FVector AGravityGrabModule::TraceGravityGrabPath(AActor* GrabbedActor, FVector HandLocation)
{
	if (IsValid(GrabbedActor))
	{
		// ��Ʈ�ѷ��� ��ü ���� �Ÿ��� ���� ������ ���̸� ����, ������ �� ���� Arc�� ����
		FVector StartPos = GrabbedActor->GetActorLocation();
		float Distance = UKismetMathLibrary::Vector_Distance(StartPos, HandLocation);
		float ArcParam = UKismetMathLibrary::Lerp(0.4f, 0.9f, UKismetMathLibrary::FClamp(Distance / GravityGrabRange, 0.0f, 1.0f));

		// �߻� ���� ���
		FVector OutLaunchVelocity;
		UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), OutLaunchVelocity, StartPos, HandLocation, 0.0f, ArcParam);

		return OutLaunchVelocity;
	}
	else
	{
		// ��� ���Ͱ� ��ȿ���� ���� ���
		return FVector(0.0f,0.0f,0.0f);
	}
	
}


void AGravityGrabModule::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsGravityOn)
	{
		CheckGravityGrab();
	}
	else 
	{
		HasAmmoSteelTarget = false;
		// ���� �� �ִ� ��ü�� ã�� �� ������ �ߴ�
		AbortGravityGrab();
	}
}

