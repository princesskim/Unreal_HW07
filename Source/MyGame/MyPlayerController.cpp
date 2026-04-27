// MyPlayerController.cpp

#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"

AMyPlayerController::AMyPlayerController()
	: InputMappingContext(nullptr), MoveAction(nullptr), LookAction(nullptr), RollAction(nullptr), SprintAction(nullptr)
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())		// 현재 PlayerController에 연결된 Local Player 객체를 가져옴
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem 
			= LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
															// Local Player에서 EnhancedInputLocalPlayerSubsystem을 획득
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
															// Subsystem을 통해 우리가 할당한 IMC를 활성화
															// 우선순위 0순위
			}
		}
	}
}
