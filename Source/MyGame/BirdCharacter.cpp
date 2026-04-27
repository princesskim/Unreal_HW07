// BirdCharacter.cpp

#include "BirdCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"
#include "Math/RotationMatrix.h"

ABirdCharacter::ABirdCharacter()
{
 	PrimaryActorTick.bCanEverTick = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;							// 메쉬가 마우스에 끌려다니지 않도록
																// 대신 캐릭터 클래스의 경우, GetCharacterMovement()->bOrientRotationToMovement = true; 로
																// 진행방향을 자연스럽게 바라보는 것이 가능했음
																// 폰 클래스에서는 직접 코드로 구현해줘야 함! Move 함수 쪽에 구현하겠음!
	
	// ===== 컴포넌트 생성 및 부착 =====
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Capsule"));
	RootComponent = BoxComp;
	
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;				// 컨트롤러 회전할 때, 스프링암도 함께 회전
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
																// 스프링암의 끝부분을 의미하는 변수 (SocketName)
	CameraComp->bUsePawnControlRotation = false;
	
	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
																// 로직 컴포넌트라서, 계층구조에 붙을 필요 없음
	
	// ===== 기본값 초기화 =====
	NormalSpeed = 1200.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	
	UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = NormalSpeed;
	}															// 캐릭터 클래스에서 GetCharacterMovement()로 하는 것을
																// 폰 클래스에서는 이렇게 함!
}

void ABirdCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);		// 컨트롤러 회전할 때, 카메라는 회전하지 않음
																// 삼각대인 스프링암이 움직이기 때문
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,										// 키가 눌렸을 때 호출된 함수의 객체의 포인터
					&ABirdCharacter::Move
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ABirdCharacter::Look
				);
			}
			if (PlayerController->RollAction)
			{
				EnhancedInput->BindAction(
					PlayerController->RollAction,
					ETriggerEvent::Triggered,
					this,
					&ABirdCharacter::Roll
				);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ABirdCharacter::StartSprint
				);
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ABirdCharacter::StopSprint
				);
			}
		}
	}
}

void ABirdCharacter::Move(const FInputActionValue& value)
{
	if (!Controller) return;
	// Enhanced Input에서 액션 값을 받아온 구조체에서 IA_Move의 입력값 (WASDQR) 얻기
	const FVector MoveInput = value.Get<FVector>();
	
	// 이동
	// 단, 액터 기준 좌우앞뒤가 아니고, 플레이어 기준이어야 함!
	// 상하 이동은 무조건 월드 기준
	float ControllerYaw = Controller->GetControlRotation().Yaw;
	FRotator YawRotation(0.f,ControllerYaw, 0.f);
	
	FVector ControllerForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector ControllerRightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ControllerForwardVector, MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(ControllerRightVector, MoveInput.Y); 
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddMovementInput(FVector::UpVector, MoveInput.Z);
	}
	
	// 이동방향으로 자연스럽게 회전 (상하로 갈 때는 꺾이지 않음)
	FVector Direction = MoveInput.X * ControllerForwardVector
						+ MoveInput.Y * ControllerRightVector;
	
	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRotation  = Direction.Rotation();
		FRotator NewRotation = FMath::RInterpTo(	// Rotator 보간 함수, 외에도 FInterpTo(), VInterpTo()가 있음
			GetActorRotation(),						// 현재 회전값에서 출발
			TargetRotation,							// 목표 회전값, 이 방향으로 보간
			GetWorld()->GetDeltaSeconds(),			// DeltaTime, 프레임레이트와 무관하게 일정한 속도로 회전하도록
			7.f										// 보간 속도(InterpSpeed), "1초에 남은 거리의 몇 배를 줄일지"
		);
		SetActorRotation(NewRotation);
	}
	
}

void ABirdCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ABirdCharacter::Roll(const FInputActionValue& value)
{
	// 액터만 Roll, 카메라는 고정
	float RollInput = value.Get<float>();
	
	if (!FMath::IsNearlyZero(RollInput))
	{
		FRotator NewRotation = GetActorRotation();
		NewRotation.Roll += RollInput;
		SetActorRotation(NewRotation);
	}
}

void ABirdCharacter::StartSprint(const FInputActionValue& value)
{
	UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = SprintSpeed;
	}
}

void ABirdCharacter::StopSprint(const FInputActionValue& value)
{
	UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = NormalSpeed;
	}
}
