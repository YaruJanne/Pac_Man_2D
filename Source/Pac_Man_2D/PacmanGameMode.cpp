#include "PacmanGameMode.h"
#include "PacmanPawn.h" // Don't forget to include your Pawn's header!
#include "Kismet/GameplayStatics.h" 
#include "Camera/CameraActor.h"

APacmanGameMode::APacmanGameMode()
{
	// Set the default pawn class to our custom Pacman Pawn
	DefaultPawnClass = APacmanPawn::StaticClass();
}

void APacmanGameMode::StartPlay()
{
    Super::StartPlay();

    // 1. Find the CameraActor in the world (assuming you placed one manually)
    AActor* MyCamera = UGameplayStatics::GetActorOfClass(this, ACameraActor::StaticClass());

    // 2. Get the Player Controller
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    // 3. Set the view
    if (PC && MyCamera)
    {
        PC->SetViewTargetWithBlend(MyCamera);
    }
}