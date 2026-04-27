// SwanPath.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwanPath.generated.h"

class USplineComponent;

UCLASS()
class MYGAME_API ASwanPath : public AActor
{
	GENERATED_BODY()
	
public:	
	ASwanPath();
	
	// ===== 스플라인 컴포넌트 (백조가 따라가는 경로) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TObjectPtr<USplineComponent> Spline;
												// VisibleAnywhere vs EditAnywhere
												// 변수가 가리키는 컴포넌트는 교체할 일이 없기 때문에 VisibleAnywhere로 잠그는 것이 관행
												// 에디터에서 포인트 편집 가능!
	
};
