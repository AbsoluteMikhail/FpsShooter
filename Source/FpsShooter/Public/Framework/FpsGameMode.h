// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FpsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSSHOOTER_API AFpsGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	void BeginPlay() override;
};


