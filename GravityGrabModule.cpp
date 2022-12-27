

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
			// 대상 액터 속도 확인
			if (UKismetMathLibrary::VSizeSquared(GravityGrabActor->GetVelocity()) < 100.0f)
			{
				// 감속 효과가 완료되었으면 종료
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
		// 대상 액터가 유효하지 않으면 종료
		else 
		{
			PeformingGravityGrab = false;
			GravityGrabExitTimer = 0.0f;
			return;
		}
	}
	// 잡을 수 있는 객체들을 스피어 콜리전으로 검사
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
			// 인터페이스 필터를 통해 잡을 수 있는 객체만 검사
			if (UKismetSystemLibrary::DoesImplementInterface(OutHit.GetActor(), UIPickUpOutline::StaticClass()))
			{
				// 상호작용 가능한 오브젝트가 있으면 대상 액터에 저장
				if (IsValid(GravityGrabActor))
				{
					Cast<IIPickUpOutline>(GravityGrabActor)->ToggleOutline(GravityGrabActor, false);
				}
				GravityGrabActor = OutHit.GetActor();

				// 대상을 이미 손에 쥐고 있는 경우 중지, 아니면 대상 객체 하이라이트
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
			// 잡을 수 있는 개체를 찾을 수 없으면 중단
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
		// 객체를 잡기 쉽도록 잡기 범위 확대
		PeformingGravityGrab = true;
		CharacterRef->GrabSphere->SetSphereRadius(GrabSphereRadiusGravityGrab);

		if (IsValid(GravityGrabActor))
		{
			// 대상 객체가 유효한 경우 객체 발사
			FVector LaunchVelocity = TraceGravityGrabPath(GravityGrabActor, CharacterRef->HandMesh->GetSocketLocation(TraceStartSocketName));
			if (IsValid(GravityGrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass())))
			{
				GravityGrabPrimitive = Cast<UPrimitiveComponent>(GravityGrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
				GravityGrabPrimitive->SetAllPhysicsLinearVelocity(LaunchVelocity);
				GravityGrabPrimitive->SetLinearDamping(0.0f);
				FName BoneName = TEXT("None");
				// 날라오면서 다른 객체를 밀어내도록 질량을 높혔다가 원래대로 돌림
				GravityGrabPrimitive->SetMassScale(BoneName, 10000.0f);

				FTimerHandle DelayHandle;
				float DelayTime = 0.2f;

				// Delay
				GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([&]()
					{
						if(IsValid(GravityGrabPrimitive))
							GravityGrabPrimitive->SetMassScale(BoneName, 1.0f);
						// TimerHandle 초기화
						GetWorld()->GetTimerManager().ClearTimer(DelayHandle);
					}), DelayTime, false);	// 반복하려면 false를 true로 변경
			}
		}
	}

}

FVector AGravityGrabModule::TraceGravityGrabPath(AActor* GrabbedActor, FVector HandLocation)
{
	if (IsValid(GrabbedActor))
	{
		// 컨트롤러와 객체 간의 거리를 통해 궤적의 높이를 결정, 가까우면 더 높은 Arc를 가짐
		FVector StartPos = GrabbedActor->GetActorLocation();
		float Distance = UKismetMathLibrary::Vector_Distance(StartPos, HandLocation);
		float ArcParam = UKismetMathLibrary::Lerp(0.4f, 0.9f, UKismetMathLibrary::FClamp(Distance / GravityGrabRange, 0.0f, 1.0f));

		// 발사 벡터 계산
		FVector OutLaunchVelocity;
		UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), OutLaunchVelocity, StartPos, HandLocation, 0.0f, ArcParam);

		return OutLaunchVelocity;
	}
	else
	{
		// 대상 액터가 유효하지 않은 경우
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
		// 잡을 수 있는 개체를 찾을 수 없으면 중단
		AbortGravityGrab();
	}
}

