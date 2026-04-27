// Castle.cpp

#include "Castle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SplineComponent.h"
#include "Swan.h"
#include "SwanPath.h"
#include "Engine/World.h"
#include "ClockworkControlled.h"			// 인터페이스 헤더
#include "Kismet/GameplayStatics.h"

ACastle::ACastle()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// ===== 컴포넌트 생성 및 부착 =====
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));		// Root로 주로 사용하는 컴포넌트
	RootComponent = Root;
	
	CastleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CastleMesh"));
	CastleMesh->SetupAttachment(Root);
 
	ClockBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClockBody"));
	ClockBody->SetupAttachment(CastleMesh);
 
	ClockNums = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClockNums"));
	ClockNums->SetupAttachment(ClockBody);
 
	ClockHourHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HourHand"));
	ClockHourHand->SetupAttachment(ClockBody);
 
	ClockMinuteHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MinuteHand"));
	ClockMinuteHand->SetupAttachment(ClockBody);
	
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(Root);
	SunLight->SetMobility(EComponentMobility::Movable);					// 런타임에 회전할 수 있도록
	SunLight->SetAtmosphereSunLight(true);					// Sky Atmosphere가 이 라이트를 태양으로 인식
	SunLight->ForwardShadingPriority = 1;								// 반투명/안개 등의 렌더링에 하나의 DirectionalLight를 사용 가능
																		// 숫자가 클수록 메인이라는 뜻
	
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(Root);
	MoonLight->SetMobility(EComponentMobility::Movable);				// 런타임에 회전할 수 있도록
	MoonLight->ForwardShadingPriority = 0;								// 달이 보조가 되게끔	
	
	// ===== 기본값 초기화 =====
	GameSecondsPerRealSecond = 300.f;									// 실제 1초 = 게임타임 5분
	StartHour = 6;
	StartMinute = 0;
	CurrentGameSeconds = 0.f;
	bInvertClockRotation = false;
	ClockAngleOffset = 90.f;
	SunYaw = 0.f;
	SunIntensity = 10.f;
	MoonIntensity = 0.5f;
	MoonColor = FLinearColor(0.4f, 0.5f, 0.9f);			// 푸른 톤 새벽 감성
	SwanCount = 20;
	SwanSpeed = 150.f;
	ClockworkActiveStartHour = 6;
	ClockworkActiveEndHour = 22;
	bWereClockworkActorsActive = true;
}

void ACastle::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentGameSeconds = static_cast<float>(StartHour * 3600 + StartMinute * 60);
	
	SpawnSwans();
	RegisterClockworkActors();
	
	UpdateClockHands();
	UpdateSunAndMoon();
	
	const bool bInitialActive = IsClockworkActiveTime();
	NotifyClockworkActors(bInitialActive);
	bWereClockworkActorsActive = bInitialActive;
}

void ACastle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CurrentGameSeconds += DeltaTime * GameSecondsPerRealSecond;
	
	const float SecondsPerDay = 86400.f;
	if (CurrentGameSeconds >= SecondsPerDay)
	{
		CurrentGameSeconds = FMath::Fmod(CurrentGameSeconds, SecondsPerDay);
																		// % 연산자는 정수타입에서만 사용 가능
	}
	UpdateClockHands();
	UpdateSunAndMoon();
	UpdateClockworkActorsActiveState();
}

void ACastle::GetCurrentGameTime(int32& OutHour, int32& OutMinute) const
{
	const int32 TotalSeconds = FMath::FloorToInt32(CurrentGameSeconds) % 86400;
																		// 방어코드: 한번 더 % 86400
	OutHour = (TotalSeconds / 3600) % 24;
	OutMinute = (TotalSeconds % 3600) % 60;
}

void ACastle::SpawnSwans()
{
	if (!SwanClass || !SwanPath || !SwanPath->Spline || SwanCount < 1) return;
	
	UWorld* World = GetWorld();											// UWorld의 멤버 함수인 SpawnActor를 호출하기 위해 GetWorld() 필요
	if (!World) return;
	
	const float SplineLength = SwanPath->Spline->GetSplineLength();
	if (SplineLength <= 0.f) return;
	
	SpawnedSwans.Reserve(SwanCount);									// Capacity 확보 (최적화)
	const float SwanInterval = SplineLength / static_cast<float>(SwanCount);
	
	for (int32 i = 0 ; i < SwanCount ; ++i)
	{
		float DistanceAlongSpline = SwanInterval * i;
		// FActorSpawnParameters 설정 (콜리전 처리, Owner 지정)
		// SpawnActor로 ASwan 생성
		
		FActorSpawnParameters Params;									// SpawnActor에 전달하는 인자 FActorSpawnParameters는
																		// 스폰 방법 옵션들의 묶음 (구조체)
																		// 액터 이름, 소유자, 행동 주체, 배치할 레벨, 생성 지연 등 원하는 옵션만 채워서 전달
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Owner = this;											
																		// 옵션 1. 스폰 위치에 이미 다른 액터가 있으면 어떻게 할지 결정 - AlwaysSpawn
																			// 원점 (0,0,0)에 스폰한 뒤 바로 InitSwan으로 옮기는 과정에서 겹칠 수 있음
																		// 옵션 2. 이 액터의 "소유자" == this = ACastle 인스턴스
		
		TObjectPtr<ASwan> NewSwan = World->SpawnActor<ASwan>(SwanClass,
													FVector::ZeroVector,
													FRotator::ZeroRotator,
															Params);
		if (NewSwan)
		{
			NewSwan->InitSwan(SwanPath, DistanceAlongSpline, SwanSpeed);
			SpawnedSwans.Add(NewSwan);
		}
		
	}
	
}

void ACastle::UpdateClockHands()
{
	// ClockHourHand: 12 시간에 한 바퀴 == 43200 초
	// ClockMinuteHand : 60 분에 한 바퀴 == 3600 초
	
	const float HourHandElapsed = FMath::Fmod(CurrentGameSeconds, 43200.f);
	const float MinuteHandElapsed = FMath::Fmod(CurrentGameSeconds, 3600.f);
																		// 주기 내에서 경과된 초 수
	
	float HourHandAngle = (HourHandElapsed / 43200.f) * 360.f;			// 경과된 초 수만큼 비율에 따른 각도
	float MinuteHandAngle = (MinuteHandElapsed / 3600.f) * 360.f;			
	
	if (bInvertClockRotation)
	{
		HourHandAngle = -HourHandAngle;
		MinuteHandAngle = -MinuteHandAngle;
	}
	
	ClockHourHand->SetRelativeRotation(FRotator(HourHandAngle + ClockAngleOffset, 0.f, 0.f));
	ClockMinuteHand->SetRelativeRotation(FRotator(MinuteHandAngle + ClockAngleOffset, 0.f, 0.f));
																		// 시계 바늘이 y축 방향 바라보고 있음
																		// FRotator는 인자 순서가 pitch(y축), yaw(z축), roll(x축)
																		// 메쉬가 3시방향을 가리키게끔 90도 회전되어 임포트 된 것을 보정
	
}

void ACastle::UpdateSunAndMoon()
{
	if (!SunLight || !MoonLight) return;

	const float DayElapsedRatioAngle = CurrentGameSeconds / 86400.f * 360.f;
	const float SunPitchAngle = 90 - DayElapsedRatioAngle;				// Directional Light의 Pitch 값 == 빛이 쏘아지는 방향																	
																		// Pitch == 90		위로 빛 쏘기 (한밤중)			하루의 시작 (DayElapsedRatioAngle == 0)
																		// Pitch == 0		수평으로 빛 쏘기 (일출/일몰)	하루의 25% 경과 (DayElapsedRatioAngle == 90)
																		// Pitch == -90		아래로 빛 쏘기 (정오)			하루의 50% 경과 (DayElapsedRatioAngle == 180)
																		// Pitch == -180	아래로 빛 쏘기 (일출/일몰)	하루의 75% 경과 (DayElapsedRatioAngle == 270)
																		// 시간이 흐를수록 Pitch 값이 작아져야 함!
	
	const float MoonPitchAngle = SunPitchAngle + 180.f;					// 정반대 방향
	
	SunLight->SetWorldRotation(FRotator(SunPitchAngle, SunYaw, 0));
	MoonLight->SetWorldRotation(FRotator(MoonPitchAngle, SunYaw, 0));
	
	// 태양, 달의 높이에 따라 밝기가 달라지도록
	const float SunHeight = -FMath::Sin(FMath::DegreesToRadians(SunPitchAngle));
																		// Pitch == 90 이면 자정이니까 -1 나와야 함
																		// Pitch == 0 이면 일출/일몰이니까 0 나와야 함
																		// Pitch == -90 이면 정오니까 1 나와야 함
																		// 즉, -Sin() 그래프
	const float SunAlpha = FMath::Clamp(SunHeight, 0.f, 1.f);
	//const float SunIntensity(FMath::Lerp(0.f, 10.f, SunAlpha));
	//SunLight->SetIntensity(SunIntensity);
																		// Lerp의 알파 값은 0~1이 "정상 범위"
																		// 수평선 밑에 있을 때, SunHeight를 다 0으로 취급함
																		// 낮엔 최대 10, 밤엔 0
																		// Lerp : Alpha만큼 A와 B 사이를 보간한 값
	
	const float MoonAlpha = FMath::Clamp(-SunHeight, 0.f, 1.f);;
	MoonLight->SetIntensity(MoonIntensity * MoonAlpha);
	MoonLight->SetLightColor(MoonColor);
																		// 태양 정반대에 있으니까, 태양이 수평선 밑에 있으면 달은 떠있음
	
	
}

bool ACastle::IsClockworkActiveTime() const
{
	const int32 TotalSeconds = FMath::FloorToInt32(CurrentGameSeconds) % 86400;
																		// 방어코드: 한번 더 % 86400 (하루)
	int32 CurrentHour = (TotalSeconds / 3600) % 24;
	return (ClockworkActiveStartHour <= CurrentHour) && (CurrentHour< ClockworkActiveEndHour);
}

void ACastle::RegisterClockworkActors()
{
	UWorld* World = GetWorld();											// 월드 내 액터 검색을 위해
	if (!World) return;
	
	ClockworkActors.Empty();
	
	TArray<AActor*> FoundActors;
																		// 컴파일러는 TArray<TObjectPtr<AActor>> vs TArray<AActor*> 를 다른 타입으로 보기 때문에 
																		// FoundActors로 받은 후, ClockworkActors에 추가해주는 방식
	
	UGameplayStatics::GetAllActorsWithInterface(						// 인터페이스 구현한 액터만 추가
		World,
		UClockworkControlled::StaticClass(),
		FoundActors
	);
																		// UE 인터페이스는 클래스가 2개 생성됨
																		// IClockworkControlled  ← 실제 인터페이스 (순수 가상 함수 선언)
																		// UClockworkControlled  ← UObject 래퍼 (리플렉션/StaticClass() 담당)
	
	for (AActor* Actor : FoundActors)
	{
		ClockworkActors.Add(Actor);
	}
}

void ACastle::NotifyClockworkActors(bool bActive)
{
																		// for 루프 변수는 TObjectPtr<AActor>는 AActor*로의 암묵적 변환을 지원함
																		// 루프 안에서는 AActor*로 쓰는 경우가 대부분
																				// TObjectPtr의 이점(GC 추적 등)은 배열에 저장될 때 의미있고,
																				// 루프 변수에서는 어차피 임시로 꺼내 쓰는 거라 의미가 없음
	for (TObjectPtr<AActor> Actor : ClockworkActors)
	{
		if (Actor && Actor->Implements<UClockworkControlled>())			// null 체크 및 인터페이스가 실제 구현되어 있는지 확인
		{
			Cast<IClockworkControlled>(Actor)->SetClockworkActive(bActive);
																		// I 버전으로 캐스팅해서 인터페이스 함수 호출
		}
	}
}

void ACastle::UpdateClockworkActorsActiveState()
{
	const bool bShouldBeActive = IsClockworkActiveTime();
	
	if (bShouldBeActive != bWereClockworkActorsActive)					// 활성 상태 바뀜
	{
		NotifyClockworkActors(bShouldBeActive);							// 인터페이스 기반 호출
		bWereClockworkActorsActive = bShouldBeActive;
	}
}

