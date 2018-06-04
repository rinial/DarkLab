// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"
#include "MainCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.h"
// #include "MenuGameMode.h"
#include "Lighter.h"
#include "GameHUD.h"
#include "MenuHUD.h"
#include "Kismet/GameplayStatics.h"
// TODO delete?
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"

// Movement controls
void AMainPlayerController::MoveUp(const float value)
{
	if (bIsMenu)
		return;

	//if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveUp(value);
}
void AMainPlayerController::MoveRight(const float value)
{
	if (bIsMenu)
		return;

	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->MoveRight(value);
}

// Rotation controls
void AMainPlayerController::LookWithMouse()
{
	if (bIsMenu)
		return;

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
	if (bIsMenu)
		return;

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
	if (bIsMenu)
		return;

	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->UseEquiped();
}
// Makes the character avtivate smth near him
void AMainPlayerController::Activate()
{
	if (bIsMenu)
		return;

	// if (!bShowingMenu && !bShowingHelp && MainCharacter && !MainCharacter->bIsDisabled)
	if (!GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Activate();
}
// Equipes first item
void AMainPlayerController::Equip1()
{
	if (bIsMenu)
		Start();
	else if (bShowingMenu)
		Restart();
	else if (!bShowingHelp && !GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip1();
}
// Equipes second item
void AMainPlayerController::Equip2()
{
	if (bIsMenu)
		ExitGame();
	else if (bShowingMenu)
		ToMainMenu();
	else if (!bShowingHelp && !GameMode->bHasWon && MainCharacter && !MainCharacter->bIsDisabled)
		MainCharacter->Equip2();
}
// Equipes third item
void AMainPlayerController::Equip3()
{
	if (bIsMenu)
		return;

	if (bShowingMenu)
		ExitGame();
}

// Show/Hide menu
void AMainPlayerController::ShowHideMenu()
{
	if (bIsMenu)
	{
		// ExitGame();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ShowHideMenu called"));

	/*if (bShowingHelp)
		ShowHideHelp();*/

	bShowingMenu = !bShowingMenu;
	GameHUD->ShowHideMenu(bShowingMenu);
	
	UGameplayStatics::SetGlobalTimeDilation(this, bShowingMenu || bShowingHelp ? 0.05f : 1.f);
	// SetPause(bShowingMenu);
}
// Show/Hide help
void AMainPlayerController::ShowHideHelp()
{
	if (bIsMenu)
		return;

	UE_LOG(LogTemp, Warning, TEXT("ShowHideHelp called"));

	/*if (bShowingMenu)
		ShowHideMenu();*/

	bShowingHelp = !bShowingHelp;
	GameHUD->ShowHideHelp(bShowingHelp);

	UGameplayStatics::SetGlobalTimeDilation(this, bShowingMenu || bShowingHelp ? 0.05f : 1.f);
	// SetPause(bShowingHelp);
}

// Starts the game
void AMainPlayerController::Start()
{
	if (!bIsMenu)
		return;

	MenuHUD->OnChangeMap();

	// We add a small delay to show loading screen
	FTimerHandle handler;
	GetWorldTimerManager().SetTimer(handler, this, &AMainPlayerController::FinishStart, 1.0f, false, 0.01f);
}
// Restarts the game
void AMainPlayerController::Restart()
{
	if (bIsMenu)
		return;

	GameHUD->OnRestart();

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
	if (bIsMenu)
		return;
	
	GameHUD->OnChangeMap();

	// We add a small delay to show loading screen
	FTimerHandle handler;
	GetWorldTimerManager().SetTimer(handler, this, &AMainPlayerController::FinishToMainMenu, 1.0f, false, 0.001f); // Its that small because of the low time dilation
}
// Exits game
void AMainPlayerController::ExitGame()
{
	if (bIsMenu)
		MenuHUD->OnExit();
	else
		GameHUD->OnExit();

	FGenericPlatformMisc::RequestExit(false);
}

// Resets map, only used for debug
void AMainPlayerController::ResetMap()
{
	if (bIsMenu)
		return;

	// UE_LOG(LogTemp, Warning, TEXT("ResetMap called"));

	Cast<AMainGameMode>(GetWorld()->GetAuthGameMode())->ResetMap();
}

// Some fucntions called after a delay
void AMainPlayerController::FinishStart()
{
	UGameplayStatics::OpenLevel(this, FName("TestMap"));
}
void AMainPlayerController::FinishToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName("MenuMap"));
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
		GameHUDAssetClass = gameHUDBP.Object;
	static ConstructorHelpers::FObjectFinder<UClass> menuHUDBP(TEXT("Class'/Game/Blueprints/MenuHUDBP.MenuHUDBP_C'"));
	if (menuHUDBP.Succeeded())
		MenuHUDAssetClass = menuHUDBP.Object;
}

// Called when the game starts or when spawned
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Find game mode and decide if it's menu
	GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
		bIsMenu = true;

	if (!bIsMenu)
	{
		// Find character
		MainCharacter = Cast<AMainCharacter>(GetCharacter());

		// Add HUD
		// UClass* GameHUDAssetClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("Class'/Game/Blueprints/GameHUDBP.GameHUDBP_C'")); //("WidgetBlueprint'/Game/Blueprints/GameHUDBP.GameHUDBP_C'"));
		GameHUD = Cast<UGameHUD>(CreateWidget<UUserWidget>(this, GameHUDAssetClass));
		GameHUD->Controller = this;
		GameHUD->Character = MainCharacter;
		MainCharacter->GameHUD = GameHUD;
		GameHUD->GameMode = GameMode;
		GameMode->GameHUD = GameHUD;
		GameHUD->AddToViewport();

		GameHUD->ShowHideWarning(true, FText::FromString("You wake up in a room with a white keycard and a lighter in your pockets. You don't remember where you are, but this looks like some kind of a lab"));

		// We spawn a lighter for the character
		ALighter* lighter = GameMode->SpawnLighter(0, 0);
		lighter->ActivateObject(MainCharacter);
		Equip1();
		UseEquiped();

		// SetTickableWhenPaused(true);
	}
	else
	{
		// Add HUD
		MenuHUD = Cast<UMenuHUD>(CreateWidget<UUserWidget>(this, MenuHUDAssetClass));
		MenuHUD->Controller = this;
		MenuHUD->AddToViewport();

		this->SetInputMode(FInputModeGameAndUI());
	}
	
	GetMousePosition(LastMousePosition.X, LastMousePosition.Y);
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