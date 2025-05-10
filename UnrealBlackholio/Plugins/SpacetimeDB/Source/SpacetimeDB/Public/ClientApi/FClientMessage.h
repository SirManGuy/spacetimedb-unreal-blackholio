#pragma once

#include "CoreMinimal.h"
#include "FBinaryReader.h"
#include "FBinaryWriter.h"
#include "FQueryId.h"
#include "LogStdb.h"

UENUM()
enum class EClientMessageType : uint8
{
	CallReducer,
	Subscribe,
	OneOffQuery,
	SubscribeSingle,
	SubscribeMulti,
	Unsubscribe,
	UnsubscribeMulti
};

struct SPACETIMEDB_API FCallReducerData
{
	FString Reducer;
	TArray<uint8> Args;
	uint32 RequestId;
	uint8 Flags;

	FCallReducerData()
		: Reducer("")
		, RequestId(0)
		, Flags(0)
	{
	}

	FCallReducerData(const FString& InReducer, const TArray<uint8>& InArgs, uint32 InRequestId, uint8 InFlags)
		: Reducer(InReducer)
		, Args(InArgs)
		, RequestId(InRequestId)
		, Flags(InFlags)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		Reducer = reader.ReadString();
		Args = reader.ReadArray<uint8>([](FBinaryReader& R) { return R.ReadByte(); });
		RequestId = reader.ReadUInt32();
		Flags = reader.ReadByte();
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteString(Reducer);
		writer.WriteArray<uint8>(Args, [](FBinaryWriter& W, const uint8& V) { W.WriteByte(V); });
		writer.WriteUInt32(RequestId);
		writer.WriteByte(Flags);
	}
};

struct SPACETIMEDB_API FSubscribeData
{
	TArray<FString> QueryStrings;
	uint32 RequestId;

	void ReadFields(FBinaryReader& reader)
	{
		QueryStrings = reader.ReadStringArray();
		RequestId = reader.ReadUInt32();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteStringArray(QueryStrings);		
		writer.WriteUInt32(RequestId);
	}
};

struct SPACETIMEDB_API FOneOffQueryData
{
	TArray<uint8> MessageId;
	FString QueryString;

	FOneOffQueryData()
		: QueryString("")
	{
	}

	FOneOffQueryData(const TArray<uint8>& InMessageId, const FString& InQueryString)
		: MessageId(InMessageId)
		, QueryString(InQueryString)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		MessageId = reader.ReadArray<uint8>([](FBinaryReader& R) { return R.ReadByte(); });
		QueryString = reader.ReadString();
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteArray<uint8>(MessageId, [](FBinaryWriter& W, const uint8& V) { W.WriteByte(V); });
		writer.WriteString(QueryString);
	}
};

struct SPACETIMEDB_API FSubscribeSingleData
{
	FString Query;
	uint32 RequestId;
	FQueryId QueryId;

	FSubscribeSingleData()
		: Query("")
		, RequestId(0)
		, QueryId()
	{
	}

	FSubscribeSingleData(const FString& InQuery, uint32 InRequestId, const FQueryId& InQueryId)
		: Query(InQuery)
		, RequestId(InRequestId)
		, QueryId(InQueryId)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		Query = reader.ReadString();
		RequestId = reader.ReadUInt32();
		QueryId.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteString(Query);
		writer.WriteUInt32(RequestId);
		QueryId.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FSubscribeMultiData
{
	TArray<FString> QueryStrings;
	uint32 RequestId;
	FQueryId QueryId;

	FSubscribeMultiData()
		: RequestId(0)
		, QueryId()
	{
	}

	FSubscribeMultiData(const TArray<FString>& InQueryStrings, uint32 InRequestId, const FQueryId& InQueryId)
		: QueryStrings(InQueryStrings)
		, RequestId(InRequestId)
		, QueryId(InQueryId)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		QueryStrings = reader.ReadArray<FString>([](FBinaryReader& R) { return R.ReadString(); });
		RequestId = reader.ReadUInt32();
		QueryId.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteArray<FString>(QueryStrings, [](FBinaryWriter& W, const FString& V) { W.WriteString(V); });
		writer.WriteUInt32(RequestId);
		QueryId.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FUnsubscribeData
{
	uint32 RequestId;
	FQueryId QueryId;

	FUnsubscribeData()
		: RequestId(0)
		, QueryId()
	{
	}

	FUnsubscribeData(uint32 InRequestId, const FQueryId& InQueryId)
		: RequestId(InRequestId)
		, QueryId(InQueryId)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		RequestId = reader.ReadUInt32();
		QueryId.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteUInt32(RequestId);
		QueryId.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FUnsubscribeMultiData
{
	uint32 RequestId;
	FQueryId QueryId;

	FUnsubscribeMultiData()
		: RequestId(0)
		, QueryId()
	{
	}

	FUnsubscribeMultiData(uint32 InRequestId, const FQueryId& InQueryId)
		: RequestId(InRequestId)
		, QueryId(InQueryId)
	{
	}

	void ReadFields(FBinaryReader reader)
	{
		RequestId = reader.ReadUInt32();
		QueryId.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter writer) const
	{
		writer.WriteUInt32(RequestId);
		QueryId.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FClientMessage
{
	EClientMessageType Type;
	TVariant<
		FCallReducerData,
		FSubscribeData,
		FOneOffQueryData,
		FSubscribeSingleData,
		FSubscribeMultiData,
		FUnsubscribeData,
		FUnsubscribeMultiData> Data;
 
	static FClientMessage Deserialize(FBinaryReader& reader)
	{
		FClientMessage result;
		uint8 messageType = reader.ReadByte();
		result.Type = static_cast<EClientMessageType>(messageType);
		
		switch (result.Type)
		{
		case EClientMessageType::Subscribe:
			{
				FSubscribeData Subscribe;
				Subscribe.ReadFields(reader);
				result.Data.Emplace<FSubscribeData>(MoveTemp(Subscribe));
				break;
			}
		case EClientMessageType::CallReducer:
			{
				FCallReducerData CallReducer;
				CallReducer.ReadFields(reader);
				result.Data.Emplace<FCallReducerData>(MoveTemp(CallReducer));
				break;
			}
		case EClientMessageType::OneOffQuery:
			{
				FOneOffQueryData OneOffQuery;
				OneOffQuery.ReadFields(reader);
				result.Data.Emplace<FOneOffQueryData>(MoveTemp(OneOffQuery));
				break;
			}
		case EClientMessageType::SubscribeSingle:
			{
				FSubscribeSingleData SubscribeSingle;
				SubscribeSingle.ReadFields(reader);
				result.Data.Emplace<FSubscribeSingleData>(MoveTemp(SubscribeSingle));
				break;
			}
		case EClientMessageType::SubscribeMulti:
			{
				FSubscribeMultiData SubscribeMulti;
				SubscribeMulti.ReadFields(reader);
				result.Data.Emplace<FSubscribeMultiData>(MoveTemp(SubscribeMulti));
				break;
			}
		case EClientMessageType::Unsubscribe:
			{
				FUnsubscribeData Unsubscribe;
				Unsubscribe.ReadFields(reader);
				result.Data.Emplace<FUnsubscribeData>(MoveTemp(Unsubscribe));
				break;
			}
		case EClientMessageType::UnsubscribeMulti:
			{
				FUnsubscribeMultiData UnsubscribeMulti;
				UnsubscribeMulti.ReadFields(reader);
				result.Data.Emplace<FUnsubscribeMultiData>(MoveTemp(UnsubscribeMulti));
				break;
			}
		default:
			UE_LOG(LogStdb, Warning, TEXT("Unknown client message type: %d"), static_cast<int32>(result.Type));
			break;
		}
		return result;		
	}

	static void Serialize(FClientMessage msg, FBinaryWriter& writer)
	{
		writer.WriteByte(static_cast<uint8>(msg.Type));
		switch (msg.Type)
		{
		case EClientMessageType::Subscribe:
			{
				FSubscribeData Subscribe = msg.Data.Get<FSubscribeData>();
				Subscribe.WriteFields(writer);
				break;
			}
		case EClientMessageType::CallReducer:
			{
				const FCallReducerData& CallReducer = msg.Data.Get<FCallReducerData>();
				CallReducer.WriteFields(writer);
				break;
			}
		case EClientMessageType::OneOffQuery:
			{
				const FOneOffQueryData& OneOffQuery = msg.Data.Get<FOneOffQueryData>();
				OneOffQuery.WriteFields(writer);
				break;
			}
		case EClientMessageType::SubscribeSingle:
			{
				const FSubscribeSingleData& SubscribeSingle = msg.Data.Get<FSubscribeSingleData>();
				SubscribeSingle.WriteFields(writer);
				break;
			}
		case EClientMessageType::SubscribeMulti:
			{
				const FSubscribeMultiData& SubscribeMulti = msg.Data.Get<FSubscribeMultiData>();
				SubscribeMulti.WriteFields(writer);
				break;
			}
		case EClientMessageType::Unsubscribe:
			{
				const FUnsubscribeData& Unsubscribe = msg.Data.Get<FUnsubscribeData>();
				Unsubscribe.WriteFields(writer);
				break;
			}
		case EClientMessageType::UnsubscribeMulti:
			{
				const FUnsubscribeMultiData& UnsubscribeMulti = msg.Data.Get<FUnsubscribeMultiData>();
				UnsubscribeMulti.WriteFields(writer);
				break;
			}
		default:
			UE_LOG(LogStdb, Warning, TEXT("Cannot serialize unknown client message type: %d"), static_cast<int32>(msg.Type));
			break;
		}
	}

	static FClientMessage Subscribe(FSubscribeData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::Subscribe;
		Message.Data.Emplace<FSubscribeData>(MoveTemp(data));
		return Message;
	}
	static FClientMessage CallReducer(const FCallReducerData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::CallReducer;
		Message.Data.Emplace<FCallReducerData>(data);
		return Message;
	}

	static FClientMessage OneOffQuery(const FOneOffQueryData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::OneOffQuery;
		Message.Data.Emplace<FOneOffQueryData>(data);
		return Message;
	}

	static FClientMessage SubscribeSingle(const FSubscribeSingleData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::SubscribeSingle;
		Message.Data.Emplace<FSubscribeSingleData>(data);
		return Message;
	}

	static FClientMessage SubscribeMulti(const FSubscribeMultiData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::SubscribeMulti;
		Message.Data.Emplace<FSubscribeMultiData>(data);
		return Message;
	}

	static FClientMessage Unsubscribe(const FUnsubscribeData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::Unsubscribe;
		Message.Data.Emplace<FUnsubscribeData>(data);
		return Message;
	}

	static FClientMessage UnsubscribeMulti(const FUnsubscribeMultiData& data)
	{
		FClientMessage Message;
		Message.Type = EClientMessageType::UnsubscribeMulti;
		Message.Data.Emplace<FUnsubscribeMultiData>(data);
		return Message;
	}
};
