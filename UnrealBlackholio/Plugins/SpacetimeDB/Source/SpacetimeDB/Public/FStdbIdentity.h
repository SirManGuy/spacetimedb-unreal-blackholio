#pragma once

#include "CoreMinimal.h"
#include "FByteUtils.h"
#include "FU256.h"
#include "FStdbIdentity.generated.h"

USTRUCT(BlueprintType)
struct SPACETIMEDB_API FStdbIdentity
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FU256 Value;

public:
	FStdbIdentity()
		: Value()
	{
	}

	explicit FStdbIdentity(const FU256& InValue)
		: Value(InValue)
	{
	}

	FORCEINLINE const FU256& GetValue() const { return Value; }

	static FStdbIdentity FromLittleEndian(const TArray<uint8>& Bytes)
	{
		return FStdbIdentity(FByteUtils::ReadFU256FromLE(Bytes));
	}
	
	static FStdbIdentity FromBigEndian(const TArray<uint8>& Bytes)
	{
		return FStdbIdentity(FByteUtils::ReadFU256FromBE(Bytes));
	}

	static FStdbIdentity FromHexString(const FString& Hex)
	{
		TArray<uint8> Bytes = FByteUtils::HexStringToBytes(Hex);
		return FromBigEndian(Bytes);
	}

	FString ToString() const
	{
		return Value.ToString();
	}

	int32 CompareTo(const FStdbIdentity& Other) const
	{
		return Value.CompareTo(Other.Value);
	}

	friend bool operator==(const FStdbIdentity& A, const FStdbIdentity& B)
	{
		return A.Value == B.Value;
	}

	friend bool operator!=(const FStdbIdentity& A, const FStdbIdentity& B)
	{
		return !(A == B);
	}

	friend bool operator<(const FStdbIdentity& A, const FStdbIdentity& B)
	{
		return A.Value < B.Value;
	}

	friend bool operator>(const FStdbIdentity& A, const FStdbIdentity& B)
	{
		return A.Value > B.Value;
	}

	friend uint32 GetTypeHash(const FStdbIdentity& Identity)
	{
		return ::GetTypeHash(Identity.Value);
	}

	bool Equals(const FStdbIdentity& Other) const
	{
		return Value == Other.Value;
	}
};
