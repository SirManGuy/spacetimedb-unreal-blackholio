#include "FTimeDuration.h"

FTimeDuration::FTimeDuration()
    : DurationMicros(0)
{
}

FTimeDuration::FTimeDuration(int64 InMicros)
    : DurationMicros(InMicros)
{
}

int64 FTimeDuration::GetMicros() const
{
    return DurationMicros;
}

double FTimeDuration::GetMillis() const
{
    return static_cast<double>(DurationMicros) / static_cast<double>(MICROS_PER_MILLIS);
}

FTimeDuration FTimeDuration::FromMillis(double InMillis)
{
    // Direct port of TypeScript implementation
    int64 MillisInt = static_cast<int64>(InMillis);
    return FTimeDuration(MillisInt * MICROS_PER_MILLIS);
}

FTimeDuration FTimeDuration::operator+(const FTimeDuration& Other) const
{
    return FTimeDuration(DurationMicros + Other.DurationMicros);
}

FTimeDuration FTimeDuration::operator-(const FTimeDuration& Other) const
{
    return FTimeDuration(DurationMicros - Other.DurationMicros);
}

bool FTimeDuration::operator==(const FTimeDuration& Other) const
{
    return DurationMicros == Other.DurationMicros;
}

bool FTimeDuration::operator!=(const FTimeDuration& Other) const
{
    return DurationMicros != Other.DurationMicros;
}

FString FTimeDuration::ToString() const
{
    double MillisValue = GetMillis();
    return FString::Printf(TEXT("%.3f ms"), MillisValue);
}