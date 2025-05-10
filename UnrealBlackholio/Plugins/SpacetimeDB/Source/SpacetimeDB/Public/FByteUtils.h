#pragma once

#include "CoreMinimal.h"
#include "FU128.h"
#include "FU256.h"

class FByteUtils
{
public:
    static uint64 ReadUInt64BE(const uint8* Data)
    {
        return (static_cast<uint64>(Data[0]) << 56) |
               (static_cast<uint64>(Data[1]) << 48) |
               (static_cast<uint64>(Data[2]) << 40) |
               (static_cast<uint64>(Data[3]) << 32) |
               (static_cast<uint64>(Data[4]) << 24) |
               (static_cast<uint64>(Data[5]) << 16) |
               (static_cast<uint64>(Data[6]) << 8)  |
               (static_cast<uint64>(Data[7]));
    }

    static FU128 ReadFU128FromBE(const TArray<uint8>& Bytes, int32 Offset = 0)
    {
        check(Bytes.Num() >= Offset + 16);
        return FU128(
            ReadUInt64BE(Bytes.GetData() + Offset),
            ReadUInt64BE(Bytes.GetData() + Offset + 8)
        );
    }

    static FU128 ReadFU128FromLE(const TArray<uint8>& Bytes, int32 Offset = 0)
    {
        check(Bytes.Num() >= Offset + 16);
        return FU128(
            *reinterpret_cast<const uint64*>(Bytes.GetData() + Offset + 8),
            *reinterpret_cast<const uint64*>(Bytes.GetData() + Offset)
        );
    }

    static FU256 ReadFU256FromBE(const TArray<uint8>& Bytes)
    {
        check(Bytes.Num() == 32);
        return FU256(
            ReadFU128FromBE(Bytes, 0),
            ReadFU128FromBE(Bytes, 16)
        );
    }

    static FU256 ReadFU256FromLE(const TArray<uint8>& Bytes)
    {
        check(Bytes.Num() == 32);
        return FU256(
            ReadFU128FromLE(Bytes, 16),
            ReadFU128FromLE(Bytes, 0)
        );
    }

    static TArray<uint8> HexStringToBytes(const FString& Hex)
    {
        FString CleanHex = Hex.Replace(TEXT("0x"), TEXT(""));
        int32 NumBytes = CleanHex.Len() / 2;
        TArray<uint8> Bytes;
        Bytes.SetNumUninitialized(NumBytes);

        for (int32 i = 0; i < NumBytes; ++i)
        {
            FString ByteStr = CleanHex.Mid(i * 2, 2);
            Bytes[i] = FParse::HexDigit(ByteStr[0]) << 4 | FParse::HexDigit(ByteStr[1]);
        }

        return Bytes;
    }
};
