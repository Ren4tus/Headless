// Fill out your copyright notice in the Description page of Project Settings.


#include "FutureWatch.h"

AFutureWatch::AFutureWatch()
{
	GetStaticMeshComponent()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	RoomScaleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomScaleMesh"));
	Laser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Laser"));
	SkillUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("SkillUI"));
	HP = CreateDefaultSubobject<UWidgetComponent>(TEXT("HP"));

	RoomScaleMesh->AttachToComponent(GetStaticMeshComponent(), FAttachmentTransformRules::KeepWorldTransform);
	Laser->AttachToComponent(GetStaticMeshComponent(), FAttachmentTransformRules::KeepWorldTransform);
	SkillUI->AttachToComponent(GetStaticMeshComponent(), FAttachmentTransformRules::KeepWorldTransform);
	HP->AttachToComponent(GetStaticMeshComponent(), FAttachmentTransformRules::KeepWorldTransform);
}
