// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.h"
#include "Lighter.h"
#include "GameHUD.h"
#include "Kismet/GameplayStatics.h"
// TODO delete?
#include "UObject/ConstructorHelpers.h"

// Movement controls
void AMainPlayerController::MoveUp(const float value)
{
	//if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveUp(value);
}
void AMainPlayerController::MoveRight(const float value)
{
	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveRight(value);
}

// Rotation controls
void AMainPlayerController::LookWithMouse()
{
	if (GameMode->bHasWon || bShowingMenu || bShowingHelp)
		return;

	if (!MainCharacter || MainCharacter->bIsDisabled)
		return;

	FVector mLocation, mDirection;
	if (!DeprojectMousePositionToWorld(mLocation, mDirection))
		return;

	const FVector cLocation = MainCharacter->GetActorLocation();

	FVector direction = mLocation - cLocation;
	// TODO change something to use camera lag
	//FVector direction = mLocation + mDirection * MainCharacter->CameraBoom->CameraLagSpeed - cLocation;
	//FVector direction = mLocation - MainCharacter->TopDownCamera->GetComponentLocation();
	direction.Z = 0.0f;

	MainCharacter->Look(direction);
}
void AMainPlayerController::LookWithStick()
{
	if (GameMode->bHasWon || bShowingMenu || bShowingHelp)
		return;

	if (!MainCharacter || MainCharacter->bIsDisabled)
		return;

	const float xValue = InputComponent->GetAxisValue("LookUp");
	const float yValue = InputComponent->GetAxisValue("LookRight");

	FVector direction(xValue, yValue, 0.0f);

	// Doesn't work on small direction vectors to prevent undesired rotations
	// Also doesn't override bLookWithMouse if no input is found
	if (direction.Size() < 0.25f)
		return;

	if (bLookWithMouse)
	{
		// Now we look with stick
		bLookWithMouse = false;
		// And we dont need cursor
		bShowMouseCursor = false;
	}

	MainCharacter->Look(direction);
}

// Makes the character use something he has equiped
void AMainPlayerController::UseEquiped()
{
	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->UseEquiped();
}
// Makes the character avtivate smth near him
void AMainPlayerController::Activate()
{
	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Activate();
}
// Equipes first item
void AMainPlayerController::Equip1()
{
	if (bShowingMenu)
		Restart();
	if (!bShowingHelp && !GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip1();
}
// Equipes second item
void AMainPlayerController::Equip2()
{
	if (bShowingMenu)
		ToMainMenu();
	if (!bShowingHelp && !GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip2();
}
// Equipes third item
void AMainPlayerController::Equip3()
{
	if (bShowingMenu)
		ExitGame();
	// TODO
}

// Show/Hide menu
void AMainPlayerController::ShowHideMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowHideMenu called"));

	/*if (bShowingHelp)
		ShowHideHelp();*/

	bShowingMenu = !bShowingMenu;
	HUD->ShowHideMenu(bShowingMenu);
	
	UGameplayStatics::SetGlobalTimeDilation(this, bShowingMenu || bShowingHelp ? 0.05f : 1.f);
	// SetPause(bShowingMenu);
}
// Show/Hide help
void AMainPlayerController::ShowHideHelp()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowHideHelp called"));

	/*if (bShowingMenu)
		ShowHideMenu();*/

	bShowingHelp = !bShowingHelp;
	HUD->ShowHideHelp(bShowingHelp);

	UGameplayStatics::SetGlobalTimeDilation(this, bShowingMenu || bShowingHelp ? 0.05f : 1.f);
	// SetPause(bShowingHelp);
}

// Restarts the game
void AMainPlayerController::Restart()
{
	HUD->OnRestart();

	// We add a small delay to show loading screen
	FTimerHandle handler;
	GetWorldTimerManager().SetTimer(handler, this, &AMainPlayerController::RestartLevel, 1.0f, false, 0.001f); // Its that small because of the low time dilation
	// RestartLevel();

	// Haven't tried
	// ClientTravel(TEXT("?restart"), TRAVEL_Relative);
	// GetWorld()->Exec(GetWorld(), TEXT("RestartLevel"));
	// UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
	
	// Doesn't work
	// GameMode->ResetLevel();
}
// Returns to main menu
void AMainPlayerController::ToMainMenu()
{
	HUD->OnChangeMap();
	// TODO
}
// Exits game
void AMainPlayerController::ExitGame()
{
	HUD->OnExit();
	FGenericPlatformMisc::RequestExit(false);
}

// Resets map, only used for debug
void AMainPlayerController::ResetMap()
{
	// UE_LOG(LogTemp, Warning, TEXT("ResetMap called"));

	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->ResetMap();
}

// Takes one 'life' and calls CalculateLoss
void AMainPlayerController::OnDisabled()
{
	if (Lives > 0)
	{
		--Lives;
		UE_LOG(LogTemp, Warning, TEXT("Disabled once"));

		CalculateLoss();
	}
}
// Checks for the loss and calls OnLoss
void AMainPlayerController::CalculateLoss()
{
	if (Lives <= 0)
		OnLoss();
	else
	{
		// TODO make gamemode do this
		// Respawn after a delay
		FTimerHandle handler;
		((AActor*)this)->GetWorldTimerManager().SetTimer(handler, this, &AMainPlayerController::Enable, 1.0f, false, RespawnDelay);
	}
}
// Called on loss
void AMainPlayerController::OnLoss()
{
	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->OnLoss();

	// TODO
}
// Respawns the character
void AMainPlayerController::Enable()
{
	// We only unposses at this point, otherwise we would have no character during the respawn delay
	// UnPossess();
	// GetWorld()->GetAuthGameMode()->RestartPlayer(this);
	// MainCharacter = Cast<AMainCharacter>(GetCharacter());

	MainCharacter->Enable();
}

// Sets default values
AMainPlayerController::AMainPlayerController()
{
	// Show cursor in game
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	// bShouldPerformFullTickWhenPaused = true;

	static ConstructorHelpers::FObjectFinder<UClass> gameHUDBP(TEXT("Class'/Game/Blueprints/GameHUDBP.GameHUDBP_C'"));
	if (gameHUDBP.Succeeded())
		HUDAssetClass = gameHUDBP.Object;
}

// Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Find game mode
	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	// Find character
	MainCharacter = Cast<AMainCharacter>(GetCharacter());

	// Add HUD
	// UClass* HUDAssetClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("Class'/Game/Blueprints/GameHUDBP.GameHUDBP_C'")); //("WidgetBlueprint'/Game/Blueprints/GameHUDBP.GameHUDBP_C'"));
	HUD = Cast<UGameHUD>(CreateWidget<UUserWidget>(this, HUDAssetClass));
	HUD->Controller = this;
	HUD->Character = MainCharacter;
	MainCharacter->HUD = HUD;
	HUD->GameMode = GameMode;
	GameMode->HUD = HUD;
	HUD->AddToViewport();

	HUD->ShowHideWarning(true, FText::FromString("You wake up in a room with a white keycard and a lighter in your pockets. You don't remember where you are, but this looks like some kind of a lab"));

	GetMousePosition(LastMousePosition.X, LastMousePosition.Y);

	// We spawn a lighter for the character
	ALighter* lighter = GameMode->SpawnLighter(0, 0);
	lighter->ActivateObject(MainCharacter);
	Equip1();
	UseEquiped();

	// SetTickableWhenPaused(true);
}

// Sets controls
void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// check(InputComponent);

	// Bind axis and buttons for use

	InputComponent->BindAxis("MoveUp", this, &AMainPlayerController::MoveUp);
	InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);

	InputComponent->BindAxis("LookUp");
	InputComponent->BindAxis("LookRight");

	InputComponent->BindAction("UseEquiped", IE_Pressed, this, &AMainPlayerController::UseEquiped);
	InputComponent->BindAction("Activate", IE_Pressed, this, &AMainPlayerController::Activate);
	InputComponent->BindAction("Equip1", IE_Pressed, this, &AMainPlayerController::Equip1);
	InputComponent->BindAction("Equip2", IE_Pressed, this, &AMainPlayerController::Equip2);
	InputComponent->BindAction("Equip3", IE_Pressed, this, &AMainPlayerController::Equip3);

	InputComponent->BindAction("Menu", IE_Pressed, this, &AMainPlayerController::ShowHideMenu);
	InputComponent->BindAction("Help", IE_Pressed, this, &AMainPlayerController::ShowHideHelp);

	InputComponent->BindAction("DebugReset", IE_Pressed, Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()), &AMainGameMode::ResetMap);
	InputComponent->BindAction("DebugShow", IE_Pressed, Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()), &AMainGameMode::ShowHideDebug);
}

// Called every frame
void AMainPlayerController::PlayerTick(const float deltaTime)
{
	Super::PlayerTick(deltaTime);

	// We update LastMousePosition and make sure to enable mouse controls if mouse was moved
	FVector2D newMousePosition;
	GetMousePosition(newMousePosition.X, newMousePosition.Y);
	if ((LastMousePosition - newMousePosition).Size() > 0.0f)
	{	// We look with mouse again
		bLookWithMouse = true;
		// And we need cursor
		bShowMouseCursor = true;
	}
	LastMousePosition = newMousePosition;

	if (bLookWithMouse)
		LookWithMouse();

	// We try to look with stick anyway
	// If we get input, we start looking with stick
	LookWithStick();
}