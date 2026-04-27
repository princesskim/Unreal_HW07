// ClockworkControlled.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ClockworkControlled.generated.h"

UINTERFACE(MinimalAPI)
class UClockworkControlled : public UInterface
{
	GENERATED_BODY()
};


class MYGAME_API IClockworkControlled
{
	GENERATED_BODY()

public:
	virtual void SetClockworkActive(bool bActive) = 0;	// Castle에서 활성제어 결과를 얻어오기
	
};