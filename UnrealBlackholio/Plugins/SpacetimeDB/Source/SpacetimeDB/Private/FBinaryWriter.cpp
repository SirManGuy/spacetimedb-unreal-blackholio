#include "FBinaryWriter.h"
#include "FTimeDuration.h"
#include "FTimestamp.h"
#include "FStdbConnectionId.h"
#include "FStdbIdentity.h"

FBinaryWriter::FBinaryWriter(int64 InitialCapacity)
    : Data(&InternalData)
    , bOwnsData(true)
    , Position(0)
{
    InternalData.Reserve(InitialCapacity);
}

FBinaryWriter::FBinaryWriter(TArray<uint8>& InData)
    : Data(&InData)
    , bOwnsData(false)
    , Position(InData.Num())
{
}

FBinaryWriter::~FBinaryWriter()
{
}

void FBinaryWriter::SetPosition(int64 NewPosition)
{
    check(NewPosition >= 0);
    
    if (NewPosition > Data->Num())
    {
        int64 OldSize = Data->Num();
        Data->SetNum(NewPosition);
        
        if (NewPosition > OldSize)
        {
            FMemory::Memzero(Data->GetData() + OldSize, NewPosition - OldSize);
        }
    }
    
    Position = NewPosition;
}

void FBinaryWriter::Reset()
{
    Data->Empty();
    Position = 0;
}

void FBinaryWriter::EnsureCapacity(int64 AdditionalBytes)
{
    int64 NeededSize = Position + AdditionalBytes;
    if (NeededSize > Data->Num())
    {
        Data->SetNum(NeededSize);
    }
}

void FBinaryWriter::WriteBytes(const void* InData, int64 Count)
{
    if (Count <= 0)
        return;
        
    EnsureCapacity(Count);
    FMemory::Memcpy(Data->GetData() + Position, InData, Count);
    Position += Count;
}

void FBinaryWriter::WriteBool(bool Value)
{
    WriteByte(Value ? 1 : 0);
}

void FBinaryWriter::WriteByte(uint8 Value)
{
    EnsureCapacity(1);
    (*Data)[Position++] = Value;
}

void FBinaryWriter::WriteOptionalByte(const TOptional<uint8>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteByte(Value.GetValue());
    }
}

void FBinaryWriter::WriteSByte(int8 Value)
{
    WriteByte(static_cast<uint8>(Value));
}

void FBinaryWriter::WriteOptionalSByte(const TOptional<int8>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteSByte(Value.GetValue());
    }
}

void FBinaryWriter::WriteInt16(int16 Value)
{
    EnsureCapacity(2);
    FMemory::Memcpy(Data->GetData() + Position, &Value, 2);
    Position += 2;
}

void FBinaryWriter::WriteOptionalInt16(const TOptional<int16>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteInt16(Value.GetValue());
    }
}

void FBinaryWriter::WriteUInt16(uint16 Value)
{
    WriteInt16(static_cast<int16>(Value));
}

void FBinaryWriter::WriteOptionalUInt16(const TOptional<uint16>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteUInt16(Value.GetValue());
    }
}

void FBinaryWriter::WriteInt32(int32 Value)
{
    EnsureCapacity(4);
    FMemory::Memcpy(Data->GetData() + Position, &Value, 4);
    Position += 4;
}

void FBinaryWriter::WriteOptionalInt32(const TOptional<int32>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteInt32(Value.GetValue());
    }
}

void FBinaryWriter::WriteUInt32(uint32 Value)
{
    WriteInt32(static_cast<int32>(Value));
}

void FBinaryWriter::WriteOptionalUInt32(const TOptional<uint32>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteUInt32(Value.GetValue());
    }
}

void FBinaryWriter::WriteInt64(int64 Value)
{
    EnsureCapacity(8);
    FMemory::Memcpy(Data->GetData() + Position, &Value, 8);
    Position += 8;
}

void FBinaryWriter::WriteOptionalInt64(const TOptional<int64>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteInt64(Value.GetValue());
    }
}

void FBinaryWriter::WriteUInt64(uint64 Value)
{
    WriteInt64(static_cast<int64>(Value));
}

void FBinaryWriter::WriteOptionalUInt64(const TOptional<uint64>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteUInt64(Value.GetValue());
    }
}

void FBinaryWriter::WriteFloat(float Value)
{
    union {
        float f;
        uint32 i;
    } u;
    
    u.f = Value;
    WriteUInt32(u.i);
}

void FBinaryWriter::WriteOptionalFloat(const TOptional<float>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteFloat(Value.GetValue());
    }
}

void FBinaryWriter::WriteDouble(double Value)
{
    union {
        double d;
        uint64 i;
    } u;
    
    u.d = Value;
    WriteUInt64(u.i);
}

void FBinaryWriter::WriteOptionalDouble(const TOptional<double>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteDouble(Value.GetValue());
    }
}

void FBinaryWriter::WriteString(const FString& Value)
{
    // Get UTF-8 bytes
    FTCHARToUTF8 Converter(*Value);
    int32 Length = Converter.Length();
    
    // Write length first
    WriteInt32(Length);
    
    if (Length > 0)
    {
        // Write string data
        WriteBytes(Converter.Get(), Length);
    }
}

void FBinaryWriter::WriteOptionalString(const TOptional<FString>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteString(Value.GetValue());
    }
}

void FBinaryWriter::WriteI128(const FI128& Value)
{
    WriteUInt64(Value.GetUpper());
    WriteUInt64(Value.GetLower());
}

void FBinaryWriter::WriteOptionalI128(const TOptional<FI128>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteI128(Value.GetValue());
    }
}

void FBinaryWriter::WriteU128(const FU128& Value)
{
    WriteUInt64(Value.GetUpper());
    WriteUInt64(Value.GetLower());
}

void FBinaryWriter::WriteOptionalU128(const TOptional<FU128>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteU128(Value.GetValue());
    }
}

void FBinaryWriter::WriteI256(const FI256& Value)
{
    WriteI128(Value.GetUpper());
    WriteI128(Value.GetLower());
}

void FBinaryWriter::WriteOptionalI256(const TOptional<FI256>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteI256(Value.GetValue());
    }
}

void FBinaryWriter::WriteU256(const FU256& Value)
{
    WriteU128(Value.GetUpper());
    WriteU128(Value.GetLower());
}

void FBinaryWriter::WriteOptionalU256(const TOptional<FU256>& Value)
{
    WriteBool(Value.IsSet());
    if (Value.IsSet())
    {
        WriteU256(Value.GetValue());
    }
}


void FBinaryWriter::WriteTimeDuration(const FTimeDuration& Value)
{
    WriteInt64(Value.GetMicros());
}

void FBinaryWriter::WriteTimestamp(const FTimestamp& Value)
{
    WriteInt64(Value.GetMicros());
}

void FBinaryWriter::WriteConnectionId(const FStdbConnectionId& Value)
{
    WriteU128(Value.GetValue());
}

void FBinaryWriter::WriteIdentity(const FStdbIdentity& Value)
{
    WriteU256(Value.GetValue());
}

void FBinaryWriter::WriteStringArray(const TArray<FString>& Array)
{
    WriteArray<FString>(Array, [](FBinaryWriter& Writer, const FString& Str) { Writer.WriteString(Str); });
}

template<typename T>
void FBinaryWriter::WritePrimitiveArray(const TArray<T>& Array)
{
    WriteInt32(Array.Num());
    
    if (Array.Num() > 0)
    {
        // Write directly from the array's buffer for performance
        WriteBytes(Array.GetData(), Array.Num() * sizeof(T));
    }
}