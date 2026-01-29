#include "GhostPawn.h"
#include "MazeGenerator.h"
#include "PacmanPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

AGhostPawn::AGhostPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostMesh"));
    RootComponent = GhostMesh;

    // Use a Cylinder for now (so it looks different from Pacman)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylMesh.Succeeded())
    {
        GhostMesh->SetStaticMesh(CylMesh.Object);
        GhostMesh->SetWorldScale3D(FVector(0.4f, 0.4f, 0.4f)); 
    }
}

void AGhostPawn::BeginPlay()
{
    Super::BeginPlay();

    // 1. Find Maze 
    AActor* FoundMaze = UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGenerator::StaticClass());
    GameMaze = Cast<AMazeGenerator>(FoundMaze);

    // 2. Try to find Player
    AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
    PlayerPawn = Cast<APacmanPawn>(FoundPlayer);

    if (GameMaze)
    {
        // --- 3. POSITIONING LOGIC ---
        // Instead of using the Editor location, we FORCE the spawn point based on Ghost Type
        int32 StartRow = 14;
        int32 StartCol = 14;

        switch (GhostType)
        {
        case EGhostType::Red:    // Blinky
            StartRow = 13; 
            StartCol = 14; 
            break;

        case EGhostType::Pink:   // Pinky
            StartRow = 13; 
            StartCol = 13; 
            break;

        case EGhostType::Blue:   // Cyan (Inky)
            StartRow = 14; 
            StartCol = 14; 
            break;

        case EGhostType::Orange: // Clyde
            StartRow = 14; 
            StartCol = 13; 
            break;
        }

        // Snap to the calculated Grid Position
        FVector CenterPos = GameMaze->GetLocationFromGrid(StartRow, StartCol);
        SetActorLocation(CenterPos);

        // --- 4. KICKSTART MOVEMENT ---
        // Make a decision IMMEDIATELY so they don't stand still
        MakeDecisionAtIntersection(StartRow, StartCol);
        
        // Safety Fallback: If trapped, force a safe direction (Up usually works in ghost house)
        if (CurrentDir == EGhostDirection::None)
        {
            CurrentDir = EGhostDirection::Up;
        }
    }
}

void AGhostPawn::Tick(float DeltaTime)
{
    if (!GameMaze) return;

    // Lazy Load Player if missing
    if (!PlayerPawn)
    {
        AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
        if (FoundPlayer) PlayerPawn = Cast<APacmanPawn>(FoundPlayer);
    }

    // --- 1. MOVEMENT PHYSICS ---
    FVector MyPos = GetActorLocation();
    
    int32 CurrentRow = FMath::FloorToInt(MyPos.X / GameMaze->TileSize);
    int32 CurrentCol = FMath::FloorToInt(MyPos.Y / GameMaze->TileSize);
    
    FVector TileCenter = GameMaze->GetLocationFromGrid(CurrentRow, CurrentCol);
    float DistToCenter = FVector::Dist2D(MyPos, TileCenter);

    // --- 2. AI DECISION MAKING ---
    // Check if we are close to the center of a tile
    if (DistToCenter < 5.0f)
    {
        MakeDecisionAtIntersection(CurrentRow, CurrentCol);
        
        // Snap to center to prevent drift
        FVector NewPos = GetActorLocation();
        NewPos.X = TileCenter.X;
        NewPos.Y = TileCenter.Y;
        SetActorLocation(NewPos);
    }

    // --- 3. APPLY MOVEMENT ---
    if (CurrentDir != EGhostDirection::None)
    {
        FVector MoveVec = GetVectorFromEnum(CurrentDir);
        AddActorWorldOffset(MoveVec * MovementSpeed * DeltaTime, true); // Sweep=true for safety
    }
}

FVector2D AGhostPawn::GetTargetTile()
{
    // Default safe target (Ghost House) if player is missing
    if (!PlayerPawn) return FVector2D(14, 14);

    FVector PlayerPos = PlayerPawn->GetActorLocation();
    int32 PRow = FMath::FloorToInt(PlayerPos.X / GameMaze->TileSize);
    int32 PCol = FMath::FloorToInt(PlayerPos.Y / GameMaze->TileSize);

    // AI PERSONALITY
    switch (GhostType)
    {
        case EGhostType::Red:
            // Blinky: Chases Player directly
            return FVector2D(PRow, PCol);

        case EGhostType::Pink:
            // Pinky: Ambush (Target 4 tiles ahead - Placeholder for now)
            return FVector2D(PRow, PCol); 

        case EGhostType::Blue:
            // Cyan: Flanking (Placeholder)
            return FVector2D(PRow, PCol);

        case EGhostType::Orange:
            // Clyde: Random/Scatter (Placeholder)
            return FVector2D(1, 1); // Go to corner

        default:
            return FVector2D(PRow, PCol);
    }
}

void AGhostPawn::MakeDecisionAtIntersection(int32 CurrentRow, int32 CurrentCol)
{
    FVector2D Target = GetTargetTile();

    EGhostDirection BestDir = EGhostDirection::None;
    float ShortestDist = 9999999.0f;

    EGhostDirection PossibleDirs[] = { EGhostDirection::Up, EGhostDirection::Down, EGhostDirection::Left, EGhostDirection::Right };

    for (EGhostDirection TestDir : PossibleDirs)
    {
        // Rule: Ghosts cannot reverse direction (180 turn) unless dead end
        if (TestDir == GetOppositeDir(CurrentDir)) continue;

        FVector DirVec = GetVectorFromEnum(TestDir);
        int32 NextRow = CurrentRow + (int32)DirVec.X;
        int32 NextCol = CurrentCol + (int32)DirVec.Y;

        // Wall Check
        if (GameMaze->IsWall(NextRow, NextCol)) continue;

        // Distance Check
        float Dist = FVector2D::Distance(FVector2D(NextRow, NextCol), Target);

        if (Dist < ShortestDist)
        {
            ShortestDist = Dist;
            BestDir = TestDir;
        }
    }

    if (BestDir != EGhostDirection::None)
    {
        CurrentDir = BestDir;
    }
    else
    {
        // Dead End Logic: Only option is to reverse
        CurrentDir = GetOppositeDir(CurrentDir);
    }
}

EGhostDirection AGhostPawn::GetOppositeDir(EGhostDirection Dir)
{
    switch(Dir) {
        case EGhostDirection::Up: return EGhostDirection::Down;
        case EGhostDirection::Down: return EGhostDirection::Up;
        case EGhostDirection::Left: return EGhostDirection::Right;
        case EGhostDirection::Right: return EGhostDirection::Left;
        default: return EGhostDirection::None;
    }
}

FVector AGhostPawn::GetVectorFromEnum(EGhostDirection Dir)
{
    switch (Dir)
    {
    case EGhostDirection::Up:    return FVector(-1, 0, 0);
    case EGhostDirection::Down:  return FVector(1, 0, 0);
    case EGhostDirection::Right: return FVector(0, -1, 0);
    case EGhostDirection::Left:  return FVector(0, 1, 0);
    default: return FVector::ZeroVector;
    }
}