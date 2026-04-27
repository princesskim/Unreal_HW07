// BirdCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BirdCharacter.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	TObjectPtr<UFloatingPawnMovement> MovementComp;				// 중력 제약이 약한 자유 이동
																// 기본 전제가 “낙하”보다 “조종 가능한 공중 이동”에 더 가까움
	

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== 입력 바인딩 함수 =====
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()													// 리플렉션 시스템에 등록되어 있어야만 인식할 수 있음
	void Look(const FInputActionValue& value);					// Enhanced Input에서 액션 값은 FInputActionValue로 전달됨
	UFUNCTION()
	void Roll(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	
	// ===== 이동 속도 관련 프로퍼티 ===== 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed;
	
};