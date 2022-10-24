#include "DMXDataRedirector.h"
#include <string>
#include<sstream>
#include <fstream> 
#include <iostream>

static uint8 reverse_map[] =
{
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
     255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
     255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
};

ADMXDataRedirector::ADMXDataRedirector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SocketTCP = NULL;
	SocketUDP = NULL;
	RecvBuffer.SetNumUninitialized(RECV_BUFFER_SIZE);
	is_running = true;
	WaitTime = FTimespan::FromMilliseconds(100);
	SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	FString configDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir());
	std::string path(TCHAR_TO_UTF8(*configDir));
	std::string uidFile = path + "uid.txt";
	std::ifstream cfile(uidFile);
	getline(cfile, uid);
	cfile.close();
}

void ADMXDataRedirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	is_running = false;

	dataRecvThread.join();

	if (SocketTCP)
	{
		SocketTCP->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SocketTCP);
	}
}


bool ADMXDataRedirector::StartDMXDataRedirector(const FString& SocketName, const FString& TcpServerIp, const int32 TcpPort)
{
	//FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
	int32 BufferSize = RECV_BUFFER_SIZE;

	SocketUDP = FUdpSocketBuilder(*SocketName).AsNonBlocking()
		.AsReusable()
		.AsBlocking()
		.WithReceiveBufferSize(BufferSize);

	ConnectTCPServer(TcpServerIp, TcpPort);
	//ConnectTCPServer("106.55.47.254",9094);

	dataRecvThread = std::thread(std::bind(&ADMXDataRedirector::dataReceving, this));
	
	return true;
}

int ADMXDataRedirector::ConnectTCPServer(FString ip, int32 Port)
{
	FIPv4Endpoint ServerEndpoint;
	FIPv4Endpoint::Parse(ip, ServerEndpoint);
	TSharedPtr<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();
	bool Success = true;
	addr->SetIp(*ip, Success);
	if (!Success)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("DMX fail"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("DMX success"));
	}

	addr->SetPort(Port);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ip);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%d"), Port));
	SocketTCP = FTcpSocketBuilder(TEXT("Client DMX Socket"))
		.AsReusable()
		.AsBlocking()
		.WithReceiveBufferSize(RECV_BUFFER_SIZE)
		.WithSendBufferSize(RECV_BUFFER_SIZE);

	uint8* sendData = (uint8*)malloc(sizeof(uint8) * 1024);
	std::string str = "RECV:" + uid;
	FString fstr(str.c_str());
	memcpy(sendData, str.c_str(), str.length());
	GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, fstr);

	if (SocketTCP->Connect(*addr))
	{
		uint32 size;
		SocketTCP->HasPendingData(size);
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, TEXT("DMX Client Connect Success"));

		int32 sent = 0;
		int len = str.length();
		if (SocketTCP->Send(sendData, len, sent))
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, fstr);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, TEXT("Send recv msg fail"));
		}
		return 0;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, TEXT("DMX Client Connect Fail"));
		return -1;
	}



	return 0;
}

void ADMXDataRedirector::dataReceving()
{
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	while (is_running)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("Recving body data, size %d"), 0));
		if (SocketTCP->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
		{
			int32 Read = 0;

			SocketTCP->Recv(RecvBuffer.GetData(), RecvBuffer.Num(), Read);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Recving DMX data, size %d"), Read));
			if (Read > 0)
			{
				TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData = MakeShareable(new TArray<uint8>());
				ReceivedData->SetNumUninitialized(Read);
				memcpy(ReceivedData->GetData(), RecvBuffer.GetData(), Read);
				redirectData(ReceivedData, Read);  
			}
		}
	}
}

uint32 ADMXDataRedirector::base64_decode(const uint8* code, uint32 code_len, uint8* plain)
{
    ///assert((code_len & 0x03) == 0);  //如果它的条件返回错误，则终止程序执行。4的倍数。
    uint32 i, j = 0;
    uint8 quad[4];
    for (i = 0; i < code_len; i += 4)
    {
        for (uint32 k = 0; k < 4; k++)
        {
            quad[k] = reverse_map[code[i + k]];//分组，每组四个分别依次转换为base64表内的十进制数
        }

        //assert(quad[0] < 64 && quad[1] < 64);
        plain[j++] = (quad[0] << 2) | (quad[1] >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的前2位进行组合
        if (quad[2] >= 64)
            break;
        else if (quad[3] >= 64)
        {
            plain[j++] = (quad[1] << 4) | (quad[2] >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应base64表的十进制数的前4位进行组合
            break;
        }
        else
        {
            plain[j++] = (quad[1] << 4) | (quad[2] >> 2);
            plain[j++] = (quad[2] << 6) | quad[3];//取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
        }
    }
    return j;
}

void ADMXDataRedirector::redirectData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData, int len)
{
	TSharedPtr<FInternetAddr> RemoteAddr = SocketSubsystem->CreateInternetAddr();
	bool bIsValid;
	FString ip("127.0.0.1");
	RemoteAddr->SetIp(*ip, bIsValid);
	RemoteAddr->SetPort(6454);


	uint8* decodeData = (uint8*)malloc(len*2);
	int size = base64_decode((uint8*)ReceivedData->GetData(), len, decodeData);

	int32 sent = 0;
	int ret = SocketUDP->SendTo(decodeData, size, sent, *RemoteAddr);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Send DMX data with size %d return %d"), size, ret));
	free(decodeData);
}
