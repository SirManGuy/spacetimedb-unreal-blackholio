#include "FTimestamp.h"

FTimestamp::FTimestamp()
    : MicrosSinceEpoch(0)
{
}

FTimestamp::FTimestamp(int64 InMicros)
    : MicrosSinceEpoch(InMicros)
{
}

int64 FTimestamp::GetMicros() const
{
    return MicrosSinceEpoch;
}

FTimestamp FTimestamp::Now()
{
    // Get current time as FDateTime
    FDateTime CurrentTime = FDateTime::UtcNow();
    
    // Convert to our timestamp format
    return FromDateTime(CurrentTime);
}


bool FTimestamp::operator==(const FTimestamp& Other) const
{
    return MicrosSinceEpoch == Other.MicrosSinceEpoch;
}

bool FTimestamp::operator!=(const FTimestamp& Other) const
{
    return MicrosSinceEpoch != Other.MicrosSinceEpoch;
}

bool FTimestamp::operator<(const FTimestamp& Other) const
{
    return MicrosSinceEpoch < Other.MicrosSinceEpoch;
}

bool FTimestamp::operator>(const FTimestamp& Other) const
{
    return MicrosSinceEpoch > Other.MicrosSinceEpoch;
}

bool FTimestamp::operator<=(const FTimestamp& Other) const
{
    return MicrosSinceEpoch <= Other.MicrosSinceEpoch;
}

bool FTimestamp::operator>=(const FTimestamp& Other) const
{
    return MicrosSinceEpoch >= Other.MicrosSinceEpoch;
}

FDateTime FTimestamp::ToDateTime() const
{
    // Unreal's FDateTime uses ticks (100 nanoseconds) since January 1, 0001
    // Unix epoch is January 1, 1970
    
    // First, convert microseconds to ticks (1 microsecond = 10 ticks)
    int64 Ticks = MicrosSinceEpoch * 10;
    
    // Add the offset from 0001-01-01 to Unix epoch (1970-01-01)
    static const int64 TicksFromYear0001ToUnixEpoch = FDateTime(1970, 1, 1, 0, 0, 0, 0).GetTicks();
    Ticks += TicksFromYear0001ToUnixEpoch;
    
    return FDateTime(Ticks);
}

FTimestamp FTimestamp::FromDateTime(const FDateTime& DateTime)
{
    // Convert from FDateTime ticks to Unix epoch microseconds
    static const int64 TicksFromYear0001ToUnixEpoch = FDateTime(1970, 1, 1, 0, 0, 0, 0).GetTicks();
    
    // Get ticks since year 0001
    int64 Ticks = DateTime.GetTicks();
    
    // Convert to ticks since Unix epoch
    Ticks -= TicksFromYear0001ToUnixEpoch;
    
    // Convert to microseconds (10 ticks = 1 microsecond)
    int64 Micros = Ticks / 10;
    
    return FTimestamp(Micros);
}

FString FTimestamp::ToString() const
{
    FDateTime DateTime = ToDateTime();
    return DateTime.ToString(TEXT("%Y-%m-%d %H:%M:%S.%s"));
}