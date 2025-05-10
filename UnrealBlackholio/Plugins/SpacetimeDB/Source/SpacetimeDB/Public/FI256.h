#pragma once

#include "CoreMinimal.h"
#include "FI128.h"
#include "FI256.generated.h"

USTRUCT(BlueprintType)
struct SPACETIMEDB_API FI256
{
    GENERATED_BODY()

private:
    UPROPERTY()
    FI128 Lower;

    UPROPERTY()
    FI128 Upper;

public:
    FI256()
        : Lower(), Upper()
    {}

    FI256(const FI128& InUpper, const FI128& InLower)
        : Lower(InLower), Upper(InUpper)
    {}

    FORCEINLINE const FI128& GetLower() const { return Lower; }
    FORCEINLINE const FI128& GetUpper() const { return Upper; }

    // Sign check based on highest bit of Upper's upper 64-bits
    static bool IsNegative(const FI256& Value)
    {
        return static_cast<int64>(Value.Upper.GetUpper()) < 0;
    }

    // Comparison operators
    friend bool operator<(const FI256& A, const FI256& B)
    {
        if (IsNegative(A) != IsNegative(B))
            return IsNegative(A);

        if (A.Upper < B.Upper)
            return true;

        return A.Upper == B.Upper && A.Lower < B.Lower;
    }

    friend bool operator>(const FI256& A, const FI256& B)
    {
        if (IsNegative(A) != IsNegative(B))
            return IsNegative(B);

        if (A.Upper > B.Upper)
            return true;

        return A.Upper == B.Upper && A.Lower > B.Lower;
    }

    friend bool operator==(const FI256& A, const FI256& B)
    {
        return A.Upper == B.Upper && A.Lower == B.Lower;
    }

    friend bool operator!=(const FI256& A, const FI256& B)
    {
        return !(A == B);
    }

    bool Equals(const FI256& Other) const
    {
        return *this == Other;
    }

    int32 CompareTo(const FI256& Other) const
    {
        if (*this < Other) return -1;
        if (*this > Other) return 1;
        return 0;
    }

    FString ToString() const
    {
        // Combine both parts into a single string in hex
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

FORCEINLINE uint32 GetTypeHash(const FI256& Value)
{
    return HashCombine(::GetTypeHash(Value.GetLower()), ::GetTypeHash(Value.GetUpper()));
}