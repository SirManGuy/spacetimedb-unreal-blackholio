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

class SPACETIMEDB_API FBinaryWriter
{
public:
    FBinaryWriter(int64 InitialCapacity = 1024);
    
    FBinaryWriter(TArray<uint8>& InData);
    
    ~FBinaryWriter();
    
    int64 GetPosition() const { return Position; }
    
    const TArray<uint8>& GetData() const { return *Data; }
    
    void SetPosition(int64 NewPosition);
    
    void Reset();

    void WriteBytes(const void* InData, int64 Count);
    
    void WriteBool(bool Value);
    
    void WriteByte(uint8 Value);
    void WriteOptionalByte(const TOptional<uint8>& Value);

    void WriteSByte(int8 Value);
    void WriteOptionalSByte(const TOptional<int8>& Value);

    void WriteInt16(int16 Value);
    void WriteOptionalInt16(const TOptional<int16>& Value);

    void WriteUInt16(uint16 Value);
    void WriteOptionalUInt16(const TOptional<uint16>& Value);

    void WriteInt32(int32 Value);
    void WriteOptionalInt32(const TOptional<int32>& Value);

    void WriteUInt32(uint32 Value);
    void WriteOptionalUInt32(const TOptional<uint32>& Value);

    void WriteInt64(int64 Value);
    void WriteOptionalInt64(const TOptional<int64>& Value);

    void WriteUInt64(uint64 Value);
    void WriteOptionalUInt64(const TOptional<uint64>& Value);

    void WriteFloat(float Value);
    void WriteOptionalFloat(const TOptional<float>& Value);

    void WriteDouble(double Value);
    void WriteOptionalDouble(const TOptional<double>& Value);

    void WriteString(const FString& Value);
    void WriteOptionalString(const TOptional<FString>& Value);

    void WriteI128(const FI128& Value);
    void WriteOptionalI128(const TOptional<FI128>& Value);

    void WriteU128(const FU128& Value);
    void WriteOptionalU128(const TOptional<FU128>& Value);

    void WriteI256(const FI256& Value);
    void WriteOptionalI256(const TOptional<FI256>& Value);

    void WriteU256(const FU256& Value);
    void WriteOptionalU256(const TOptional<FU256>& Value);
    
    void WriteTimeDuration(const FTimeDuration& Value);
    void WriteTimestamp(const FTimestamp& Value);
    void WriteConnectionId(const FStdbConnectionId& Value);
    void WriteIdentity(const FStdbIdentity& Value);

    template<typename T>
    void WriteArray(const TArray<T>& Array, TFunction<void(FBinaryWriter&, const T&)> ElementWriter)
    {
        WriteInt32(Array.Num());
    
        for (const T& Item : Array)
        {
            ElementWriter(*this, Item);
        }
    }
    
    void WriteStringArray(const TArray<FString>& Array);
    template<typename T>
    void WritePrimitiveArray(const TArray<T>& Array);
private:
    TArray<uint8>* Data;
    TArray<uint8> InternalData;
    bool bOwnsData;
    int64 Position;
    
    void EnsureCapacity(int64 AdditionalBytes);
    
};