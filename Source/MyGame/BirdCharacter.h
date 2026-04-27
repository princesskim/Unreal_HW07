// BirdCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BirdCharacter.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
//class UFloatingPawnMovement;									// 코드 직접 구현을 위해 제외
struct FInputActionValue;

UCLASS()
class MYGAME_API ABirdCharacter : public APawn
{
	GENERATED_BODY()

public:
	ABirdCharacter();
	
	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Capsule")
	TObjectPtr<UBoxComponent> BoxComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComp;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	//TObjectPtr<UFloatingPawnMovement> MovementComp;				// 중력 제약이 약한 자유 이동
																// 기본 전제가 “낙하”보다 “조종 가능한 공중 이동”에 더 가까움
	
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void UpdateMovement(float DeltaTime);

	// ===== 입력 바인딩 함수 =====
	UFUNCTION()
	void StartMove(const FInputActionValue& value);
	UFUNCTION()
	void StopMove(const FInputActionValue& value);				// 물리를 수동으로 구현하다보니, 멤버변수 초기화가 필요해서 함수 추가
	UFUNCTION()													// 리플렉션 시스템에 등록되어 있어야만 인식할 수 있음
	void Look(const FInputActionValue& value);					// Enhanced Input에서 액션 값은 FInputActionValue로 전달됨
	UFUNCTION()
	void Roll(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	
	// ===== 이동 설정 =====
	UPROPERTY(EditAnywhere, Category = "Movement")
	float NormalSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeedMultiplier;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Gravity;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AccelerationRate;										//가속 시
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float DecelerationRate;										//감속 시
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float RotationInterpSpeed ;	
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxPitchAngle  ;	
	
	float SprintSpeed;
	float CurrentMaxSpeed;										// 현재 적용 중인 최대 속도
	
private:
	
	// ===== 이동 상태(런타임) =====
	UPROPERTY(VisibleAnywhere, Category = "Movement|Debug")
	FVector Velocity;
	
	UPROPERTY(VisibleAnywhere, Category = "Movement|Debug")
	FVector InputDirection;
	
	UPROPERTY(VisibleAnywhere, Category = "Movement|Debug")
	bool bIsGrounded;
	
};