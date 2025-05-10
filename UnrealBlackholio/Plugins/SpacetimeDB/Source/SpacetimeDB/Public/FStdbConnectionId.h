#pragma once

#include "CoreMinimal.h"
#include "FByteUtils.h"
#include "FU128.h"
#include "FStdbConnectionId.generated.h"

USTRUCT(BlueprintType)
struct FStdbConnectionId
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintreadWrite)
	FU128 Value;

public:
	FStdbConnectionId()
		: Value()
	{}

	explicit FStdbConnectionId(const FU128& InValue)
		: Value(InValue)
	{}

	FORCEINLINE const FU128& GetValue() const { return Value; }

	static TOptional<FStdbConnectionId> FromLittleEndian(const TArray<uint8>& Bytes)
	{
		if (Bytes.Num() != 16)
		{
			return TOptional<FStdbConnectionId>();
		}

		FU128 Parsed = FByteUtils::ReadFU128FromLE(Bytes);
		return Parsed == FU128() ? TOptional<FStdbConnectionId>() : TOptional<FStdbConnectionId>(FStdbConnectionId(Parsed));
	}

	static TOptional<FStdbConnectionId> FromBigEndian(const TArray<uint8>& Bytes)
	{
		if (Bytes.Num() != 16)
		{
			return TOptional<FStdbConnectionId>();
		}

		FU128 Parsed = FByteUtils::ReadFU128FromBE(Bytes);
		return Parsed == FU128() ? TOptional<FStdbConnectionId>() : TOptional<FStdbConnectionId>(FStdbConnectionId(Parsed));
	}

	static TOptional<FStdbConnectionId> FromHexString(const FString& Hex)
	{
		TArray<uint8> Bytes = FByteUtils::HexStringToBytes(Hex);
		return FromBigEndian(Bytes);
	}

	FString ToString() const
	{
		return Value.ToString();
	}

	int32 CompareTo(const FStdbConnectionId& Other) const
	{
		return Value.CompareTo(Other.Value);
	}

	friend bool operator==(const FStdbConnectionId& A, const FStdbConnectionId& B)
	{
		return A.Value == B.Value;
	}

	friend bool operator!=(const FStdbConnectionId& A, const FStdbConnectionId& B)
	{
		return !(A == B);
	}

	friend uint32 GetTypeHash(const FStdbConnectionId& Id)
	{
		return ::GetTypeHash(Id.Value);
	}

	bool Equals(const FStdbConnectionId& Other) const
	{
		return Value == Other.Value;
	}
};
