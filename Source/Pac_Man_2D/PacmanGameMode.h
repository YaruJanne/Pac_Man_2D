#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PacmanGameMode.generated.h"

UCLASS()
class PAC_MAN_2D_API APacmanGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APacmanGameMode();
	
protected:
	virtual void StartPlay() override;
};