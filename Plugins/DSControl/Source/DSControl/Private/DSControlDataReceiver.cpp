#include "DSControlDataReceiver.h"
ADSControlDataReceiver::ADSControlDataReceiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ListenSocket = NULL;
	is_running = true;
	RecvBuffer.SetNumUninitialized(1024);
	connected = false;
	toSet = false;
	curID = 0;
	
}

bool ADSControlDataReceiver::StartDSControlReceiver(const FString& SocketName, const int32 Port)
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	int32 BufferSize = 2 * 1024 * 1024;
	ListenSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	msgRecvThread = std::thread(std::bind(&ADSControlDataReceiver::msgRecving, this));
	return true;
}

void ADSControlDataReceiver::msgRecving()
{
	while (is_running)
	{
		int32 Read = 0;
		ListenSocket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender);
		if (Read > 0)
		{
			connected = true;
			TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
			ReceivedData->SetNumUninitialized(Read);
			memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
			parseMsg(ReceivedData);

		}
	}
}

void ADSControlDataReceiver::parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
	FString JsonString;
	JsonString.Empty(ReceivedData->Num());
	for (uint8& Byte : *ReceivedData.Get())
	{
		JsonString += TCHAR(Byte);
	}
	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, JsonString);
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool flag = FJsonSerializer::Deserialize(Reader, JsonObject);
	if (flag)  // StringתJSON
	{
		int ID;
		if (JsonObject->TryGetNumberField(TEXT("DS-SCENE"), ID))
		{
			curID = ID;
		}
		toSet = true;
	}
}

FDSData ADSControlDataReceiver::GetData()
{
	FDSData data;
	data.ID = curID;
	toSet = false;
	return data;
}

bool ADSControlDataReceiver::NewData()
{
	return toSet;
}

void ADSControlDataReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	is_running = false;
	msgRecvThread.join();

	//Clear all sockets
	if (ListenSocket)
	{
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}
}