// MyGameMode.cpp

#include "MyGameMode.h"
#include "BirdCharacter.h"
#include "MyPlayerController.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = ABirdCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	
}
