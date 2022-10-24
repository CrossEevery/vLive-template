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
#include "DMXDataRedirector.generated.h"

#define RECV_BUFFER_SIZE 1024*12

UCLASS()
class LIGHTCONTROL_API ADMXDataRedirector : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "LightControl")
		void BPEvent_DataReceived();

	UFUNCTION(BlueprintCallable, Category = "LightControl")
		bool StartDMXDataRedirector(const FString& SocketName, const FString& TcpServerIp, const int32 Port);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	int ConnectTCPServer(FString ip, int32 Port);
	void redirectData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData, int len);
	void dataReceving();
	uint32 base64_decode(const uint8* code, uint32 code_len, uint8* plain);

	FSocket* SocketTCP;
	FSocket* SocketUDP;
	TArray<uint8> RecvBuffer;
	bool is_running;
	FTimespan WaitTime;
	std::thread dataRecvThread;
	ISocketSubsystem* SocketSubsystem;
	std::string uid;

};