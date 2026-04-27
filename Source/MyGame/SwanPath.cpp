// SwanPath.cpp

#include "SwanPath.h"
#include "Components/SplineComponent.h"

// Sets default values
ASwanPath::ASwanPath()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	// ===== 컴포넌트 생성 및 부착 =====
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
	
	// ===== 스플라인 설정값 =====
	Spline->SetClosedLoop(true);				// 닫힌 루프로 설정 (백조가 계속 순환)
	
}
