#include "FStdbClientBase.h"

#include "LogStdb.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"
#include "HAL/Runnable.h"
#include "HAL/Event.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "StdbTypes.h"
#include "ClientApi/FClientMessage.h"
#include "ClientApi/FServerMessage.h"


static const int32 MAX_MESSAGE_SIZE = 0x4000000; // 64MB
static const double CONNECT_TIMEOUT_S = 10.0;

FStdbClientBase::FStdbClientBase(const FStdbConnectOptions& InOptions,
                         const FString& InAuth,
                         const FString& InHost,
                         const FString& InNameOrAddress,
                         EStdbCompression InCompression,
                         bool bInLight)
	: ConnectOptions(InOptions)
	  , AuthToken(InAuth)
	  , Host(InHost)
	  , NameOrAddress(InNameOrAddress)
	  , Compression(InCompression)
	  , bLightMode(bInLight)
	  , bStop(false)
{
	UE_LOG(LogStdb, Log, TEXT("FStdbClient constructing"));
	ConnectionIdHex = GenerateRandomConnectionId();
	WakeEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

FStdbClientBase::~FStdbClientBase()
{
	UE_LOG(LogStdb, Log, TEXT("FStdbClient destroyed"));
	Shutdown();

	if (Thread)
	{
		//Thread->Kill(true);
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (WakeEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
		WakeEvent = nullptr;
	}
}

bool FStdbClientBase::Init() { return true; }

uint32 FStdbClientBase::Run()
{
	while (!bStop)
	{
		if (!bIsConnected && bStartConnection)
		{
			bIsConnected = ConnectWebSocket();
		}

		FUnprocessedMessage Raw;
		while (RawMessageQueue.Dequeue(Raw))
		{
			//Deserialize / decompress
			TSharedPtr<FServerMessage> Processed = MakeShared<FServerMessage>();
			DecompressAndDeserialize(Raw.Bytes, Raw.Timestamp, *Processed);

			//Enqueue for game thread
			ProcessedMessageQueue.Enqueue(Processed);
		}

		// TODO: Move to a separate thread
		FClientMessage ClientMessage;
		while (ClientMessageQueue.Dequeue(ClientMessage))
		{
			SendClientMessage(ClientMessage);
		}

		WakeEvent->Wait(FTimespan::FromMilliseconds(50));
	}
	return 0;
}

void FStdbClientBase::Stop()
{
	bStop = true;
	if (WakeEvent) WakeEvent->Trigger();
}

void FStdbClientBase::Exit()
{
}


void FStdbClientBase::Connect()
{
	bStop = false;
	FString ThreadName = FString::Printf(TEXT("FStdbClient_%s"), *NameOrAddress);
	Thread = FRunnableThread::Create(this, *ThreadName);

	bStartConnection = true;
}

void FStdbClientBase::Shutdown()
{
	bStop = true;
	if (WakeEvent) WakeEvent->Trigger();

	OnConnect.Unbind();
	OnConnectError.Unbind();
	OnDisconnect.Unbind();

	TeardownWebSocket();
}

void FStdbClientBase::FrameTick()
{
	TSharedPtr<FServerMessage> Msg;
	while (ProcessedMessageQueue.Dequeue(Msg))
	{
		HandleProcessedMessage(Msg);
	}
}

void FStdbClientBase::LegacySubscribe()
{
	if (!WS->IsConnected())
	{
		UE_LOG(LogStdb, Error, TEXT("Cannot subscribe, not connected to server!"));
		return;
	}

	// TODO: Add subscription handles
	FSubscribeData SubscribeData = FSubscribeData({TEXT("SELECT * FROM *")}, 1);
	FClientMessage Message = FClientMessage::Subscribe(SubscribeData);
	ClientMessageQueue.Enqueue(Message);
	WakeEvent->Trigger();
}

void FStdbClientBase::HandleProcessedMessage(const TSharedPtr<FServerMessage>& Msg)
{
	UE_LOG(LogStdb, Log, TEXT("FStdbClient - Handle Processed Message"));
	switch (Msg->Type)
	{
	case EServerMessageType::IdentityToken:
		{
			FIdentityTokenData identityToken = Msg->Data.Get<FIdentityTokenData>();
			UE_LOG(LogStdb, Log, TEXT("FStdbClient - Handle IdentityToken with auth: %s"), *identityToken.Token);
			this->Identity = identityToken.Identity;
			OnConnect.ExecuteIfBound(identityToken.Identity, identityToken.Token);
			break;
		}
	case EServerMessageType::InitialSubscription:
		{
			FInitialSubscriptionData initialSubscription = Msg->Data.Get<FInitialSubscriptionData>();
			UE_LOG(LogStdb, Log, TEXT("Processing Initial Subscription"))
			for (FTableUpdate TableUpdate : initialSubscription.DatabaseUpdate.Tables)
			{
				UE_LOG(LogStdb, Log, TEXT("   Table Update: %s: %llu"), *TableUpdate.TableName, TableUpdate.NumRows);
			}
			break;
		}
		
	default:
		break;
	}
}


bool FStdbClientBase::ConnectWebSocket()
{
	bStartConnection = false;
	FString URL = FString::Printf(
		TEXT("%s/v1/database/%s/subscribe?connection_id=%s&compression=%s"),
		*Host,
		*NameOrAddress,
		*ConnectionIdHex,
		*CompressionToString(Compression)
	);
	if (bLightMode)
	{
		URL += TEXT("&light=true");
	}

	TMap<FString, FString> UpgradeHeaders;
	if (!AuthToken.IsEmpty())
	{
		UpgradeHeaders.Add(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}

	// Create the socket
	WS = FWebSocketsModule::Get().CreateWebSocket(URL, ConnectOptions.Protocol, UpgradeHeaders);
	if (!WS.IsValid())
	{
		LastConnectError = TEXT("Failed to create WebSocket");
		return false;
	}

	// SYNCHRONOUS handshake: wait OnConnected or OnConnectionError
	ConnectEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);

	WS->OnConnected().AddLambda([this]()
	{
		bConnectResult = true;
		UE_LOG(LogStdb, Log, TEXT("Connected"));
		if (ConnectEvent)
		{
			ConnectEvent->Trigger();
		}
	});
	// On failure
	WS->OnConnectionError().AddLambda([this](const FString& Err)
	{
		bConnectResult = false;
		UE_LOG(LogStdb, Error, TEXT("OnConnectionError"));
		LastConnectError = Err;
		if (ConnectEvent)
		{
			ConnectEvent->Trigger();
		}
	});
	// MUST set up the callbacks before Connect() as LwsWebSocket caches booleans to track this
	SetupWebSocketCallbacks();
	
	WS->Connect();

	// Wait up to timeout
	bool bGot = ConnectEvent->Wait(FTimespan::FromSeconds(CONNECT_TIMEOUT_S));
	FGenericPlatformProcess::ReturnSynchEventToPool(ConnectEvent);
	ConnectEvent = nullptr;

	if (!bGot || !bConnectResult)
	{
		if (LastConnectError.IsEmpty())
			LastConnectError = TEXT("WebSocket connect timed out");
		return false;
	}
	return true;
}

void FStdbClientBase::TeardownWebSocket()
{
	if (WS.IsValid())
	{
		// Remove all delegates to prevent late/dangling calls
		WS->OnRawMessage().RemoveAll(this);
		WS->OnClosed().RemoveAll(this);
		WS->OnConnected().RemoveAll(this);
		WS->OnConnectionError().RemoveAll(this);
		bCallbacksInitialized = false;
		WS->Close();
		WS.Reset();
	}
}

void FStdbClientBase::SetupWebSocketCallbacks()
{
	if (bCallbacksInitialized)
		return;
	
	// Incoming message (binary or text)
	WS->OnRawMessage().AddRaw(this, &FStdbClientBase::HandleRawMessage);

	// Closed (clean or error)
	WS->OnClosed().AddRaw(this, &FStdbClientBase::HandleClosed);
	bCallbacksInitialized = true;
}

void FStdbClientBase::HandleRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	UE_LOG(LogStdb, Log, TEXT("FStdbClient - Handle Raw Message"));
	if (bStop)
		return;

	// Enforce 64MB cap
	if (Size > MAX_MESSAGE_SIZE)
	{
		// close with “too big”
		WS->Close(1013, TEXT("Message too big")); // 1013: Too big
		return;
	}

	// Add to Unprocessed Queue
	FUnprocessedMessage unprocessedMessage;
	unprocessedMessage.Timestamp = FDateTime::UtcNow();
	unprocessedMessage.Bytes.Append(reinterpret_cast<const uint8*>(Data), Size);
	RawMessageQueue.Enqueue(unprocessedMessage);
	if (WakeEvent) WakeEvent->Trigger();
}

void FStdbClientBase::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	if (bStop)
		return;

	// Build a message
	FString Msg = bWasClean
		              ? FString()
		              : FString::Printf(TEXT("WebSocket closed: %s"), *Reason);
	// Do we need to do anything here?

	//OnDisconnect.ExecuteIfBound(Msg);
}

void FStdbClientBase::SendClientMessage(const FClientMessage& ClientMessage) const
{
	// TODO: Add compression
	FBinaryWriter writer;
	FClientMessage::Serialize(ClientMessage, writer);
	const TArray<uint8>& Data = writer.GetData();
	// LwsWebSocket is threaded and has an internal queue that is sent FIFO
	WS->Send(Data.GetData(), Data.Num(), true);
}

void FStdbClientBase::DecompressAndDeserialize(const TArray<uint8>& InBytes, const FDateTime& InTimestamp,
                                           FServerMessage& OutMessage)
{
	UE_LOG(LogStdb, Log, TEXT("DecompressAndDeserialize"));
	if (InBytes.Num() < 1)
	{
		UE_LOG(LogStdb, Error, TEXT("Empty message received"));
		return;
	}
	uint8 CompressionByte = InBytes[0];
	EStdbCompression Algo = static_cast<EStdbCompression>(CompressionByte);
	TArray<uint8> CompressedData;
	CompressedData.Append(InBytes.GetData() + 1, InBytes.Num() - 1);
	
	TArray<uint8> WorkingBuffer;
	switch (Compression)
	{
	case EStdbCompression::None:
		// No compression: just copy
		WorkingBuffer = CompressedData;
		break;

	// TODO: Repair this.. it seems that Zlib isn't working with Gzip or the data from SpacetimeDb isn't Gzip'd
	case EStdbCompression::Gzip:
		{
			// Pre‑allocate an approximate size (you may adjust this)
			int32 UncompressedSize = CompressedData.Num() * 4;
			WorkingBuffer.SetNum(UncompressedSize);

			if (!FCompression::UncompressMemory(
				NAME_Zlib,
				WorkingBuffer.GetData(),
				UncompressedSize,
				CompressedData.GetData(),
				CompressedData.Num()
			))
			{
				UE_LOG(LogStdb, Error, TEXT("Gzip decompression failed"));
				WorkingBuffer.Reset();
			}
			else
			{
				// Shrink to actual size
				WorkingBuffer.SetNum(UncompressedSize, /* bAllowShrinking = */ true);
			}
		}
		break;

	default:
		// Unknown compression type
		UE_LOG(LogStdb, Warning, TEXT("Unsupported compression mode; passing through raw bytes"));
		WorkingBuffer = CompressedData;
		break;
	}

	// No data no message
	if (WorkingBuffer.Num() <= 0)
		return;

	FBinaryReader reader = FBinaryReader(WorkingBuffer.GetData(), WorkingBuffer.Num());
	OutMessage = FServerMessage::Deserialize(reader);
}
