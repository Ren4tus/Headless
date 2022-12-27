// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPickUpOutline.generated.h"

UINTERFACE(MinimalAPI)
class UIPickUpOutline : public UInterface
{
	GENERATED_BODY()
};

class VRWEAPONSKIT_API IIPickUpOutline
{
	GENERATED_BODY()

public:
	virtual void ToggleOutline(AActor* GrabActor, bool ShowOutline);
};
