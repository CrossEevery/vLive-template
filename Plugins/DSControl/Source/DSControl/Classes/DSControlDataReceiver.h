#pragma once

#include "GameFramework/Actor.h"
#include "Engine.h"
#include "Networking.h"
#include "Serialization/Archive.h"
#include "Json.h"
#include <thread>
#include <functional>
#include <iostream>
#include "DSControlData.h"
#include "DSControlDataReceiver.generated.h"

UCLASS()
class DSCONTROL_API ADSControlDataReceiver : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "DSControl")
		void BPEvent_DataReceived();

	UFUNCTION(BlueprintCallable, Category = "DSControl")
		FDSData GetData();

	UFUNCTION(BlueprintCallable, Category = "DSControl")
		bool NewData();

	UFUNCTION(BlueprintCallable, Category = "DSControl")
		bool StartDSControlReceiver(const FString& SocketName,
			const int32 Port);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void msgRecving();
	void parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData);

	FSocket* ListenSocket;
	std::thread msgRecvThread;
	bool is_running;
	TArray<uint8> RecvBuffer;
	bool connected;
	bool toSet;

	int curID;

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
};