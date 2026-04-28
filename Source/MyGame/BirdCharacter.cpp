// BirdCharacter.cpp

#include "BirdCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
//#include "GameFramework/FloatingPawnMovement.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"
#include "Math/RotationMatrix.h"

ABirdCharacter::ABirdCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;							// 메쉬가 마우스에 끌려다니지 않도록
																// 대신 캐릭터 클래스의 경우, GetCharacterMovement()->bOrientRotationToMovement = true; 로
																// 진행방향을 자연스럽게 바라보는 것이 가능했음
																// 폰 클래스에서는 직접 코드로 구현해줘야 함!
	
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
	
	//MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
																// 로직 컴포넌트라서, 계층구조에 붙을 필요 없음
	
	BoxComp->SetSimulatePhysics(false);
	SkeletalMeshComp->SetSimulatePhysics(false);		// 물리 대신 코드로 직접 제어
	
	// ===== 기본값 초기화 =====
	NormalSpeed = 1200.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	CurrentMaxSpeed = NormalSpeed;
	Velocity = FVector::ZeroVector;
	InputDirection = FVector::ZeroVector;
	bIsGrounded = false;
	Gravity = -980.f;
	AccelerationRate = 10.f;
	DecelerationRate = 7.f;
	RotationInterpSpeed = 1.f;
	MaxPitchAngle = 45.f;
	
	
	/*
	UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = NormalSpeed;
	}															// 캐릭터 클래스에서 GetCharacterMovement()로 하는 것을
																// 폰 클래스에서는 이렇게 함!*/
}

void ABirdCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateMovement(DeltaSeconds);
	
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
					&ABirdCharacter::StartMove
				);
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Completed,
					this,
					&ABirdCharacter::StopMove
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

void ABirdCharacter::UpdateMovement(float DeltaTime)
{
	// [1] 목표 속도 계산
	FVector NormalizedInput = InputDirection.GetSafeNormal();				// GetSafeNormal은 내부적으로 임계값 이하면 0반환
																			// 키보드 인풋이기 때문에 입력 강도 상관 없이 0 또는 1임.
																			// 대각선 방향으로 갈 때 스피드가 루트2되지 않도록 정규화
																			// StopMove에서 Completed 바인딩하고, InputDirection을 ZeroVector로 만들어주기 때문에
																			// 키보드 환경에서는 거의 0같은 입력이 크기 1로 정규화되는 일이 없음 (안전!)
	FVector TargetVelocity  = NormalizedInput * CurrentMaxSpeed;			// 속도 = 단위방향벡터 * 속력
	
	
	// [2] 가속/감속 판단
	bool bAccelerating = !InputDirection.IsNearlyZero();					// 입력 유무로 알 수 있음 (입력 중이면 가속)
																			// 입력이 없으면 InputDirection이 무조건 ZeroVector 이기 때문에 IsNearlyZero가 true 나옴
	float InterpRate = bAccelerating ? AccelerationRate : DecelerationRate;	// 보간 속도
																			// 가속 중이면 보간 속도를 AccelerationRate, 
																			// 감속 중이면 DecelerationRate
	
	// [3] 속도 보간 (+중력 적용)
	Velocity = FMath::VInterpTo(Velocity, TargetVelocity, DeltaTime,InterpRate);
																			// 여러 프레임에 걸쳐 목표 속도에 도달
	
	if (FMath::IsNearlyZero(InputDirection.Z))							// Z축 방향 운동이 없을 경우에만 중력가속도 적용
	{
		Velocity.Z += Gravity * DeltaTime;									// 매 초마다 중력 가속도가 쌓이도록
	}
	
	
	//UE_LOG(LogTemp, Warning, TEXT("Velocity: (%.1f, %.1f, %.1f), Z IsNearlyZero: %d"), Velocity.X, Velocity.Y, Velocity.Z,FMath::IsNearlyZero(InputDirection.Z));
	
	// [4] 위치 업데이트
	AddActorWorldOffset(Velocity * DeltaTime, true);		// 컨트롤러 기준으로 움직일거라서 LocalOffset 보다는 WorldOffset으로 진행
																			// bSweep = true 이면 가는 길을 쓸고 지나가듯 검사해서
																			// 이동 경로 중 막히는 물체가 있으면 거기서 멈추거나 충돌 결과를 남김
	
	// [5] 메시 회전 처리														// 새가 부드러운 곡선으로 진행 방향 바꿈. 앞으로 가다 오른쪽으로 휙 직각 꺾임 없도록.
	FVector HorizontalVelocity = Velocity;
	HorizontalVelocity.Z = 0;
	
	bool bHasHorizontalMovement = !HorizontalVelocity.IsNearlyZero();
	
	if (bHasHorizontalMovement)												// 컨트롤러의 Pitch 값을 포함하여 메쉬가 이동방향을 봄. (즉, 위/아래 진행방향을 봄)
	{
		FRotator TargetRotation  = Velocity.Rotation();						//FVector::Rotation()은 벡터의 방향을 Pitch/Yaw로 변환
		
		if (TargetRotation.Pitch > 180) TargetRotation.Pitch -= 360.f;		//270도 -> -90도 변경
		if (TargetRotation.Pitch > 45.f) TargetRotation.Pitch = 45.f;
		if (TargetRotation.Pitch < -45.f) TargetRotation.Pitch = -45.f;		//수동으로 Clamp 해주기
		
		
		FRotator NewRotation = FMath::RInterpTo(	// Rotator 보간 함수, 외에도 FInterpTo(), VInterpTo()가 있음
			GetActorRotation(),						// 현재 회전값에서 출발
			TargetRotation,							// 목표 회전값, 이 방향으로 보간
			DeltaTime,								// DeltaTime, 프레임레이트와 무관하게 일정한 속도로 회전하도록
			RotationInterpSpeed						// 보간 속도(InterpSpeed), "1초에 남은 거리의 몇 배를 줄일지"
		);
		
		SetActorRotation(NewRotation);
	}	
	else																	// Q, E키로 순수 상하운동 할 때는 메쉬 회전 없이 그대로 
	{
		FRotator CurrentRotation = GetActorRotation();
		if (CurrentRotation.Pitch > 180) CurrentRotation.Pitch -= 360.f;	//270도 -> -90도 변경
		
		if (CurrentRotation.Pitch > 45.f) CurrentRotation.Pitch = 45.f;
		if (CurrentRotation.Pitch < -45.f) CurrentRotation.Pitch = -45.f;	//수동으로 Clamp 해주기
		
		FRotator TargetRotation = FRotator(0.f, CurrentRotation.Yaw, 0.f);
																			// 상하 운동 시, 점진적으로 수평이 되도록
		
		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,	
			DeltaTime,	
			RotationInterpSpeed	
		);
		SetActorRotation(NewRotation);
	}
	
	
}

void ABirdCharacter::StartMove(const FInputActionValue& value)
{
	if (!Controller) return;
																			// Enhanced Input에서 액션 값을 받아온 구조체에서 IA_Move의 입력값 (WASDQR) 얻기
	const FVector MoveInput = value.Get<FVector>();
	
	FRotator FullRotation = Controller->GetControlRotation();
	FVector ForwardVector = FRotationMatrix(FullRotation).GetUnitAxis(EAxis::X);	// Pitch 포함
	
	FRotator YawRotation(0.f,FullRotation.Yaw, 0.f);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);		// Roll/Pitch 영향받지 않도록
	
	//UE_LOG(LogTemp, Warning, TEXT("Pitch: %f"), FullRotation.Pitch);
	
	/*
	// 이동
	// 단, 액터 기준 좌우앞뒤가 아니고, 플레이어 기준이어야 함!
	// 상하 이동도 무조건 월드 기준
	float ControllerYaw = Controller->GetControlRotation().Yaw;
	FRotator YawRotation(0.f,ControllerYaw, 0.f);
	
	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	*/
	
	
	/*if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ControllerForwardVector, MoveInput.X);				// UFloatingPawnMovement 있어야 사용 가능한 것
		
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(ControllerRightVector, MoveInput.Y); 
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddMovementInput(FVector::UpVector, MoveInput.Z);
	}*/
	
	
	InputDirection = MoveInput.X * ForwardVector
						+ MoveInput.Y * RightVector
						+ MoveInput.Z * FVector::UpVector;					// 기존에는 "실제 이동"까지 여기서 했다면
																			// 지금은 "이동 의도(방향)"를 저장만 함. 실제 이동은 Tick에서.
																			// 시선 비행 + 명시적 월드 Z축 비행 
	
	// 기존에 여기 있던 진행 방향 바라보는 회전 보간도 UpdateMovement로 이동
}

void ABirdCharacter::StopMove(const FInputActionValue& value)
{
	InputDirection = FVector::ZeroVector;
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
	/*UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = SprintSpeed;
	}*/
	
	CurrentMaxSpeed = SprintSpeed;
}

void ABirdCharacter::StopSprint(const FInputActionValue& value)
{
	/*UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if (FloatingMovement)
	{
		FloatingMovement->MaxSpeed = NormalSpeed;
	}*/
	
	CurrentMaxSpeed = NormalSpeed;
}
