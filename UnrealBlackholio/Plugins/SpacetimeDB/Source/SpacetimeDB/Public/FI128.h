#pragma once

#include "CoreMinimal.h"
#include "FI128.generated.h"

USTRUCT(BlueprintType)
struct SPACETIMEDB_API FI128
{
    GENERATED_BODY()

private:
    UPROPERTY()
    uint64 Lower;

    UPROPERTY()
    uint64 Upper;

public:
    FI128()
        : Lower(0), Upper(0)
    {}

    FI128(uint64 InUpper, uint64 InLower)
        : Lower(InLower), Upper(InUpper)
    {}

    FORCEINLINE uint64 GetLower() const { return Lower; }
    FORCEINLINE uint64 GetUpper() const { return Upper; }

    // Sign check
    static bool IsNegative(const FI128& Value)
    {
        return static_cast<int64>(Value.Upper) < 0;
    }

    // Comparison operators
    friend bool operator<(const FI128& A, const FI128& B)
    {
        if (IsNegative(A) != IsNegative(B))
            return IsNegative(A);

        if (A.Upper < B.Upper)
            return true;

        return static_cast<int64>(A.Upper) == static_cast<int64>(B.Upper) && A.Lower < B.Lower;
    }

    friend bool operator>(const FI128& A, const FI128& B)
    {
        if (IsNegative(A) != IsNegative(B))
            return IsNegative(B);

        if (A.Upper > B.Upper)
            return true;

        return static_cast<int64>(A.Upper) == static_cast<int64>(B.Upper) && A.Lower > B.Lower;
    }

    friend bool operator==(const FI128& A, const FI128& B)
    {
        return A.Upper == B.Upper && A.Lower == B.Lower;
    }

    friend bool operator!=(const FI128& A, const FI128& B)
    {
        return !(A == B);
    }

    bool Equals(const FI128& Other) const
    {
        return *this == Other;
    }

    int32 CompareTo(const FI128& Other) const
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

FORCEINLINE uint32 GetTypeHash(const FI128& Value)
{
    return HashCombine(::GetTypeHash(Value.GetLower()), ::GetTypeHash(Value.GetUpper()));
}