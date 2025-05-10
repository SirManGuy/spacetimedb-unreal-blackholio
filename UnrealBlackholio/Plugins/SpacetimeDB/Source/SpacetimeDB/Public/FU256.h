#pragma once

#include "CoreMinimal.h"
#include "FU128.h"
#include "FU256.generated.h"

USTRUCT(BlueprintType)
struct SPACETIMEDB_API FU256
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FU128 Lower;

	UPROPERTY()
	FU128 Upper;

public:
	FU256()
		: Lower(), Upper()
	{}

	FU256(const FU128& InUpper, const FU128& InLower)
		: Lower(InLower), Upper(InUpper)
	{}

	FORCEINLINE FU128 GetLower() const { return Lower; }
	FORCEINLINE FU128 GetUpper() const { return Upper; }

	// Comparison operators
	friend bool operator<(const FU256& A, const FU256& B)
	{
		return (A.Upper < B.Upper) || (A.Upper == B.Upper && A.Lower < B.Lower);
	}

	friend bool operator>(const FU256& A, const FU256& B)
	{
		return (A.Upper > B.Upper) || (A.Upper == B.Upper && A.Lower > B.Lower);
	}

	friend bool operator==(const FU256& A, const FU256& B)
	{
		return A.Upper == B.Upper && A.Lower == B.Lower;
	}

	friend bool operator!=(const FU256& A, const FU256& B)
	{
		return !(A == B);
	}

	bool Equals(const FU256& Other) const
	{
		return *this == Other;
	}

	int32 CompareTo(const FU256& Other) const
	{
		if (*this < Other) return -1;
		if (*this > Other) return 1;
		return 0;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("0x%s%s"),
			*Upper.ToString().RightChop(2),
			*Lower.ToString().RightChop(2));
	}

	TArray<uint8> ToBytesBE() const
	{
		TArray<uint8> UpperBytes = Upper.ToBytesBE();
		TArray<uint8> LowerBytes = Lower.ToBytesBE();

		TArray<uint8> Combined;
		Combined.Append(UpperBytes);
		Combined.Append(LowerBytes);
		return Combined;
	}

	TArray<uint8> ToBytesLE() const
	{
		TArray<uint8> LowerBytes = Lower.ToBytesLE();
		TArray<uint8> UpperBytes = Upper.ToBytesLE();

		TArray<uint8> Combined;
		Combined.Append(LowerBytes);
		Combined.Append(UpperBytes);
		return Combined;
	}

};

FORCEINLINE uint32 GetTypeHash(const FU256& Value)
{
	return HashCombine(::GetTypeHash(Value.GetLower()), ::GetTypeHash(Value.GetUpper()));
}