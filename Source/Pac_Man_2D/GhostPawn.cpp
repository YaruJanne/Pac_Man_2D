#include "GhostPawn.h"
#include "MazeGenerator.h"
#include "PacmanPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

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

    // 1. Find Maze (Critical)
    AActor* FoundMaze = UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGenerator::StaticClass());
    GameMaze = Cast<AMazeGenerator>(FoundMaze);

    // 2. Try to find Player (Might fail if he hasn't spawned yet)
    AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
    PlayerPawn = Cast<APacmanPawn>(FoundPlayer);

    // 3. KICKSTART: Snap to grid and force him to move immediately!
    // If we don't do this, he starts with Direction::None and never moves.
    if (GameMaze)
    {
        // A. Snap to the exact center of the spawn tile
        FVector MyPos = GetActorLocation();
        int32 StartRow = FMath::FloorToInt(MyPos.X / GameMaze->TileSize);
        int32 StartCol = FMath::FloorToInt(MyPos.Y / GameMaze->TileSize);
        
        FVector CenterPos = GameMaze->GetLocationFromGrid(StartRow, StartCol);
        SetActorLocation(FVector(CenterPos.X, CenterPos.Y, MyPos.Z));

        // B. Make a decision IMMEDIATELY (Don't just force Left!)
        // This ensures we check walls before taking our first step.
        MakeDecisionAtIntersection(StartRow, StartCol);
        
        // C. Fallback: If trapped or decision failed, try a safe defaults
        if (CurrentDir == EGhostDirection::None)
        {
            // Try Up, then Left, etc., checking for walls manually
            if (!GameMaze->IsWall(StartRow + 1, StartCol)) CurrentDir = EGhostDirection::Up;
            else if (!GameMaze->IsWall(StartRow, StartCol - 1)) CurrentDir = EGhostDirection::Left;
            // Add more else-ifs if needed, or spawn him in a safer spot!
        }
    }
}

void AGhostPawn::Tick(float DeltaTime)
{
        
    if (!GameMaze) 
    {
        // Try to find it again
        AActor* FoundMaze = UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGenerator::StaticClass());
        GameMaze = Cast<AMazeGenerator>(FoundMaze);
        return; // Skip this frame
    }
    
    Super::Tick(DeltaTime);
    if (!GameMaze) return;

    // --- 0. LAZY LOADING PLAYER ---
    // If we missed the player in BeginPlay, try to find him now
    if (!PlayerPawn)
    {
        AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
        if (FoundPlayer)
        {
            PlayerPawn = Cast<APacmanPawn>(FoundPlayer);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Ghost Found Pacman!"));
        }
    }

    // --- 1. MOVEMENT PHYSICS ---
    FVector MyPos = GetActorLocation();
    
    // Calculate which tile we are currently "in"
    int32 CurrentRow = FMath::FloorToInt(MyPos.X / GameMaze->TileSize);
    int32 CurrentCol = FMath::FloorToInt(MyPos.Y / GameMaze->TileSize);
    
    // Calculate center of that tile
    FVector TileCenter = GameMaze->GetLocationFromGrid(CurrentRow, CurrentCol);
    float DistToCenter = FVector::Dist2D(MyPos, TileCenter);

    // --- 2. AI DECISION MAKING ---
    // Tolerance: Needs to be small (e.g. 5.0f) but not impossible to hit
    if (DistToCenter < 5.0f)
    {
        // Only make a decision if we are moving towards the center
        // (Prevents jittering back and forth)
        MakeDecisionAtIntersection(CurrentRow, CurrentCol);
        
        // Snap to center to keep lines clean
        FVector NewPos = GetActorLocation();
        NewPos.X = TileCenter.X;
        NewPos.Y = TileCenter.Y;
        SetActorLocation(NewPos);
    }

    // --- 3. APPLY MOVEMENT ---
    if (CurrentDir != EGhostDirection::None)
    {
        FVector MoveVec = GetVectorFromEnum(CurrentDir);
        AddActorWorldOffset(MoveVec * MovementSpeed * DeltaTime);
    }
}

FVector2D AGhostPawn::GetTargetTile()
{
    // If player is dead or missing, just go to 0,0
    if (!PlayerPawn) return FVector2D(1, 1);

    FVector PlayerPos = PlayerPawn->GetActorLocation();
    int32 PRow = FMath::FloorToInt(PlayerPos.X / GameMaze->TileSize);
    int32 PCol = FMath::FloorToInt(PlayerPos.Y / GameMaze->TileSize);

    // --- LOGIC SWITCHER ---
    switch (GhostType)
    {
        case EGhostType::Red:
            // Blinky: Target is exactly the player
            return FVector2D(PRow, PCol);

        case EGhostType::Pink:
            // Pinky: Target is 4 tiles "in front" of player (Task for later)
            return FVector2D(PRow, PCol); // Placeholder

        default:
            return FVector2D(PRow, PCol);
    }
}

void AGhostPawn::MakeDecisionAtIntersection(int32 CurrentRow, int32 CurrentCol)
{
    // 1. Identify the Target
    FVector2D Target = GetTargetTile();

    // 2. Check all 4 neighbors
    // We cannot turn back 180 degrees unless stuck
    EGhostDirection BestDir = EGhostDirection::None;
    float ShortestDist = 9999999.0f;

    EGhostDirection PossibleDirs[] = { EGhostDirection::Up, EGhostDirection::Down, EGhostDirection::Left, EGhostDirection::Right };

    for (EGhostDirection TestDir : PossibleDirs)
    {
        // Don't reverse! (e.g. if going Left, don't check Right)
        if (TestDir == GetOppositeDir(CurrentDir)) continue;

        // Get Grid ID of neighbor
        FVector DirVec = GetVectorFromEnum(TestDir);
        int32 NextRow = CurrentRow + (int32)DirVec.X;
        int32 NextCol = CurrentCol + (int32)DirVec.Y;

        // Is it a wall?
        if (GameMaze->IsWall(NextRow, NextCol)) continue;

        // It's a valid path! Calculate distance to Target
        // Standard Euclidean distance (Hypotenuse)
        float Dist = FVector2D::Distance(FVector2D(NextRow, NextCol), Target);

        // Is this the shortest path so far?
        if (Dist < ShortestDist)
        {
            ShortestDist = Dist;
            BestDir = TestDir;
        }
    }

    // 3. Apply the decision
    // If dead end (only option was reverse), we might end up with None. 
    // In that case, force reverse.
    if (BestDir != EGhostDirection::None)
    {
        CurrentDir = BestDir;
    }
    else
    {
        // Dead End! Turn around.
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
    // COPY THIS from your PacmanPawn.cpp to ensure directions match!
    switch (Dir)
    {
        case EGhostDirection::Up:    return FVector(1, 0, 0);
        case EGhostDirection::Down:  return FVector(-1, 0, 0);
        case EGhostDirection::Right: return FVector(0, 1, 0);
        case EGhostDirection::Left:  return FVector(0, -1, 0);
        default: return FVector::ZeroVector;
    }
}