#pragma once
#include "LightData.generated.h"

USTRUCT(BlueprintType)

struct FLightData {
	GENERATED_USTRUCT_BODY()
		UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LightControl")
		int type = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LightControl")
		int ID = 0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LightControl")
		float intensity = 0.0;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "LightControl")
		FLinearColor color = FLinearColor(0, 0, 0, 0);
	FLightData() {
	}
};
