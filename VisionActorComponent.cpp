// Fill out your copyright notice in the Description page of Project Settings.


#include "VisionActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <VRWeaponsKit/VrPlayerMotionControllerPawn.h>
#include "Components/PrimitiveComponent.h"
UVisionActorComponent::UVisionActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UVisionActorComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UVisionActorComponent::FadeIn()
{
	// Reset fade out timer
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(GetWorld(), FadeOutTimer);

	// Start fade in loop & Store a reference to the fade in timer
	FadeInTimer = UKismetSystemLibrary::K2_SetTimer(
		this, 
		FString("FadeInLoop"), 
		VisionFadeTime / 50.0f, 
		true
	);

	// Is initial pulse enabled
	if (UseInitalPulse)
	{
		// Call the initial pulse function
		InitialPulse();
	}
}

void UVisionActorComponent::FadeInLoop()
{
	if (FadeInStarted)
	{
		SetUpMaterial();
	}
	else 
	{
		FadeInStarted = true;
		UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(GetWorld(), MarkerFadeOutTimer);

		TArray<AActor*> Result;
		DetectionSphere->GetOverlappingActors(Result);
		for (AActor* Actor : Result)
		{
			HighlightOn(Actor);
		}
	}
}

void UVisionActorComponent::HighlightOn(AActor* actor)
{
	// Is this actor the player?
	if (CharacterRef == actor)
	{
		return;
	}
	else
	{
		// Un-highlight all attached static meshes.
		TArray<UActorComponent*> StaticMeshes;
		StaticMeshes = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (UActorComponent* StaticMesh : StaticMeshes)
		{
			Cast<UStaticMeshComponent>(StaticMesh)->SetRenderCustomDepth(true);
		}

		//Un-highlight all attached skeletal meshes.
		TArray<UActorComponent*> SkeletalMeshes;
		SkeletalMeshes = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (UActorComponent* SkeletalMesh : SkeletalMeshes)
		{
			Cast<UStaticMeshComponent>(SkeletalMesh)->SetRenderCustomDepth(true);
		}

		ActorsInRange.Add(actor);
	}
}

void UVisionActorComponent::InitialPulse()
{
	if (!IsValid(DistortionSphere))
	{
		Reset();
	}
	// Set distortion sphere to invisible
	DistortionSphere->SetVisibility(true, false);
	// Reset pulse counter
	PulseCounter = 0;

	// Start BlowUpPulse loop
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(GetWorld(), FadeOutTimer);
	BlowUpTimer = FadeInTimer = UKismetSystemLibrary::K2_SetTimer(
		this,
		FString("BlowUpPulse"),
		InitialPulseTime / 30.0f,
		true
	);
}

void UVisionActorComponent::Reset()
{
	AVrPlayerMotionControllerPawn* player = Cast<AVrPlayerMotionControllerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
	DistortionSphere = player->DistortionSphere;
	DetectionSphere = player->DetectionSphere;
	PostProcessComp = player->PostProcessComp;
}


void UVisionActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

