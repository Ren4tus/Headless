
#include "VrPlayerMotionControllerPawn.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
//#include "G_GameControl.h"

AVrPlayerMotionControllerPawn::AVrPlayerMotionControllerPawn()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	DistortionSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Distortion Sphere"));
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Detection Sphere"));
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Post Process"));
}

void AVrPlayerMotionControllerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVrPlayerMotionControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVrPlayerMotionControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

