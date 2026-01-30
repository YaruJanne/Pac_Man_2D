#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

// Define directions (Enum)
UENUM(BlueprintType)
enum class EPadDirection : uint8
{ 
	None, 
	Up, 
	Down, 
	Left, 
	Right 
};

UCLASS()
class PAC_MAN_2D_API APacmanPawn : public APawn
{
	GENERATED_BODY()

public:
	APacmanPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Config ---
	UPROPERTY(EditAnywhere, Category="Pacman")
	float MovementSpeed = 350.0f; 

	// --- State ---
	// The direction we are currently moving
	EPadDirection CurrentDir = EPadDirection::None;
    
	// The direction the player WANTS to move (buffered)
	EPadDirection NextDir = EPadDirection::None; 

	// Current logical grid coordinates
	FVector2D CurrentGridCoords; 
    
	// Pointer to the Map (to check walls)
	// We use "class" here to forward declare it
	UPROPERTY(VisibleAnywhere, Category = "Refs")
	class AMazeGenerator* GameMaze;

	// --- Functions ---
	// Helper to convert Enum to Vector (e.g., Up -> (1,0,0))
	FVector GetVectorFromEnum(EPadDirection Dir);
    
	// Input Handlers
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
};