// Swan.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClockworkControlled.h"				// 게임타임에 따라 활성 제어되는 기능
#include "Swan.generated.h"

class UStaticMeshComponent;
class ASwanPath;

UCLASS()
class MYGAME_API ASwan : public AActor, public IClockworkControlled
{
	GENERATED_BODY()
	
public:	
	ASwan();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	void InitSwan(TObjectPtr<ASwanPath> InPath, float InDistanceAlongSpline, float InSpeed);
												// Castle에서 스폰 시 호출
												// Castle의 SpawnActor<ASwan>() 는 파라미터 없는 기본 생성자만 호출할 수 있음
												// 백조에게 경로, 시작 위치, 속도를 별도로 부여하는 단계가 필요

												// UE에서 UObject 계열 클래스는 파라미터 없는 생성자만 허용함
	virtual void SetClockworkActive(bool bActive) override;		
												// 인터페이스 구현
												// Castle에서 활성제어 결과를 얻어오기
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SwanMesh;
	
	UPROPERTY()
	TObjectPtr<ASwanPath> PathRef;
	
	float CurrentDistanceAlongSpline;
	float Speed;
	bool bIsClockworkActive;					// 현재 이동이 활성화 되어 있는지
	
};
