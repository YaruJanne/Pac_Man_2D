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
	virtual void BeginPlay() override;

public:	
	// The visual representation of our walls
	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	class UInstancedStaticMeshComponent* WallMesh;

	// Dimensions
	UPROPERTY(EditAnywhere, Category = "Maze")
	int32 TileSize = 100; // 100 unreal units per tile

	// The Grid Data (1 = Wall, 0 = Pellet/Path, 2 = Empty/No Pellet)
	// In a real project, you might load this from a file. 
	// For this test, a hardcoded TArray or 2D array is fine.
	void GenerateMaze();
};