#pragma once

#include "GameFramework/Actor.h"
#include "Engine.h"
#include "Networking.h"
#include "Serialization/Archive.h"
#include "Json.h"
#include <thread>
#include <functional>
#include <iostream>
#include "LightData.h"
#include "LightDataReceiver.generated.h"

UCLASS()
class LIGHTCONTROL_API ALightDataReceiver : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "LightControl")
		void BPEvent_DataReceived();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		FLightData GetData();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		bool NewData();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		bool StartLightReceiver(const FString& SocketName,
			const int32 Port);

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		int SetLight();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		bool Connected();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		void SendMsg(const int32 ID, FName name, const int32 type, float intensity, FLinearColor color);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void msgRecving();
	void parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	FSocket* ListenSocket;
	std::thread msgRecvThread;
	bool is_running;
	TArray<uint8> RecvBuffer;
	bool connected;
	bool toSet;
	int curType;
	int curID;
	float curIntensity;
	FLinearColor curColor;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
};