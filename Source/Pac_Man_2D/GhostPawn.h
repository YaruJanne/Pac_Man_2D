#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GhostPawn.generated.h"

// 1. New State System
UENUM(BlueprintType)
enum class EGhostState : uint8
{
	Idle,           // Bouncing inside house
	LeavingHouse,   // Moving to the door (ignoring collision)
	Active,          // Chasing or Scattering
	Scatter         // Go to corners
};

UENUM(BlueprintType)
enum class EGhostType : uint8
{
	Red, Pink, Blue, Orange 
};

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

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* GhostMesh;

	UPROPERTY(EditAnywhere, Category = "Ghost AI")
	EGhostType GhostType = EGhostType::Red;

	UPROPERTY(EditAnywhere, Category = "Ghost AI")
	float MovementSpeed = 330.0f;

	// --- NEW STATE VARIABLE ---
	EGhostState State = EGhostState::Idle;

	EGhostDirection CurrentDir = EGhostDirection::None;
	
	UPROPERTY(VisibleAnywhere, Category = "Refs")
	class AMazeGenerator* GameMaze;
	UPROPERTY(VisibleAnywhere, Category = "Refs")
	class APacmanPawn* PlayerPawn;

	void MakeDecisionAtIntersection(int32 CurrentRow, int32 CurrentCol);
	FVector2D GetTargetTile();

	EGhostDirection GetOppositeDir(EGhostDirection Dir);
	FVector GetVectorFromEnum(EGhostDirection Dir);
};