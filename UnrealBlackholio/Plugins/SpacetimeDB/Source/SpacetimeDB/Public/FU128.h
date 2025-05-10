#pragma once

#include "CoreMinimal.h"
#include "FU128.generated.h"

USTRUCT(BlueprintType)
struct SPACETIMEDB_API FU128
{
	GENERATED_BODY()

private:
	// Lower 64 bits
	UPROPERTY()
	uint64 Lower;

	// Upper 64 bits
	UPROPERTY()
	uint64 Upper;

public:
	FU128()
		: Lower(0), Upper(0)
	{}

	FU128(uint64 InUpper, uint64 InLower)
		: Lower(InLower), Upper(InUpper)
	{}

	FORCEINLINE uint64 GetLower() const { return Lower; }
	FORCEINLINE uint64 GetUpper() const { return Upper; }

	// Comparison operators
	friend bool operator<(const FU128& A, const FU128& B)
	{
		return (A.Upper < B.Upper) || (A.Upper == B.Upper && A.Lower < B.Lower);
	}

	friend bool operator>(const FU128& A, const FU128& B)
	{
		return (A.Upper > B.Upper) || (A.Upper == B.Upper && A.Lower > B.Lower);
	}

	friend bool operator==(const FU128& A, const FU128& B)
	{
		return A.Upper == B.Upper && A.Lower == B.Lower;
	}

	friend bool operator!=(const FU128& A, const FU128& B)
	{
		return !(A == B);
	}

	bool Equals(const FU128& Other) const
	{
		return *this == Other;
	}

	int32 CompareTo(const FU128& Other) const
	{
		if (*this < Other) return -1;
		if (*this > Other) return 1;
		return 0;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("0x%016llX%016llX"), Upper, Lower);
	}

	TArray<uint8> ToBytesBE() const
	{
		TArray<uint8> Bytes;
		Bytes.SetNumUninitialized(16);

		auto WriteUInt64BE = [](uint8* Out, uint64 Value)
		{
			Out[0] = static_cast<uint8>(Value >> 56);
			Out[1] = static_cast<uint8>(Value >> 48);
			Out[2] = static_cast<uint8>(Value >> 40);
			Out[3] = static_cast<uint8>(Value >> 32);
			Out[4] = static_cast<uint8>(Value >> 24);
			Out[5] = static_cast<uint8>(Value >> 16);
			Out[6] = static_cast<uint8>(Value >> 8);
			Out[7] = static_cast<uint8>(Value);
		};

		WriteUInt64BE(&Bytes[0], Upper);
		WriteUInt64BE(&Bytes[8], Lower);

		return Bytes;
	}

	TArray<uint8> ToBytesLE() const
	{
		TArray<uint8> Bytes;
		Bytes.SetNumUninitialized(16);

		FMemory::Memcpy(&Bytes[0], &Lower, 8);
		FMemory::Memcpy(&Bytes[8], &Upper, 8);

		return Bytes;
	}
};

FORCEINLINE uint32 GetTypeHash(const FU128& Value)
{
	return HashCombine(::GetTypeHash(Value.GetLower()), ::GetTypeHash(Value.GetUpper()));
}