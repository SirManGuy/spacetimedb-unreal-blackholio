#pragma once

#include "CoreMinimal.h"

/**
 * A difference between two points in time, represented as a number of microseconds.
 * Direct port of the TypeScript TimeDuration class.
 */
struct SPACETIMEDB_API FTimeDuration
{
private:
	int64 DurationMicros;

	static constexpr int64 MICROS_PER_MILLIS = 1000;

public:
	FTimeDuration();

	explicit FTimeDuration(int64 InMicros);

	int64 GetMicros() const;

	double GetMillis() const;

	static FTimeDuration FromMillis(double InMillis);

	FTimeDuration operator+(const FTimeDuration& Other) const;
	FTimeDuration operator-(const FTimeDuration& Other) const;
	bool operator==(const FTimeDuration& Other) const;
	bool operator!=(const FTimeDuration& Other) const;

	FString ToString() const;
};