// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "VisionActorComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRWEAPONSKIT_API UVisionActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVisionActorComponent();

protected:
	virtual void BeginPlay() override;
	void FadeIn();
	void FadeInLoop();
	void HighlightOn(AActor* actor);
	UFUNCTION(BlueprintImplementableEvent)
	void SetUpMaterial();
	void InitialPulse();
	void Reset();
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	FTimerHandle FadeOutTimer;
	FTimerHandle FadeInTimer;
	FTimerHandle BlowUpTimer;
	FTimerHandle MarkerFadeOutTimer;
	float VisionFadeTime = 0.5f;
	float InitialPulseTime = 0.66f;
	bool UseInitalPulse = true;
	bool FadeInStarted = false;
	int PulseCounter = 0;

	UMaterialParameterCollection* EffectIntensity;
	APawn* CharacterRef;
	TArray<AActor*> ActorsInRange;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		UStaticMeshComponent* DistortionSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		USphereComponent* DetectionSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vision")
		UPostProcessComponent* PostProcessComp;
};
