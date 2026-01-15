#include "PacmanPawn.h"
#include "MazeGenerator.h" 
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h" 

// Sets default values
APacmanPawn::APacmanPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. Create the Mesh Component
    UStaticMeshComponent* PacmanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PacmanMesh"));
    
    // 2. Make it the Root (so the actor moves when this moves)
    RootComponent = PacmanMesh;

    // 3. Find a shape to use (The standard Unreal Sphere)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere"));
    
    if (SphereAsset.Succeeded())
    {
        PacmanMesh->SetStaticMesh(SphereAsset.Object);
        // Pacman is small! Scale him down (Tile is 50, Sphere is 100)
        PacmanMesh->SetWorldScale3D(FVector(0.4f)); 
    }
}

void APacmanPawn::BeginPlay()
{
    Super::BeginPlay();

    // 1. Find the Maze Generator in the world so we can talk to it
    AActor* FoundMaze = UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGenerator::StaticClass());
    GameMaze = Cast<AMazeGenerator>(FoundMaze);

    // 2. Set Initial Position (e.g., Row 23, Col 13 is the standard start)
    if (GameMaze)
    {
        // Snap directly to the starting tile
        FVector StartPos = GameMaze->GetLocationFromGrid(17, 14); // Adjust based on your map
        SetActorLocation(StartPos);
    }
}

void APacmanPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind Keys to our functions
    // (Ensure you have these set up in Project Settings -> Input -> Axis Mappings 
    // OR just use Action Mappings for simplicity here)
    PlayerInputComponent->BindAction("MoveUp", IE_Pressed, this, &APacmanPawn::MoveUp);
    PlayerInputComponent->BindAction("MoveDown", IE_Pressed, this, &APacmanPawn::MoveDown);
    PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &APacmanPawn::MoveLeft);
    PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &APacmanPawn::MoveRight);
}

void APacmanPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GameMaze) return;
    

    // --- 1. MOVEMENT LOGIC ---
    
    // Get current Grid Coordinate (Round to nearest integer)
    FVector MyPos = GetActorLocation();
    int32 CurrentRow = FMath::FloorToInt(MyPos.X / GameMaze->TileSize);
    int32 CurrentCol = FMath::FloorToInt(MyPos.Y / GameMaze->TileSize);

    // Get the EXACT world center of that tile
    FVector TileCenter = GameMaze->GetLocationFromGrid(CurrentRow, CurrentCol);

    // Calculate distance to that center
    // We only care about 2D distance (X and Y)
    float DistToCenter = FVector::Dist2D(MyPos, TileCenter);
    // --- DEBUG LOGGING ---
    // This prints your distance and state every frame
    FString DebugMsg = FString::Printf(TEXT("Dist: %f | Row: %d Col: %d | CurrentDir: %d | NextDir: %d"), 
        DistToCenter, CurrentRow, CurrentCol, (int)CurrentDir, (int)NextDir);
    
    GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, DebugMsg);

    // Threshold: How close is "close enough" to turn? (e.g., 5 units)
    float CenterThreshold = 5.0f;

    // --- 2. AT INTERSECTION? ---
    if (DistToCenter < CenterThreshold)
    {
        // We are at the center! Now is the time to make decisions.

        // A. Try to turn to the Buffered Direction (NextDir)
        if (NextDir != EPadDirection::None && NextDir != CurrentDir)
        {
            FVector DirVec = GetVectorFromEnum(NextDir);
            int32 TargetRow = CurrentRow + (int32)DirVec.X;
            int32 TargetCol = CurrentCol + (int32)DirVec.Y;

            // Is the Next Direction a Wall?
            if (!GameMaze->IsWall(TargetRow, TargetCol))
            {
                // It's free! Snap to center and turn.
                SetActorLocation(TileCenter); 
                CurrentDir = NextDir;
                NextDir = EPadDirection::None; // Consume the buffer
            }
        }

        // B. Check if we hit a wall in our Current Direction
        FVector CurrentVec = GetVectorFromEnum(CurrentDir);
        int32 ForwardRow = CurrentRow + (int32)CurrentVec.X;
        int32 ForwardCol = CurrentCol + (int32)CurrentVec.Y;

        if (GameMaze->IsWall(ForwardRow, ForwardCol))
        {
            // Wall ahead! STOP.
            // Snap to center so we don't drift into the wall
            SetActorLocation(TileCenter);
            CurrentDir = EPadDirection::None;
        }
    }

    // --- 3. APPLY VELOCITY ---
    if (CurrentDir != EPadDirection::None)
    {
        FVector Movement = GetVectorFromEnum(CurrentDir) * MovementSpeed * DeltaTime;
        AddActorWorldOffset(Movement);
        
        // --- 4. LANE CORRECTION (Anti-Drift) ---
        // If moving Up/Down, force X to be centered on the column
        // If moving Left/Right, force Y to be centered on the row
        // This keeps him perfectly straight without physics
        FVector NewPos = GetActorLocation();
        if (CurrentDir == EPadDirection::Up || CurrentDir == EPadDirection::Down)
        {
             // Lock Y to the tile center
             NewPos.Y = FMath::FInterpTo(NewPos.Y, TileCenter.Y, DeltaTime, 10.0f);
        }
        else 
        {
             // Lock X to the tile center
            NewPos.X = FMath::FInterpTo(NewPos.X, TileCenter.X, DeltaTime, 10.0f);
        }
        SetActorLocation(NewPos);
    }
}

// --- INPUT HELPERS ---
void APacmanPawn::MoveUp()    
{ 
    { NextDir = EPadDirection::Up; }
    // Add this log!
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("MoveUp Key Pressed!"));
    
    NextDir = EPadDirection::Up; 
}
void APacmanPawn::MoveDown()  { NextDir = EPadDirection::Down; }
void APacmanPawn::MoveLeft()  { NextDir = EPadDirection::Left; }
void APacmanPawn::MoveRight() { NextDir = EPadDirection::Right; }



FVector APacmanPawn::GetVectorFromEnum(EPadDirection Dir)
{
    // Note: This mapping depends on your camera rotation.
    // Assuming standard: X is Up/Down (Rows), Y is Left/Right (Cols)
    switch (Dir)
    {
        case EPadDirection::Up:    return FVector(1, 0, 0);  // +X (Next Row)
        case EPadDirection::Down:  return FVector(-1, 0, 0); // -X (Prev Row)
        case EPadDirection::Right: return FVector(0, 1, 0);  // +Y (Next Col)
        case EPadDirection::Left:  return FVector(0, -1, 0); // -Y (Prev Col)
        default: return FVector::ZeroVector;
    }
}