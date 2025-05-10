#pragma once

#include "CoreMinimal.h"
#include "FBinaryReader.h"
#include "FBinaryWriter.h"
#include "FStdbIdentity.h"
#include "FTimeDuration.h"
#include "FTimestamp.h"
#include "FStdbConnectionId.h"
#include "LogStdb.h"
#include "FQueryId.h"

struct FIdentityToken;

UENUM()
enum class EServerMessageType : uint8
{
	InitialSubscription,
	TransactionUpdate,
	TransactionUpdateLight,
	IdentityToken,
	OneOffQueryResponse,
	SubscribeApplied,
	UnsubscribeApplied,
	SubscriptionError,
	SubscribeMultiApplied,
	UnsubscribeMultiApplied
};

struct SPACETIMEDB_API RowSizeHint
{
	enum class EHintType : uint8
	{
		FixedSize,
		RowOffsets
	};

	RowSizeHint() : Type(EHintType::FixedSize)
	{
	}

	RowSizeHint(EHintType InType) : Type(InType)
	{
	}

	EHintType Type = EHintType::FixedSize;

	TVariant<
		uint16, // FixedSize,
		TArray<uint64> // RowOffsets
	> SizeHint;

	void ReadFields(FBinaryReader& reader)
	{
		uint8 messageType = reader.ReadByte();
		Type = static_cast<EHintType>(messageType);
		switch (Type)
		{
		case EHintType::FixedSize:
		default:
			{
				uint16 Data = reader.ReadUInt16();
				SizeHint.Emplace<uint16>(MoveTemp(Data));
				break;
			}
		case EHintType::RowOffsets:
			{
				TArray<uint64> Bytes = reader.ReadArray<uint64>(
					[](FBinaryReader& R)
					{
						return R.ReadUInt64();
					}
				);
				SizeHint.Emplace<TArray<uint64>>(MoveTemp(Bytes));
				break;
			}
		}
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteByte(static_cast<uint8>(Type));
		switch (Type)
		{
		case EHintType::FixedSize:
			{
				const uint16& Value = SizeHint.Get<uint16>();
				writer.WriteUInt16(Value);
				break;
			}
		case EHintType::RowOffsets:
			{
				const TArray<uint64>& Offsets = SizeHint.Get<TArray<uint64>>();
				writer.WriteArray<uint64>(Offsets, [](FBinaryWriter& W, const uint64& V)
				{
					W.WriteUInt64(V);
				});
				break;
			}
		}
	}
};

struct SPACETIMEDB_API FBsatnRowList
{
	RowSizeHint SizeHint;
	TArray<uint8> RowsData;

	void ReadFields(FBinaryReader& reader)
	{
		SizeHint.ReadFields(reader);
		RowsData = reader.ReadArray<uint8>(
			[](FBinaryReader& R)
			{
				return R.ReadByte();
			}
		);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		SizeHint.WriteFields(writer);
		writer.WriteArray<uint8>(RowsData,
		                         [](FBinaryWriter& W, const uint8& V)
		                         {
			                         return W.WriteByte(V);
		                         });
	}
};

struct SPACETIMEDB_API FQueryUpdate
{
	FBsatnRowList Deletes;
	FBsatnRowList Inserts;

	void ReadFields(FBinaryReader& reader)
	{
		Deletes.ReadFields(reader);
		Inserts.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		Deletes.WriteFields(writer);
		Inserts.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FCompressableQueryUpdate
{
	enum class ECompressionType : uint8
	{
		Uncompressed,
		Brotli,
		Gzip
	};

	ECompressionType Type;

	TVariant<
		FQueryUpdate, // Uncompressed,
		TArray<uint8> // Brotli/Gzip,
	> Data;

	void ReadFields(FBinaryReader& reader)
	{
		uint8 messageType = reader.ReadByte();
		Type = static_cast<ECompressionType>(messageType);
		switch (Type)
		{
		case ECompressionType::Uncompressed:
		default:
			{
				FQueryUpdate Query;
				Query.ReadFields(reader);
				Data.Emplace<FQueryUpdate>(MoveTemp(Query));
				break;
			}
		case ECompressionType::Brotli:
		case ECompressionType::Gzip:
			{
				TArray<uint8> Bytes = reader.ReadArray<uint8>(
					[](FBinaryReader& R)
					{
						return R.ReadByte();
					}
				);
				Data.Emplace<TArray<uint8>>(MoveTemp(Bytes));
				break;
			}
		}
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		// Write the compression type byte first
		writer.WriteByte(static_cast<uint8>(Type));

		switch (Type)
		{
		case ECompressionType::Uncompressed:
		default:
			{
				const FQueryUpdate& Query = Data.Get<FQueryUpdate>();
				Query.WriteFields(writer); // assumes this method exists
				break;
			}
		case ECompressionType::Brotli:
		case ECompressionType::Gzip:
			{
				const TArray<uint8>& Compressed = Data.Get<TArray<uint8>>();
				writer.WriteArray<uint8>(Compressed, [](FBinaryWriter& W, const uint8& V)
				{
					W.WriteByte(V);
				});
				break;
			}
		}
	}
};

struct SPACETIMEDB_API FTableUpdate
{
	uint32 TableId;
	FString TableName;
	uint64 NumRows;
	TArray<FCompressableQueryUpdate> Updates;

	void ReadFields(FBinaryReader& reader)
	{
		TableId = reader.ReadUInt32();
		TableName = reader.ReadString();
		NumRows = reader.ReadUInt64();
		Updates = reader.ReadArray<FCompressableQueryUpdate>(
			[](FBinaryReader& R)
			{
				FCompressableQueryUpdate Update;
				Update.ReadFields(R);
				return Update;
			}
		);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(TableId);
		writer.WriteString(TableName);
		writer.WriteUInt64(NumRows);
		writer.WriteArray<FCompressableQueryUpdate>(Updates,
		                                            [](FBinaryWriter& W, const FCompressableQueryUpdate& Update)
		                                            {
			                                            Update.WriteFields(W); // Calls each update's own writer logic
		                                            }
		);
	}
};

struct SPACETIMEDB_API FDatabaseUpdate
{
	TArray<FTableUpdate> Tables;

	void ReadFields(FBinaryReader& reader)
	{
		Tables = reader.ReadArray<FTableUpdate>(
			[](FBinaryReader& R)
			{
				FTableUpdate Table;
				Table.ReadFields(R);
				return Table;
			}
		);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteArray<FTableUpdate>(Tables,
		                                [](FBinaryWriter& W, const FTableUpdate& Table)
		                                {
			                                Table.WriteFields(W);
		                                }
		);
	}
};

struct SPACETIMEDB_API FInitialSubscriptionData
{
	FDatabaseUpdate DatabaseUpdate;
	uint32 RequestId;
	FTimeDuration TotalHostExecutionDuration;

	void ReadFields(FBinaryReader& reader)
	{
		DatabaseUpdate.ReadFields(reader);
		RequestId = reader.ReadUInt32();
		TotalHostExecutionDuration = reader.ReadTimeDuration();
	}

	void WriteField(FBinaryWriter writer) const
	{
		DatabaseUpdate.WriteFields(writer);
		writer.WriteUInt32(RequestId);
		writer.WriteTimeDuration(TotalHostExecutionDuration);
	}
};

struct SPACETIMEDB_API FStdbUnit
{
	void ReadFields(FBinaryReader& reader)
	{
	}

	void WriteFields(FBinaryWriter& writer) const
	{
	}
};

struct SPACETIMEDB_API FUpdateStatus
{
	enum class EStatusType : uint8
	{
		Committed,
		Failed,
		OutOfEnergy
	};

	EStatusType Type;

	TVariant<
		FDatabaseUpdate, // Committed,
		FString, // Failed
		FStdbUnit // OutOfEnergy
	> Data;

	void ReadFields(FBinaryReader& reader)
	{
		uint8 messageType = reader.ReadByte();
		Type = static_cast<EStatusType>(messageType);
		switch (Type)
		{
		case EStatusType::Committed:
			{
				FDatabaseUpdate DatabaseUpdate;
				DatabaseUpdate.ReadFields(reader);
				Data.Emplace<FDatabaseUpdate>(MoveTemp(DatabaseUpdate));
				break;
			}
		default:
		case EStatusType::Failed:
			{
				FString Message = reader.ReadString();
				Data.Emplace<FString>(MoveTemp(Message));
				break;
			}
		case EStatusType::OutOfEnergy:
			{
				FStdbUnit OutOfEnergy;
				OutOfEnergy.ReadFields(reader);
				Data.Emplace<FStdbUnit>(MoveTemp(OutOfEnergy));
				break;
			}
		}
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteByte(static_cast<uint8>(Type));
		switch (Type)
		{
		case EStatusType::Committed:
			{
				const FDatabaseUpdate& DatabaseUpdate = Data.Get<FDatabaseUpdate>();
				DatabaseUpdate.WriteFields(writer);
				break;
			}
		case EStatusType::Failed:
			{
				const FString& Message = Data.Get<FString>();
				writer.WriteString(Message);
				break;
			}
		case EStatusType::OutOfEnergy:
			{
				const FStdbUnit& OutOfEnergy = Data.Get<FStdbUnit>();
				OutOfEnergy.WriteFields(writer);
				break;
			}
		default:
			break;
		}
	}
};

struct SPACETIMEDB_API FReducerCallInfo
{
	FString ReducerName;
	uint32 ReducerId;
	TArray<uint8> Args;
	uint32 RequestId;

	FReducerCallInfo()
		: ReducerName("")
		  , ReducerId(0)
		  , RequestId(0)
	{
	}

	FReducerCallInfo(const FString& InReducerName, uint32 InReducerId, const TArray<uint8>& InArgs, uint32 InRequestId)
		: ReducerName(InReducerName)
		  , ReducerId(InReducerId)
		  , Args(InArgs)
		  , RequestId(InRequestId)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		ReducerName = reader.ReadString();
		ReducerId = reader.ReadUInt32();
		Args = reader.ReadArray<uint8>([](FBinaryReader& R) { return R.ReadByte(); });
		RequestId = reader.ReadUInt32();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteString(ReducerName);
		writer.WriteUInt32(ReducerId);
		writer.WriteArray<uint8>(Args, [](FBinaryWriter& W, const uint8& V) { W.WriteByte(V); });
		writer.WriteUInt32(RequestId);
	}
};

struct SPACETIMEDB_API FEnergyQuanta
{
	FU128 Quanta;

	void ReadFields(FBinaryReader& reader)
	{
		Quanta = reader.ReadU128();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteU128(Quanta);
	}
};

struct SPACETIMEDB_API FTransactionUpdateData
{
	FUpdateStatus Status;
	FTimestamp Timestamp;
	FStdbIdentity CallerIdentity;
	FStdbConnectionId CallerConnectionId;
	FReducerCallInfo ReducerCall;
	FEnergyQuanta EnergyQuantaUsed;
	FTimeDuration TotalHostExecutionDuration;

	FTransactionUpdateData()
		: Status()
		  , Timestamp()
		  , CallerIdentity()
		  , CallerConnectionId()
		  , ReducerCall()
		  , EnergyQuantaUsed()
		  , TotalHostExecutionDuration()
	{
	}

	FTransactionUpdateData(
		const FUpdateStatus& InStatus,
		const FTimestamp& InTimestamp,
		const FStdbIdentity& InCallerIdentity,
		const FStdbConnectionId& InCallerConnectionId,
		const FReducerCallInfo& InReducerCall,
		const FEnergyQuanta& InEnergyQuantaUsed,
		const FTimeDuration& InTotalHostExecutionDuration)
		: Status(InStatus)
		  , Timestamp(InTimestamp)
		  , CallerIdentity(InCallerIdentity)
		  , CallerConnectionId(InCallerConnectionId)
		  , ReducerCall(InReducerCall)
		  , EnergyQuantaUsed(InEnergyQuantaUsed)
		  , TotalHostExecutionDuration(InTotalHostExecutionDuration)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		Status.ReadFields(reader);
		Timestamp = reader.ReadTimestamp();
		CallerIdentity = reader.ReadIdentity();
		CallerConnectionId = reader.ReadConnectionId();
		ReducerCall.ReadFields(reader);
		EnergyQuantaUsed.ReadFields(reader);
		TotalHostExecutionDuration = reader.ReadTimeDuration();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		Status.WriteFields(writer);
		writer.WriteTimestamp(Timestamp);
		writer.WriteIdentity(CallerIdentity);
		writer.WriteConnectionId(CallerConnectionId);
		ReducerCall.WriteFields(writer);
		EnergyQuantaUsed.WriteFields(writer);
		writer.WriteTimeDuration(TotalHostExecutionDuration);
	}
};

struct SPACETIMEDB_API FTransactionUpdateLightData
{
	uint32 RequestId;
	FDatabaseUpdate Update;

	FTransactionUpdateLightData()
		: RequestId(0)
		  , Update()
	{
	}

	FTransactionUpdateLightData(uint32 InRequestId, const FDatabaseUpdate& InUpdate)
		: RequestId(InRequestId)
		  , Update(InUpdate)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		RequestId = reader.ReadUInt32();
		Update.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(RequestId);
		Update.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FIdentityTokenData
{
	FStdbIdentity Identity;
	FString Token;
	FStdbConnectionId ConnectionId;

	void ReadFields(FBinaryReader& reader)
	{
		Identity = reader.ReadIdentity();
		Token = reader.ReadString();
		ConnectionId = reader.ReadConnectionId();
	}

	void WriteField(FBinaryWriter writer) const
	{
		writer.WriteIdentity(Identity);
		writer.WriteString(Token);
		writer.WriteConnectionId(ConnectionId);
	}
};

struct SPACETIMEDB_API FOneOffTable
{
	FString TableName;
	FBsatnRowList Rows;

	FOneOffTable()
		: TableName("")
		  , Rows()
	{
	}

	FOneOffTable(const FString& InTableName, const FBsatnRowList& InRows)
		: TableName(InTableName)
		  , Rows(InRows)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		TableName = reader.ReadString();
		Rows.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteString(TableName);
		Rows.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FOneOffQueryResponseData
{
	TArray<uint8> MessageId;
	TOptional<FString> Error;
	TArray<FOneOffTable> Tables;
	FTimeDuration TotalHostExecutionDuration;

	FOneOffQueryResponseData()
		: TotalHostExecutionDuration()
	{
	}

	FOneOffQueryResponseData(
		const TArray<uint8>& InMessageId,
		const TOptional<FString>& InError,
		const TArray<FOneOffTable>& InTables,
		const FTimeDuration& InTotalHostExecutionDuration)
		: MessageId(InMessageId)
		  , Error(InError)
		  , Tables(InTables)
		  , TotalHostExecutionDuration(InTotalHostExecutionDuration)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		MessageId = reader.ReadArray<uint8>([](FBinaryReader& R) { return R.ReadByte(); });
		Error = reader.ReadOptionalString();
		Tables = reader.ReadArray<FOneOffTable>([](FBinaryReader& R)
		{
			FOneOffTable Table;
			Table.ReadFields(R);
			return Table;
		});

		TotalHostExecutionDuration = reader.ReadTimeDuration();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteArray<uint8>(MessageId, [](FBinaryWriter& W, const uint8& Byte) { W.WriteByte(Byte); });
		writer.WriteOptionalString(Error);
		writer.WriteArray<FOneOffTable>(Tables, [](FBinaryWriter& W, const FOneOffTable& Table)
		{
			Table.WriteFields(W);
		});

		writer.WriteTimeDuration(TotalHostExecutionDuration);
	}
};



struct SPACETIMEDB_API FSubscribeRows
{
	uint32 TableId;
	FString TableName;
	FTableUpdate TableRows;

	FSubscribeRows()
		: TableId(0)
		  , TableName("")
		  , TableRows()
	{
	}

	FSubscribeRows(uint32 InTableId, const FString& InTableName, const FTableUpdate& InTableRows)
		: TableId(InTableId)
		  , TableName(InTableName)
		  , TableRows(InTableRows)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		TableId = reader.ReadUInt32();
		TableName = reader.ReadString();
		TableRows.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(TableId);
		writer.WriteString(TableName);
		TableRows.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FSubscribeAppliedData
{
	uint32 RequestId;
	uint64 TotalHostExecutionDurationMicros;
	FQueryId QueryId;
	FSubscribeRows Rows;

	FSubscribeAppliedData()
		: RequestId(0)
		  , TotalHostExecutionDurationMicros(0)
		  , QueryId()
		  , Rows()
	{
	}

	FSubscribeAppliedData(
		uint32 InRequestId,
		uint64 InTotalHostExecutionDurationMicros,
		const FQueryId& InQueryId,
		const FSubscribeRows& InRows)
		: RequestId(InRequestId)
		  , TotalHostExecutionDurationMicros(InTotalHostExecutionDurationMicros)
		  , QueryId(InQueryId)
		  , Rows(InRows)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		RequestId = reader.ReadUInt32();
		TotalHostExecutionDurationMicros = reader.ReadUInt64();
		QueryId.ReadFields(reader);
		Rows.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(RequestId);
		writer.WriteUInt64(TotalHostExecutionDurationMicros);
		QueryId.WriteFields(writer);
		Rows.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FUnsubscribeAppliedData
{
	uint32 RequestId;
	uint64 TotalHostExecutionDurationMicros;
	FQueryId QueryId;
	FSubscribeRows Rows;

	FUnsubscribeAppliedData()
		: RequestId(0)
		  , TotalHostExecutionDurationMicros(0)
		  , QueryId()
		  , Rows()
	{
	}

	FUnsubscribeAppliedData(
		uint32 InRequestId,
		uint64 InTotalHostExecutionDurationMicros,
		const FQueryId& InQueryId,
		const FSubscribeRows& InRows)
		: RequestId(InRequestId)
		  , TotalHostExecutionDurationMicros(InTotalHostExecutionDurationMicros)
		  , QueryId(InQueryId)
		  , Rows(InRows)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		RequestId = reader.ReadUInt32();
		TotalHostExecutionDurationMicros = reader.ReadUInt64();
		QueryId.ReadFields(reader);
		Rows.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(RequestId);
		writer.WriteUInt64(TotalHostExecutionDurationMicros);
		QueryId.WriteFields(writer);
		Rows.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FSubscriptionErrorData
{
	uint64 TotalHostExecutionDurationMicros;
	TOptional<uint32> RequestId;
	TOptional<uint32> QueryId;
	TOptional<uint32> TableId;
	FString Error;

	FSubscriptionErrorData()
		: TotalHostExecutionDurationMicros(0)
		  , Error("")
	{
	}

	FSubscriptionErrorData(
		uint64 InTotalHostExecutionDurationMicros,
		const TOptional<uint32>& InRequestId,
		const TOptional<uint32>& InQueryId,
		const TOptional<uint32>& InTableId,
		const FString& InError)
		: TotalHostExecutionDurationMicros(InTotalHostExecutionDurationMicros)
		  , RequestId(InRequestId)
		  , QueryId(InQueryId)
		  , TableId(InTableId)
		  , Error(InError)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		TotalHostExecutionDurationMicros = reader.ReadUInt64();
		RequestId = reader.ReadOptionalUInt32();
		QueryId = reader.ReadOptionalUInt32();
		TableId = reader.ReadOptionalUInt32();
		Error = reader.ReadString();
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt64(TotalHostExecutionDurationMicros);
		writer.WriteOptionalUInt32(RequestId);
		writer.WriteOptionalUInt32(QueryId);
		writer.WriteOptionalUInt32(TableId);
		writer.WriteString(Error);
	}
};

struct SPACETIMEDB_API FSubscribeMultiAppliedData
{
	uint32 RequestId;
	uint64 TotalHostExecutionDurationMicros;
	FQueryId QueryId;
	FDatabaseUpdate Update;

	FSubscribeMultiAppliedData()
		: RequestId(0)
		  , TotalHostExecutionDurationMicros(0)
		  , QueryId()
		  , Update()
	{
	}

	FSubscribeMultiAppliedData(
		uint32 InRequestId,
		uint64 InTotalHostExecutionDurationMicros,
		const FQueryId& InQueryId,
		const FDatabaseUpdate& InUpdate)
		: RequestId(InRequestId)
		  , TotalHostExecutionDurationMicros(InTotalHostExecutionDurationMicros)
		  , QueryId(InQueryId)
		  , Update(InUpdate)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		RequestId = reader.ReadUInt32();
		TotalHostExecutionDurationMicros = reader.ReadUInt64();
		QueryId.ReadFields(reader);
		Update.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(RequestId);
		writer.WriteUInt64(TotalHostExecutionDurationMicros);
		QueryId.WriteFields(writer);
		Update.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FUnsubscribeMultiAppliedData
{
	uint32 RequestId;
	uint64 TotalHostExecutionDurationMicros;
	FQueryId QueryId;
	FDatabaseUpdate Update;

	FUnsubscribeMultiAppliedData()
		: RequestId(0)
		  , TotalHostExecutionDurationMicros(0)
		  , QueryId()
		  , Update()
	{
	}

	FUnsubscribeMultiAppliedData(
		uint32 InRequestId,
		uint64 InTotalHostExecutionDurationMicros,
		const FQueryId& InQueryId,
		const FDatabaseUpdate& InUpdate)
		: RequestId(InRequestId)
		  , TotalHostExecutionDurationMicros(InTotalHostExecutionDurationMicros)
		  , QueryId(InQueryId)
		  , Update(InUpdate)
	{
	}

	void ReadFields(FBinaryReader& reader)
	{
		RequestId = reader.ReadUInt32();
		TotalHostExecutionDurationMicros = reader.ReadUInt64();
		QueryId.ReadFields(reader);
		Update.ReadFields(reader);
	}

	void WriteFields(FBinaryWriter& writer) const
	{
		writer.WriteUInt32(RequestId);
		writer.WriteUInt64(TotalHostExecutionDurationMicros);
		QueryId.WriteFields(writer);
		Update.WriteFields(writer);
	}
};

struct SPACETIMEDB_API FServerMessage
{
	EServerMessageType Type;
	TVariant<
		FIdentityTokenData,
		FInitialSubscriptionData,
		FTransactionUpdateData,
		FTransactionUpdateLightData,
		FOneOffQueryResponseData,
		FSubscribeAppliedData,
		FUnsubscribeAppliedData,
		FSubscriptionErrorData,
		FSubscribeMultiAppliedData,
		FUnsubscribeMultiAppliedData> Data;

	static FServerMessage Deserialize(FBinaryReader& reader)
	{
		FServerMessage result;
		uint8 messageType = reader.ReadByte();
		result.Type = static_cast<EServerMessageType>(messageType);

		switch (result.Type)
		{
		case EServerMessageType::IdentityToken:
			{
				FIdentityTokenData IdentityToken;
				IdentityToken.ReadFields(reader);
				result.Data.Emplace<FIdentityTokenData>(MoveTemp(IdentityToken));
				break;
			}
		case EServerMessageType::InitialSubscription:
			{
				FInitialSubscriptionData InitialSubscription;
				InitialSubscription.ReadFields(reader);
				result.Data.Emplace<FInitialSubscriptionData>(MoveTemp(InitialSubscription));
				UE_LOG(LogStdb, Log, TEXT("Initial Subscription data received!"));
				break;
			}
		case EServerMessageType::TransactionUpdate:
			{
				FTransactionUpdateData TransactionUpdate;
				TransactionUpdate.ReadFields(reader);
				result.Data.Emplace<FTransactionUpdateData>(MoveTemp(TransactionUpdate));
				break;
			}
		case EServerMessageType::TransactionUpdateLight:
			{
				FTransactionUpdateLightData TransactionUpdateLight;
				TransactionUpdateLight.ReadFields(reader);
				result.Data.Emplace<FTransactionUpdateLightData>(MoveTemp(TransactionUpdateLight));
				break;
			}
		case EServerMessageType::OneOffQueryResponse:
			{
				FOneOffQueryResponseData OneOffQueryResponse;
				OneOffQueryResponse.ReadFields(reader);
				result.Data.Emplace<FOneOffQueryResponseData>(MoveTemp(OneOffQueryResponse));
				break;
			}
		case EServerMessageType::SubscribeApplied:
			{
				FSubscribeAppliedData SubscribeApplied;
				SubscribeApplied.ReadFields(reader);
				result.Data.Emplace<FSubscribeAppliedData>(MoveTemp(SubscribeApplied));
				break;
			}
		case EServerMessageType::UnsubscribeApplied:
			{
				FUnsubscribeAppliedData UnsubscribeApplied;
				UnsubscribeApplied.ReadFields(reader);
				result.Data.Emplace<FUnsubscribeAppliedData>(MoveTemp(UnsubscribeApplied));
				break;
			}
		case EServerMessageType::SubscriptionError:
			{
				FSubscriptionErrorData SubscriptionError;
				SubscriptionError.ReadFields(reader);
				result.Data.Emplace<FSubscriptionErrorData>(MoveTemp(SubscriptionError));
				break;
			}
		case EServerMessageType::SubscribeMultiApplied:
			{
				FSubscribeMultiAppliedData SubscribeMultiApplied;
				SubscribeMultiApplied.ReadFields(reader);
				result.Data.Emplace<FSubscribeMultiAppliedData>(MoveTemp(SubscribeMultiApplied));
				break;
			}
		case EServerMessageType::UnsubscribeMultiApplied:
			{
				FUnsubscribeMultiAppliedData UnsubscribeMultiApplied;
				UnsubscribeMultiApplied.ReadFields(reader);
				result.Data.Emplace<FUnsubscribeMultiAppliedData>(MoveTemp(UnsubscribeMultiApplied));
				break;
			}
		default:
			{
				UE_LOG(LogStdb, Warning, TEXT("Unknown server message type: %d"), static_cast<int32>(result.Type));
				break;
			}
		}
		return result;
	}
};
