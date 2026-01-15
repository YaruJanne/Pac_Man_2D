#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GhostPawn.generated.h"

// 1. Define the 4 Types of Ghosts
UENUM(BlueprintType)
enum class EGhostType : uint8
{
    Red,    // Blinky (Chases Player)
    Pink,   // Pinky (Ambushes)
    Blue,   // Inky (Complex)
    Orange  // Clyde (Cowardly)
};

// Reuse our direction Enum
UENUM(BlueprintType)
enum class EGhostDirection : uint8
{
    None, Up, Down, Left, Right
};

UCLASS()
class PAC_MAN_2D_API AGhostPawn : public APawn
{
    GENERATED_BODY()

public:
    AGhostPawn();

protected:
    virtual void BeginPlay() override;

public: 
    virtual void Tick(float DeltaTime) override;

    // --- VISUALS ---
    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* GhostMesh;

    // --- SETTINGS ---
    UPROPERTY(EditAnywhere, Category = "Ghost AI")
    EGhostType GhostType = EGhostType::Red;

    UPROPERTY(EditAnywhere, Category = "Ghost AI")
    float MovementSpeed = 330.0f; // Slightly slower than Pacman (usually)

    // --- STATE ---
    EGhostDirection CurrentDir = EGhostDirection::None;
    
    // Ghosts don't have "NextDir" input buffering like players.
    // Instead, they decide their move instantly at intersections.

    // References
    class AMazeGenerator* GameMaze;
    class APacmanPawn* PlayerPawn;

    // --- AI FUNCTIONS ---
    // The brain: Decides which way to turn at an intersection
    void MakeDecisionAtIntersection(int32 CurrentRow, int32 CurrentCol);
    
    // Returns the grid tile we WANT to reach
    FVector2D GetTargetTile();

    // Helper: Opposites (Ghosts generally can't reverse direction)
    EGhostDirection GetOppositeDir(EGhostDirection Dir);
    FVector GetVectorFromEnum(EGhostDirection Dir);
};