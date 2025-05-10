#include "FBinaryReader.h"

#include "FTimeDuration.h"
#include "FTimestamp.h"
#include "FStdbConnectionId.h"
#include "FStdbIdentity.h"

FBinaryReader::FBinaryReader(uint8* InData, int64 InSize, bool bInOwnsData)
    : Data(InData)
    , Size(InSize)
    , Position(0)
    , bOwnsData(bInOwnsData)
{
}

FBinaryReader::FBinaryReader(const TArray<uint8>& InData)
    : Data(const_cast<uint8*>(InData.GetData()))
    , Size(InData.Num())
    , Position(0)
    , bOwnsData(false)
{
}

FBinaryReader::~FBinaryReader()
{
    if (bOwnsData && Data)
    {
        FMemory::Free(Data);
        Data = nullptr;
    }
}

void FBinaryReader::SetPosition(int64 NewPosition)
{
    check(NewPosition >= 0 && NewPosition <= Size);
    Position = NewPosition;
}

void FBinaryReader::ReadBytes(void* OutData, int64 Count)
{
    EnsureRemaining(Count);
    FMemory::Memcpy(OutData, Data + Position, Count);
    Position += Count;
}

bool FBinaryReader::ReadBool()
{
    return ReadByte() != 0;
}
uint8 FBinaryReader::ReadByte()
{
    EnsureRemaining(1);
    return Data[Position++];
}

TOptional<uint8> FBinaryReader::ReadOptionalByte()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadByte();
    }
    return TOptional<uint8>();
}

int8 FBinaryReader::ReadSByte()
{
    return static_cast<int8>(ReadByte());
}

TOptional<int8> FBinaryReader::ReadOptionalSByte()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadSByte();
    }
    return TOptional<int8>();
}

int16 FBinaryReader::ReadInt16()
{
    EnsureRemaining(2);
    int16 Value = *reinterpret_cast<int16*>(Data + Position);
    Position += 2;
    return Value;
}

TOptional<int16> FBinaryReader::ReadOptionalInt16()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadInt16();
    }
    return TOptional<int16>();
}

uint16 FBinaryReader::ReadUInt16()
{
    return static_cast<uint16>(ReadInt16());
}

TOptional<uint16> FBinaryReader::ReadOptionalUInt16()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadUInt16();
    }
    return TOptional<uint16>();
}

int32 FBinaryReader::ReadInt32()
{
    EnsureRemaining(4);
    int32 Value = *reinterpret_cast<int32*>(Data + Position);
    Position += 4;
    return Value;
}

TOptional<int32> FBinaryReader::ReadOptionalInt32()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadInt32();
    }
    return TOptional<int32>();
}

uint32 FBinaryReader::ReadUInt32()
{
    return static_cast<uint32>(ReadInt32());
}

TOptional<uint32> FBinaryReader::ReadOptionalUInt32()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadUInt32();
    }
    return TOptional<uint32>();
}

int64 FBinaryReader::ReadInt64()
{
    EnsureRemaining(8);
    int64 Value = *reinterpret_cast<int64*>(Data + Position);
    Position += 8;
    return Value;
}

TOptional<int64> FBinaryReader::ReadOptionalInt64()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadInt64();
    }
    return TOptional<int64>();
}

uint64 FBinaryReader::ReadUInt64()
{
    return static_cast<uint64>(ReadInt64());
}

TOptional<uint64> FBinaryReader::ReadOptionalUInt64()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadUInt64();
    }
    return TOptional<uint64>();
}

float FBinaryReader::ReadFloat()
{
    EnsureRemaining(4);
    float Value = *reinterpret_cast<float*>(Data + Position);
    Position += 4;
    return Value;
}

TOptional<float> FBinaryReader::ReadOptionalFloat()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadFloat();
    }
    return TOptional<float>();
}

double FBinaryReader::ReadDouble()
{
    EnsureRemaining(8);
    double Value = *reinterpret_cast<double*>(Data + Position);
    Position += 8;
    return Value;
}

TOptional<double> FBinaryReader::ReadOptionalDouble()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadDouble();
    }
    return TOptional<double>();
}

FString FBinaryReader::ReadString()
{
    // Read string length first
    int32 Length = ReadInt32();
    
    if (Length < 0)
    {
        // Handle null or invalid string
        return FString();
    }
    
    EnsureRemaining(Length);
    
    // Convert to FString
    FString Result;
    Result.Reserve(Length);
    
    // Assuming UTF-8 encoding
    for (int32 i = 0; i < Length; ++i)
    {
        Result.AppendChar(static_cast<TCHAR>(Data[Position++]));
    }
    
    return Result;
}

TOptional<FString> FBinaryReader::ReadOptionalString()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadString();
    }
    return TOptional<FString>();
}

void FBinaryReader::EnsureRemaining(int64 Count) const
{
    checkf(Position + Count <= Size, TEXT("Binary reader: attempting to read past end of data. Position: %lld, Size: %lld, Requested: %lld"), 
        Position, Size, Count);
}

// Implementation of new 128/256-bit methods
FI128 FBinaryReader::ReadI128()
{
    EnsureRemaining(16);
    
    // Assuming the binary format has the upper 64 bits first, then the lower 64 bits
    uint64 Upper = ReadUInt64();
    uint64 Lower = ReadUInt64();
    
    return FI128(Upper, Lower);
}

TOptional<FI128> FBinaryReader::ReadOptionalI128()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadI128();
    }
    return TOptional<FI128>();
}

FU128 FBinaryReader::ReadU128()
{
    EnsureRemaining(16);
    
    // Assuming similar structure to FI128 with upper/lower components
    uint64 Upper = ReadUInt64();
    uint64 Lower = ReadUInt64();
    
    return FU128(Upper, Lower);
}

TOptional<FU128> FBinaryReader::ReadOptionalU128()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadU128();
    }
    return TOptional<FU128>();
}

FI256 FBinaryReader::ReadI256()
{
    EnsureRemaining(32);
    
    FI128 Part1 = ReadI128();
    FI128 Part0 = ReadI128();
    
    return FI256(Part1, Part0);
}

TOptional<FI256> FBinaryReader::ReadOptionalI256()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadI256();
    }
    return TOptional<FI256>();
}

FU256 FBinaryReader::ReadU256()
{
    EnsureRemaining(32);
    
    FU128 Part1 = ReadU128();
    FU128 Part0 = ReadU128();
        
    return FU256(Part1, Part0);
}

TOptional<FU256> FBinaryReader::ReadOptionalU256()
{
    bool hasValue = ReadBool();
    if (hasValue)
    {
        return ReadU256();
    }
    return TOptional<FU256>();
}

FTimeDuration FBinaryReader::ReadTimeDuration()
{
    int64 Micros = ReadInt64();
    return FTimeDuration(Micros);
}

FTimestamp FBinaryReader::ReadTimestamp()
{
    int64 Micros = ReadInt64();
    return FTimestamp(Micros);
}

FStdbConnectionId FBinaryReader::ReadConnectionId()
{
    FU128 Value = ReadU128();
    return FStdbConnectionId(Value);
}

FStdbIdentity FBinaryReader::ReadIdentity()
{
    FU256 Value = ReadU256();
    return FStdbIdentity(Value);
}

TArray<FString> FBinaryReader::ReadStringArray()
{
    return ReadArray<FString>([](FBinaryReader& Reader) { return Reader.ReadString(); });
}

template<typename T>
TArray<T> FBinaryReader::ReadPrimitiveArray()
{
    int32 Length = ReadInt32();
    
    if (Length < 0)
    {
        return TArray<T>();
    }
    
    TArray<T> Result;
    Result.SetNum(Length);
    
    if (Length > 0)
    {
        // Read directly into the array's buffer for performance
        EnsureRemaining(Length * sizeof(T));
        FMemory::Memcpy(Result.GetData(), Data + Position, Length * sizeof(T));
        Position += Length * sizeof(T);
    }
    
    return Result;
}