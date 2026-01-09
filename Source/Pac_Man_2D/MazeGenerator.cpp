#include "MazeGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"

// =========================================================
// 1. MAP DATA (28x31)
// =========================================================
// We use 'static' so these variables don't conflict with other files
static const int32 NumRows = 31;
static const int32 NumCols = 28;

// 1 = Wall, 0 = Path
static const int32 GlobalMapLayout[NumRows][NumCols] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Row 0
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 1
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 2
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 3
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 4
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 5
    {1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1}, // Row 6
    {1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1}, // Row 7
    {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1}, // Row 8
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1}, // Row 9
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1}, // Row 10
    {1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1}, // Row 11
    {1,1,1,1,1,1,0,1,1,0,1,1,1,0,0,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 12 (Ghost House Top)
    {1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1,1,1}, // Row 13
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0}, // Row 14 (Tunnel)
    {1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1,1,1}, // Row 15
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 16 (Ghost House Bottom)
    {1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1}, // Row 17
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 18
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 19
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 20
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 21
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 22
    {1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1}, // Row 23
    {1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1}, // Row 24
    {1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1}, // Row 25
    {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1}, // Row 26
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1}, // Row 27
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1}, // Row 28
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 29
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}  // Row 30
};

// =========================================================
// 2. ACTOR IMPLEMENTATION
// =========================================================

AMazeGenerator::AMazeGenerator()
{
    PrimaryActorTick.bCanEverTick = false; 
    WallMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallMesh"));
    RootComponent = WallMesh;
}

void AMazeGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    GenerateMaze();
}

void AMazeGenerator::GenerateMaze()
{
    if (WallMaterial)
    {
        WallMesh->SetMaterial(0, WallMaterial);
    }
    
    WallMesh->ClearInstances();

    // Use the global constants NumRows (31) and NumCols (28)
    for (int32 Row = 0; Row < NumRows; Row++)
    {
       for (int32 Col = 0; Col < NumCols; Col++)
       {
          if (GlobalMapLayout[Row][Col] == 1)
          {
             // Calculate World Position using TileSize
             // Note: Swap Row/Col if your map comes out rotated 90 degrees
             float X = Row * TileSize;
             float Y = Col * TileSize;
             
             // Get Center Position (Add half tile size)
             // This is CRITICAL for Pacman movement logic
             float HalfTile = TileSize * 0.5f;
             FVector Location(X + HalfTile, Y + HalfTile, 0.0f);
             
             FRotator Rotation(0.0f, 0.0f, 0.0f);

             WallMesh->AddInstance(FTransform(Rotation, Location));
          }
       }
    }
}

bool AMazeGenerator::IsWall(int32 Row, int32 Col) const
{
    // Check Bounds
    if (Row < 0 || Row >= NumRows || Col < 0 || Col >= NumCols) {
       return true; 
    }
    // Check Global Array
    return GlobalMapLayout[Row][Col] == 1;
}

FVector AMazeGenerator::GetLocationFromGrid(int32 Row, int32 Col) const
{
    // Returns the exact world coordinates for a tile center
    float HalfTile = TileSize * 0.5f;
    return FVector(Row * TileSize + HalfTile, Col * TileSize + HalfTile, 0.0f);
}