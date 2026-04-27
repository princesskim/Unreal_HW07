// Ballerina.cpp

#include "Ballerina.h"
#include "Components/StaticMeshComponent.h"

ABallerina::ABallerina()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	// ===== 컴포넌트 생성 및 부착 =====
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	BallerinaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallerinaMesh"));
	BallerinaMesh->SetupAttachment(Root);
	
	// ===== 기본값 초기화 =====
	RotationSpeed = 45.f;								// 초당 회전 45도 회전
	bIsClockworkActive = true;
}

void ABallerina::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsClockworkActive) return;
	
	const FRotator DeltaRot(0.f,DeltaTime * RotationSpeed, 0.f);
														// FRotator는 인자 순서가 pitch(y축), yaw(z축), roll(x축)
														// 제자리 Yaw 축 회전
	AddActorLocalRotation(DeltaRot);
}

void ABallerina::SetClockworkActive(bool bActive)
{
	bIsClockworkActive = bActive;
}

