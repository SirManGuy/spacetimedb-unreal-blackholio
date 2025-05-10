#pragma once

#include "HAL/Runnable.h"
#include "HAL/Event.h"
#include "StdbTypes.h"
#include "WebSocketsModule.h"
#include "ClientApi/FServerMessage.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformProcess.h"


struct FClientMessage;
/**
 * FStdbClient: Owns the websocket worker and is a preprocessing thread.
 * Receives raw messages from the websocket worker, preprocesses (decompress/deserializes),
 * and enqueues structured messages for the game thread.
 */
class SPACETIMEDB_API FStdbClientBase : public FRunnable, public TSharedFromThis<FStdbClientBase>
{
	
public:
	FStdbClientBase(const FStdbConnectOptions& InOptions,
				const FString& InAuth,
				const FString& InHost,
				const FString& InNameOrAddress,
				EStdbCompression InCompression,
				bool bInLight);
	virtual ~FStdbClientBase();
	
	// FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	void Connect();
	void Shutdown();	
	void FrameTick();
	
	void LegacySubscribe();
	
	DECLARE_DELEGATE_TwoParams(FOnConnect, FStdbIdentity /*Identity*/, FString /*Token*/);
	DECLARE_DELEGATE_OneParam(FOnConnectError, const FString& /*Error*/);
	DECLARE_DELEGATE_OneParam(FOnDisconnect, const FString& /*Error*/);

	FOnConnect OnConnect;
	FOnConnectError OnConnectError;
	FOnDisconnect OnDisconnect;
	
private:
	void DecompressAndDeserialize(const TArray<uint8>& InBytes, const FDateTime& InTimestamp, FServerMessage& OutMessage);
	void HandleProcessedMessage(const TSharedPtr<FServerMessage>& Msg);

	FStdbIdentity Identity;
	const FStdbConnectOptions ConnectOptions;
	const FString AuthToken;
	const FString Host;
	const FString NameOrAddress;
	const EStdbCompression Compression;
	const bool bLightMode;

	FThreadSafeBool bIsConnected = false;
	FThreadSafeBool bStartConnection = false;
	bool bCallbacksInitialized = false;
	FEvent* ConnectEvent = nullptr;
	bool bConnectResult = false;
	FString LastConnectError;
	TSharedPtr<IWebSocket> WS;
	bool ConnectWebSocket();
	void TeardownWebSocket();
	void SetupWebSocketCallbacks();
	
	void HandleRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);
	void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void SendClientMessage(const FClientMessage& ClientMessage) const;
	
	FThreadSafeBool bStop;
	FRunnableThread* Thread;
	FEvent* WakeEvent = nullptr;

	struct FUnprocessedMessage {
		TArray<uint8> Bytes;
		FDateTime Timestamp;
		FUnprocessedMessage() = default;
		FUnprocessedMessage(const TArray<uint8>& InBytes, const FDateTime& InTimestamp)
			: Bytes(InBytes), Timestamp(InTimestamp) {}
	};
	FThreadSafeQueue<FUnprocessedMessage> RawMessageQueue;
	FThreadSafeQueue<TSharedPtr<FServerMessage>> ProcessedMessageQueue;
	FThreadSafeQueue<FClientMessage> ClientMessageQueue;
	
	static inline FString CompressionToString(EStdbCompression Compression)
	{
		switch (Compression)
		{
		case EStdbCompression::None:
			return TEXT("None");
		case EStdbCompression::Gzip:
			return TEXT("Gzip");
		// Add Brotli
		default:
			return TEXT("None");
		}
	}
	
	FString ConnectionIdHex;
	static FString GenerateRandomConnectionId()
	{
		uint8 Bytes[16];
		for (int i = 0; i < 16; ++i)
			Bytes[i] = FMath::Rand() & 0xFF;
		FString HexStr;
		for (int i = 0; i < 16; ++i)
			HexStr += FString::Printf(TEXT("%02x"), Bytes[i]);
		return HexStr;
	}
};