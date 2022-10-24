#pragma once
#include "DSControlData.generated.h"

USTRUCT(BlueprintType)

struct FDSData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "DSControl")
		int ID = 0;

	FDSData() {
	}
};