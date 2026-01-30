#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeGenerator.generated.h"

UCLASS()
class PAC_MAN_2D_API AMazeGenerator : public AActor
{
	GENERATED_BODY()

public:	
	AMazeGenerator();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override; // <--- Added BeginPlay back for setup

public:	
	// The visual representation of our walls
	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	class UInstancedStaticMeshComponent* WallMesh;
	
	// --- NEW CAMERA ---
	UPROPERTY(VisibleAnywhere, Category = "Maze")
	class UCameraComponent* MazeCamera;
	// Dimensions
	UPROPERTY(EditAnywhere, Category = "Maze")
	int32 TileSize = 50; // 100 unreal units per tile
	
	// This creates a dedicated slot to pick your color/material
	UPROPERTY(EditAnywhere, Category = "Maze")
	UMaterialInterface* WallMaterial;
	
	// If TRUE, the code will auto-center and resize the camera.
	// If FALSE, you can manually move/rotate the camera in the Viewport.
	UPROPERTY(EditAnywhere, Category = "Maze Camera")
	bool bAutoCameraSetup = true;

	// Use this to nudge the camera while keeping Auto Setup ON.
	UPROPERTY(EditAnywhere, Category = "Maze Camera", meta=(EditCondition="bAutoCameraSetup"))
	FVector CameraOffset = FVector::ZeroVector;
	
	// Leave at 0 to let the game calculate it automatically.
	UPROPERTY(EditAnywhere, Category = "Maze Camera")
	float CameraOrthoWidth = 0.0f;
	
	// Helper to check walls from other classes
	bool IsWall(int32 Row, int32 Col) const;
    
	// Allow other classes to ask for world coordinates
	FVector GetLocationFromGrid(int32 Row, int32 Col) const;
	// The Grid Data (1 = Wall, 0 = Pellet/Path, 2 = Empty/No Pellet)
	// In a real project, you might load this from a file. 
	// For this test, a hardcoded TArray or 2D array is fine.
	void GenerateMaze();
};

