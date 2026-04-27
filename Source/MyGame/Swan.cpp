// Swan.cpp

#include "Swan.h"

#include "SwanPath.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"

ASwan::ASwan()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	// ===== 컴포넌트 생성 및 부착 =====
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	SwanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwanMesh"));
	SwanMesh->SetupAttachment(Root);
	
	// ===== 기본값 초기화 =====
	CurrentDistanceAlongSpline = 0.f;
	Speed = 150.f;										// 어차피 InitSwan에서 Castle의 SwanSpeed로 덮어씀
	bIsClockworkActive = true;
}

void ASwan::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASwan::InitSwan(TObjectPtr<ASwanPath> InPath, float InDistanceAlongSpline, float InSpeed)
{
	// Castle의 SpawnSwans에 의해 백조가 스폰되면 바로, InitSwan 호출됨
	// 스플라인 상 거리에 따라 백조들의 위치와 바라보는 각도를 초기화 함
	
	PathRef = InPath;
	CurrentDistanceAlongSpline = InDistanceAlongSpline;
	Speed = InSpeed;
	
	if (PathRef && PathRef->Spline)
	{
		const FVector Loc = PathRef->Spline->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongSpline,
																			ESplineCoordinateSpace::World);
														// 얻어온 Spline Point에 백조를 배치할건데, 
														// SetActorLocation은 월드좌표를 기대함
														// SwanPath 액터 기준으로 포인트 좌표 가져오면 엉뚱한 곳에 가게 됨
		
		const FRotator Rot = PathRef->Spline->GetRotationAtDistanceAlongSpline(CurrentDistanceAlongSpline,
																			ESplineCoordinateSpace::World);
														// 백조가 '스플라인의 진행방향'을 바라보도록
		
		SetActorLocation(Loc);
		SetActorRotation(Rot);
			
	}
}

void ASwan::SetClockworkActive(bool bActive)
{
	bIsClockworkActive = bActive;
}


void ASwan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsClockworkActive) return;						// 안 움직이는 시간이기 때문에 위치 업데이트 생략
	if (!PathRef || !PathRef->Spline) return;
	
	// 실시간으로 백조 객체가 스플라인 위로 이동한 누적 거리를 측정하고,
	// 현재 거리에 해당하는 위치와 회전값을 적용해 백조의 움직임 구현
	
	CurrentDistanceAlongSpline += Speed * DeltaTime;
	
	const float SplineLength = PathRef->Spline->GetSplineLength();
	if (SplineLength <= 0.f) return;
	
	if (CurrentDistanceAlongSpline >= SplineLength)
	{
		CurrentDistanceAlongSpline = FMath::Fmod(CurrentDistanceAlongSpline, SplineLength);
	}
	
	const FVector Loc = PathRef->Spline->GetLocationAtDistanceAlongSpline(CurrentDistanceAlongSpline,
																			ESplineCoordinateSpace::World);
	const FRotator Rot = PathRef->Spline->GetRotationAtDistanceAlongSpline(CurrentDistanceAlongSpline,
																				ESplineCoordinateSpace::World);
		
	SetActorLocation(Loc);
	SetActorRotation(Rot);
}
