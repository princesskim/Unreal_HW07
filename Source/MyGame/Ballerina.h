// Ballerina.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClockworkControlled.h"				// 게임타임에 따라 활성 제어되는 기능
#include "Ballerina.generated.h"

class UStaticMeshComponent;

UCLASS()
class MYGAME_API ABallerina : public AActor, public IClockworkControlled
{
	GENERATED_BODY()
	
public:	
	ABallerina();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetClockworkActive(bool bActive) override;	
												// 인터페이스 구현
												// Castle에서 활성제어 결과를 얻어오기

protected:
	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BallerinaMesh;

private:
	UPROPERTY(EditAnywhere, Category = "Ballerina", meta = (ClampMin = "0.0"))
	float RotationSpeed;						// 초당 회전 각도 (도/초)
	
	bool bIsClockworkActive;					// 현재 이동이 활성화 되어 있는지
};
