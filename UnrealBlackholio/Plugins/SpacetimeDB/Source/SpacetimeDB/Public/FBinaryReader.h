#pragma once

#include "CoreMinimal.h"
#include "FI128.h"
#include "FI256.h"
#include "FU128.h"
#include "FU256.h"

struct FTimeDuration;
struct FTimestamp;
struct FStdbConnectionId;
struct FStdbIdentity;

class SPACETIMEDB_API FBinaryReader
{
public:
    FBinaryReader(uint8* InData, int64 InSize, bool bInOwnsData = false);
    FBinaryReader(const TArray<uint8>& InData);
    
    ~FBinaryReader();
    
    int64 GetPosition() const { return Position; }
    
    int64 GetSize() const { return Size; }
    
    void SetPosition(int64 NewPosition);
    
    void ReadBytes(void* OutData, int64 Count);
    
    bool ReadBool();
    
    uint8 ReadByte();
    TOptional<uint8> ReadOptionalByte();
    
    int8 ReadSByte();
    TOptional<int8> ReadOptionalSByte();

    int16 ReadInt16();
    TOptional<int16> ReadOptionalInt16();
    
    uint16 ReadUInt16();
    TOptional<uint16> ReadOptionalUInt16();
    
    int32 ReadInt32();
    TOptional<int32> ReadOptionalInt32();
    
    uint32 ReadUInt32();
    TOptional<uint32> ReadOptionalUInt32();
    
    int64 ReadInt64();
    TOptional<int64> ReadOptionalInt64();
    
    uint64 ReadUInt64();
    TOptional<uint64> ReadOptionalUInt64();
    
    float ReadFloat();
    TOptional<float> ReadOptionalFloat();
    
    double ReadDouble();
    TOptional<double> ReadOptionalDouble();
    
    FString ReadString();
    TOptional<FString> ReadOptionalString();
    
    FI128 ReadI128();
    TOptional<FI128> ReadOptionalI128();
    
    FU128 ReadU128();
    TOptional<FU128> ReadOptionalU128();
    
    FI256 ReadI256();
    TOptional<FI256> ReadOptionalI256();
    
    FU256 ReadU256();
    TOptional<FU256> ReadOptionalU256();

    FTimeDuration ReadTimeDuration();
    FTimestamp ReadTimestamp();
    FStdbConnectionId ReadConnectionId();
    FStdbIdentity ReadIdentity();

    template<typename T>
    TArray<T> ReadArray(TFunction<T(FBinaryReader&)> ElementReader)
    {
        int32 Length = ReadInt32();
    
        if (Length < 0)
        {
            // Handle invalid array length
            return TArray<T>();
        }
    
        TArray<T> Result;
        Result.Reserve(Length);
    
        for (int32 i = 0; i < Length; ++i)
        {
            Result.Add(ElementReader(*this));
        }
    
        return Result;
    }
    
    TArray<FString> ReadStringArray();
    template<typename T>
    TArray<T> ReadPrimitiveArray();
    
private:
    uint8* Data;
    int64 Size;
    int64 Position;
    bool bOwnsData;
    
    void EnsureRemaining(int64 Count) const;
};