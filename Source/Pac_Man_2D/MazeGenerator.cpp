#include "MazeGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Camera/CameraComponent.h"

// =========================================================
// 1. MAP DATA (36x28)
// =========================================================
static const int32 NumRows = 36;
static const int32 NumCols = 28;    

// 1 = Wall, 0 = Path
static const int32 GlobalMapLayout[NumRows][NumCols] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Row 0 (Padding)
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Row 1 (Padding)
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Row 2 (Padding)
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Row 3
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 4
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 5
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 6
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 7
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 8
    {1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1}, // Row 9
    {1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1}, // Row 10
    {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1}, // Row 11
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1}, // Row 12
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1}, // Row 13
    {1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1}, // Row 14
    {1,1,1,1,1,1,0,1,1,0,1,1,1,0,0,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 15 (Ghost House Top)
    {1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1,1,1}, // Row 16
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0}, // Row 17 (Tunnel)
    {1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1,1,1}, // Row 18
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 19 (Ghost House Bottom)
    {1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1}, // Row 20
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 21
    {1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1}, // Row 22
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 23
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 24
    {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1}, // Row 25
    {1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1}, // Row 26
    {1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1}, // Row 27
    {1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1}, // Row 28
    {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1}, // Row 29
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1}, // Row 30
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1}, // Row 31
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // Row 32
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // Row 33
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Row 34 (Padding)
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}  // Row 35 (Padding)
};

// =========================================================
// 2. ACTOR IMPLEMENTATION
// =========================================================

AMazeGenerator::AMazeGenerator()
{
    PrimaryActorTick.bCanEverTick = false; 
    WallMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallMesh"));
    RootComponent = WallMesh;
    
    // 1. Create the Camera
    MazeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MazeCamera"));
    MazeCamera->SetupAttachment(RootComponent);

    // 2. Set Default Rotation (Pitch -90 look down, Yaw 180 to flip world so Row 0 is Top)
    MazeCamera->SetRelativeRotation(FRotator(-90.0f, 180.0f, 0.0f));
}

void AMazeGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    GenerateMaze();
}

void AMazeGenerator::BeginPlay()
{
    Super::BeginPlay();
    // Ensure maze is generated at runtime too
    GenerateMaze();
}

void AMazeGenerator::GenerateMaze()
{
    if (WallMaterial)
    {
        WallMesh->SetMaterial(0, WallMaterial);
    }
    
    WallMesh->ClearInstances();
    
    // --- CAMERA LOGIC ---
    // FIX 1: Check bAutoCameraSetup. If FALSE, we skip this and let you move it manually.
    if (MazeCamera && bAutoCameraSetup) 
    {
        float MapHeight = NumRows * TileSize; 
        float MapWidth = NumCols * TileSize;  

        float CenterX = MapHeight * 0.5f;
        float CenterY = MapWidth * 0.5f;

        // FIX 2: Force Orthographic Mode (Just in case)
        MazeCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);

        // CHECK: Did the user provide a manual width?
        if (CameraOrthoWidth > 0.0f)
        {
            // Yes! Use the user's custom value
            MazeCamera->OrthoWidth = CameraOrthoWidth;
        }
        else
        {
            // No (Value is 0), so Auto-Calculate it
            float TargetOrthoWidth = FMath::Max(MapHeight, MapWidth) * 1.1f;
            MazeCamera->OrthoWidth = TargetOrthoWidth;
        }

        // Handle Location (Center + Offset)
        FVector TargetPos = FVector(CenterX, CenterY, 1000.0f) + CameraOffset;
        MazeCamera->SetRelativeLocation(TargetPos);
    }

    // Generate Walls
    for (int32 Row = 0; Row < NumRows; Row++)
    {
        for (int32 Col = 0; Col < NumCols; Col++)
        {
            if (GlobalMapLayout[Row][Col] == 1)
            {
                float X = Row * TileSize;
                float Y = Col * TileSize;
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