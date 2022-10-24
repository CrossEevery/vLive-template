#include "LightDataReceiver.h"
ALightDataReceiver::ALightDataReceiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ListenSocket = NULL;
	is_running = true;
	RecvBuffer.SetNumUninitialized(1024);
	connected = false;
	toSet = false;
}

bool ALightDataReceiver::StartLightReceiver(const FString& SocketName, const int32 Port)
{
	FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	int32 BufferSize = 2 * 1024 * 1024;
	ListenSocket = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	msgRecvThread = std::thread(std::bind(&ALightDataReceiver::msgRecving, this));
	return true;
}

void ALightDataReceiver::msgRecving()
{
	//TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	while (is_running)
	{
		int32 Read = 0;
		ListenSocket->RecvFrom(RecvBuffer.GetData(), RecvBuffer.Num(), Read, *Sender);
		//ListenSocket->Recv(RecvBuffer.GetData(), RecvBuffer.Num(), Read);
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

void ALightDataReceiver::parseMsg(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
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
		int type;
		int ID;
		double intensity;
		double color[4];
		//int value;
		//double speed;
		const TArray<TSharedPtr<FJsonValue>>* tmp;
		if (JsonObject->TryGetNumberField(TEXT("LIGHT-TYPE"), type))
		{
			curType = type;
		}
		if (JsonObject->TryGetNumberField(TEXT("LIGHT-ID"), ID))
		{
			curID = ID;
		}
		if (JsonObject->TryGetNumberField(TEXT("Intensity"), intensity))
		{
			curIntensity = intensity;
		}
		if (JsonObject->TryGetArrayField(TEXT("Color"), tmp))
		{
			TArray<TSharedPtr<FJsonValue>> tmpColor = JsonObject->GetArrayField(TEXT("Color"));
			color[0] = tmpColor[0]->AsNumber();
			color[1] = tmpColor[1]->AsNumber();
			color[2] = tmpColor[2]->AsNumber();
			color[3] = tmpColor[3]->AsNumber();

			FLinearColor linearColor(color[0], color[1], color[2], color[3]);

			curColor = linearColor;
		}
		toSet = true;
	}
}

FLightData ALightDataReceiver::GetData()
{
	FLightData data;
	data.ID = curID;
	data.type = curType;
	data.intensity = curIntensity;
	data.color = curColor;
	toSet = false;
	return data;
}

bool ALightDataReceiver::NewData()
{
	return toSet;
}

int ALightDataReceiver::SetLight()
{
	return 0;// Light_type;
}

bool ALightDataReceiver::Connected()
{
	return connected;
}

void ALightDataReceiver::SendMsg(const int32 ID, FName name, const int32 type, float intensity, FLinearColor color)
{
	if (connected)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("connected and sending: ID: %d, name: %s, type: %d"), ID, *name.ToString(), type));
		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		jsonObject->SetNumberField(TEXT("LIGHT-TYPE"), type);
		jsonObject->SetStringField(TEXT("LIGHT-Name"), TCHAR_TO_UTF8(*name.ToString()));
		jsonObject->SetNumberField(TEXT("LIGHT-ID"), ID);
		jsonObject->SetNumberField(TEXT("Intensity"), intensity);

		TArray<TSharedPtr<FJsonValue>> tmpColor;

		TSharedPtr<FJsonValueNumber> t_value;
		t_value = MakeShareable(new FJsonValueNumber(color.R));
		tmpColor.Add(t_value);
		t_value = MakeShareable(new FJsonValueNumber(color.G));
		tmpColor.Add(t_value);
		t_value = MakeShareable(new FJsonValueNumber(color.B));
		tmpColor.Add(t_value);
		t_value = MakeShareable(new FJsonValueNumber(color.A));
		tmpColor.Add(t_value);

		jsonObject->SetArrayField(TEXT("Color"), tmpColor);

		FString jsonStr = TEXT("");
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonStr);
		FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, jsonStr);
		int len = jsonStr.Len();
		int32 sent = 0;
		TCHAR* SendMessage = jsonStr.GetCharArray().GetData();
		int ret = ListenSocket->SendTo((uint8*)TCHAR_TO_UTF8(SendMessage), len, sent, *Sender);
		uint32 out; 
		Sender->GetIp(out);
		int port = Sender->GetPort();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("send to ip: %d, port %d, ret: %d, "), out, port, ret));
	}
}

void ALightDataReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
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