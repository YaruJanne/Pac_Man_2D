#include "GhostPawn.h"
#include "MazeGenerator.h"
#include "PacmanPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h" 
#include "UObject/ConstructorHelpers.h"

// Sets default values
AGhostPawn::AGhostPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostMesh"));
    RootComponent = GhostMesh;

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

    AActor* FoundMaze = UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGenerator::StaticClass());
    GameMaze = Cast<AMazeGenerator>(FoundMaze);

    AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
    PlayerPawn = Cast<APacmanPawn>(FoundPlayer);

    if (GameMaze)
    {
        // Default Start Positions
        int32 StartRow = 16; 
        int32 StartCol = 14;

        switch (GhostType)
        {
            case EGhostType::Red:    
                StartRow = 14; StartCol = 14; // Start slightly higher
                State = EGhostState::LeavingHouse; // Red leaves IMMEDIATELY
                CurrentDir = EGhostDirection::Left; // Give him a nudge
                break;

            case EGhostType::Pink:   
                StartRow = 16; StartCol = 14; // Center
                State = EGhostState::Idle;    
                break;

            case EGhostType::Blue:   
                StartRow = 16; StartCol = 12; // Left side
                State = EGhostState::Idle;    
                break;

            case EGhostType::Orange: 
                StartRow = 16; StartCol = 16; // Right side
                State = EGhostState::Idle;    
                break;
        }

        // Snap to grid
        FVector CenterPos = GameMaze->GetLocationFromGrid(StartRow, StartCol);
        SetActorLocation(CenterPos);
        
    }
}

void AGhostPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // Always good practice

    if (!GameMaze) return;

    // Lazy load player if we missed it in BeginPlay
    if (!PlayerPawn)
    {
        AActor* FoundPlayer = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
        if (FoundPlayer) PlayerPawn = Cast<APacmanPawn>(FoundPlayer);
    }

    // --- STATE 1: IDLE (Do nothing) ---
    if (State == EGhostState::Idle) return; 
    

    // --- STATE 2: LEAVING HOUSE (No Collision Physics) ---
    if (State == EGhostState::LeavingHouse)
    {
        // Target the tile just outside the door (Row 11, Col 14)
        FVector ExitPos = GameMaze->GetLocationFromGrid(14, 12); 
        FVector MyPos = GetActorLocation();

        // Move directly towards exit (ignoring walls)
        FVector Direction = (ExitPos - MyPos).GetSafeNormal();
        
        // If we are extremely close, snap and switch state
        if (FVector::Dist(MyPos, ExitPos) < 5.0f)
        {
            SetActorLocation(ExitPos);
            State = EGhostState::Scatter;     // Start Chasing
            CurrentDir = EGhostDirection::Left; // Pick an initial direction
        }
        else
        {
            AddActorWorldOffset(Direction * (MovementSpeed * 0.5f) * DeltaTime);
        }
        return; 
    }

    // --- STATE 3 & 4: ACTIVE (CHASE) OR SCATTER ---
    // The physics are the same, only the Target Tile changes!
    if (State == EGhostState::Active || State == EGhostState::Scatter)
    {
        FVector MyPos = GetActorLocation();
        
        int32 CurrentRow = FMath::FloorToInt(MyPos.X / GameMaze->TileSize);
        int32 CurrentCol = FMath::FloorToInt(MyPos.Y / GameMaze->TileSize);
        
        FVector TileCenter = GameMaze->GetLocationFromGrid(CurrentRow, CurrentCol);
        float DistToCenter = FVector::Dist2D(MyPos, TileCenter);

        // Decision Logic
        if (DistToCenter < 5.0f)
        {
            MakeDecisionAtIntersection(CurrentRow, CurrentCol);
            
            // Anti-Drift
            FVector NewPos = GetActorLocation();
            if (CurrentDir == EGhostDirection::Up || CurrentDir == EGhostDirection::Down)
                NewPos.Y = FMath::FInterpTo(NewPos.Y, TileCenter.Y, DeltaTime, 15.0f);
            else
                NewPos.X = FMath::FInterpTo(NewPos.X, TileCenter.X, DeltaTime, 15.0f);
                
            SetActorLocation(NewPos);
        }

        // Apply Movement
        if (CurrentDir != EGhostDirection::None)
        {
            FVector MoveVec = GetVectorFromEnum(CurrentDir);
            AddActorWorldOffset(MoveVec * MovementSpeed * DeltaTime, true);
        }
    }
}

// --- MISSING HELPER FUNCTIONS START HERE ---

void AGhostPawn::MakeDecisionAtIntersection(int32 CurrentRow, int32 CurrentCol)
{
    FVector2D Target = GetTargetTile();

    EGhostDirection BestDir = EGhostDirection::None;
    float ShortestDist = 9999999.0f;

    // Check all 4 directions
    EGhostDirection PossibleDirs[] = { EGhostDirection::Up, EGhostDirection::Down, EGhostDirection::Left, EGhostDirection::Right };

    for (EGhostDirection TestDir : PossibleDirs)
    {
        // 1. Don't reverse (U-Turn) unless stuck
        if (TestDir == GetOppositeDir(CurrentDir)) continue;

        // 2. Calculate next grid position
        FVector DirVec = GetVectorFromEnum(TestDir);
        int32 NextRow = CurrentRow + (int32)DirVec.X;
        int32 NextCol = CurrentCol + (int32)DirVec.Y;

        // 3. Is it a wall?
        if (GameMaze->IsWall(NextRow, NextCol)) continue;

        // 4. Is it closer to target?
        float Dist = FVector2D::Distance(FVector2D(NextRow, NextCol), Target);

        if (Dist < ShortestDist)
        {
            ShortestDist = Dist;
            BestDir = TestDir;
        }
    }

    // If we found a valid path, take it
    if (BestDir != EGhostDirection::None)
    {
        CurrentDir = BestDir;
    }
    else
    {
        // Dead end! We are forced to reverse.
        CurrentDir = GetOppositeDir(CurrentDir);
    }
}

FVector2D AGhostPawn::GetTargetTile()
{
    // 1. SCATTER MODE (Fixed Corners)
    if (State == EGhostState::Scatter)
    {
        switch (GhostType)
        {
        case EGhostType::Red:    
            // Top Right (Row 0, Col 26) - adjusted for new map size
            return FVector2D(0, 26); 

        case EGhostType::Pink:   
            // Top Left (Row 0, Col 3)
            return FVector2D(0, 3); 

        case EGhostType::Orange: 
            // Bottom Left (Row 36, Col 1) - forces looping at corner
            return FVector2D(36, 1); 

        case EGhostType::Blue:   
            // Bottom Right (Row 36, Col 28)
            return FVector2D(36, 28);
        }
    }

    // 2. ACTIVE / CHASE MODE (Target Player)
    if (PlayerPawn)
    {
        FVector PlayerPos = PlayerPawn->GetActorLocation();
        if (GameMaze->TileSize > 0)
        {
            int32 PRow = FMath::FloorToInt(PlayerPos.X / GameMaze->TileSize);
            int32 PCol = FMath::FloorToInt(PlayerPos.Y / GameMaze->TileSize);
            return FVector2D(PRow, PCol);
        }
    }

    return FVector2D(14, 14);
}

EGhostDirection AGhostPawn::GetOppositeDir(EGhostDirection Dir)
{
    switch(Dir) {
        case EGhostDirection::Up:    return EGhostDirection::Down;
        case EGhostDirection::Down:  return EGhostDirection::Up;
        case EGhostDirection::Left:  return EGhostDirection::Right;
        case EGhostDirection::Right: return EGhostDirection::Left;
        default: return EGhostDirection::None;
    }
}

FVector AGhostPawn::GetVectorFromEnum(EGhostDirection Dir)
{
    // IMPORTANT: Matches Pacman's inverted controls
    switch (Dir)
    {
        case EGhostDirection::Up:    return FVector(-1, 0, 0);
        case EGhostDirection::Down:  return FVector(1, 0, 0);
        case EGhostDirection::Right: return FVector(0, -1, 0);
        case EGhostDirection::Left:  return FVector(0, 1, 0);
        default: return FVector::ZeroVector;
    }
}