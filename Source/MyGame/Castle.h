// Castle.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Castle.generated.h"

class UStaticMeshComponent;
class UDirectionalLightComponent;
class ASwan;
class ASwanPath;

UCLASS()
class MYGAME_API ACastle : public AActor
{
	GENERATED_BODY()
	
public:	
	ACastle();
	
protected:
	virtual void BeginPlay() override;			// BeginPlay가 private인 이유: 엔진이 한 번만 호출하고 외부에서 중복 실행하지 않도록, 
												// 자식 클래스에서는 오버라이드할 수 있도록.
	
public:
	virtual void Tick(float DeltaTime) override;
	
	// ===== Getter : 현재 게임타임 조회 =====
	UFUNCTION(BlueprintPure, Category = "Castle|Time")
	float GetCurrentGameSeconds() const { return CurrentGameSeconds; }
												// 게임타임 누적 초 반환
												// BlueprintCallable - 실행 핀 있음
												// BlueprintPure - 실행 핀 없음 (Getter는 BP에서 실행핀 없는 노드로)
	
	UFUNCTION(BlueprintPure, Category = "Castle|Time")
	void GetCurrentGameTime(int32& OutHour, int32& OutMinute) const;
												// 참조로 받아서 원본 수정
												// 사람이 읽는 형식인 시/분 형식으로 반환
	
protected: 
	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CastleMesh;			// 메시 할당은 블루프린트 파생 클래스에서 진행하는 것이 일반적
													// VisibleAnywhere: C++로 생성된 컴포넌트를 에디터에서 지울 수 없도록
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClockBody;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClockNums;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClockHourHand;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClockMinuteHand;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> SunLight;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> MoonLight;
	
	// ===== 게임타임 설정 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Castle|Time", meta = (ClampMin = "1.0"))
	float GameSecondsPerRealSecond;				// 실제 1초당 흐르는 게임 초.
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Castle|Time", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartHour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Castle|Time", meta = (ClampMin = "0", ClampMax = "59"))
	int32 StartMinute;
												// UPROPERTY 메타데이터 ClampMin / ClampMax
												// 에디터의 디테일 창에서 해당 값의 최소/최대 범위를 제한
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Castle|Time")
	float CurrentGameSeconds;					// 현재 게임타임 (초 단위 누적)
	
	// ===== 시계 바늘 설정 =====
	UPROPERTY(EditAnywhere, Category = "Castle|Clock")
	bool bInvertClockRotation;					//시계방향, 시계반대방향 설정
	UPROPERTY(EditAnywhere, Category = "Castle|Clock")
	float ClockAngleOffset;						// 시계 바늘 12시 방향 보정용 (메쉬가 90도 돌아가 있음)
	
	// ===== 태양 설정 =====
	UPROPERTY(EditAnywhere, Category = "Castle|Sun")
	float SunYaw;								// 태양의 방위각
	UPROPERTY(EditAnywhere, Category = "Castle|Moon")
	float SunIntensity;	
	
	// ===== 달 설정 =====
	UPROPERTY(EditAnywhere, Category = "Castle|Moon")
	float MoonIntensity;						// 달의 빛 세기
	UPROPERTY(EditAnywhere, Category = "Castle|Moon")
	FLinearColor MoonColor;						// 달빛 색상
	
	// ===== 백조 설정 =====
	UPROPERTY(EditAnywhere, Category = "Castle|Swan")
	TSubclassOf<ASwan> SwanClass;				// C++ 로직 -> BP_Swan 파생 -> BP_Swan 스폰해야 함 (즉, 현재는 메시가 없어서 직접 스폰해도 안 보임)
												// TSubclassOf<T> : T를 상속받은 클래스 자체(인스턴스 말고 클래스 정보)를 담는 타입
												// T의 자식 클래스만 선택 가능하도록 타입 안전성을 보장
												// 에디터에서 이 변수를 열면 ASwan을 상속받은 클래스(C++/BP)만 뜸
	UPROPERTY(EditAnywhere, Category = "Castle|Swan")
	TObjectPtr<ASwanPath> SwanPath;				// ASwanPath: 스플라인 경로 액터
 
	UPROPERTY(EditAnywhere, Category = "Castle|Swan", meta = (ClampMin = "1"))
	int32 SwanCount;

	UPROPERTY(EditAnywhere, Category = "Castle|Swan", meta = (ClampMin = "0.0"))
	float SwanSpeed;							// 실제 시간 기반 백조 이동속도
	
	UPROPERTY()
	TArray<TObjectPtr<ASwan>> SpawnedSwans;		// 내부용 변수라서 따로 매크로 인자가 필요 없음
	
	// ===== 움직임 제어 설정 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Castle|Clockwork", meta = (ClampMin = "0", ClampMax = "23"))
	int32 ClockworkActiveStartHour;					// 백조, 발레리나 움직이기 시작하는 시간 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Castle|Clockwork", meta = (ClampMin = "0", ClampMax = "59"))
	int32 ClockworkActiveEndHour;					// 백조, 발레리나 멈추는 시간 지정
	
	
private:
	// ===== 내부 함수 =====
	void SpawnSwans();
	void UpdateClockHands();
	void UpdateSunAndMoon();
	bool IsClockworkActiveTime() const;			// 시간 계산 캡슐화 - 이 함수의 결과, 백조가 움직일지 말지 알 수 있음
												// 결과값을 백조에게 넘겨주기 위한 함수가 필요
	void RegisterClockworkActors();				// 월드 내 시간 제어 대상 검색 및 등록
	void NotifyClockworkActors(bool bActive);	// 모든 시간 제어 대상에게 활성/비활성 알림
	
	void UpdateClockworkActorsActiveState();				// 활성 상태에 변동이 있을 때만 전체 대상에게 알림
	
	// ===== 내부 변수 =====
	bool bWereClockworkActorsActive;					// IsSwanActiveTime() 의 반환값과 비교하여 백조에게 알림
											// 관례적으로 생성자에서 초기화 할 때, Swan 클래스의 bIsMovementActive 초기값과 동일하게 할 것.
											// Swan의 bIsMovementActive : 나 지금 움직이는 중?
											// Castle의 bWereSwansActive : 백조들이 활성인가?		-> 두 값이 동일해야 올바르게 활성 상태를 감지함
	
											// 생성자 초기값과 무관하게 무조건 정확한 상태로 동기화되는 트릭
													// bWereSwansActive를 현재 시간 기준의 반대값으로 강제 설정
													// UpdateSwansActiveState 호출 시 무조건 변화 감지 됨
													// 모든 Swan에게 정확한 상태 전파
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ClockworkActors;
											// 시간 제어 받는 모든 액터 수집 (인터페이스 기반)
};
