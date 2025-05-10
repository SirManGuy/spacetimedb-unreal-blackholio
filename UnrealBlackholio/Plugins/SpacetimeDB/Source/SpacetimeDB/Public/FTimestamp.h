#pragma once

#include "CoreMinimal.h"

/**
 * A point in time, represented as microseconds since the Unix epoch.
 * Direct port of the TypeScript Timestamp class.
 */
struct SPACETIMEDB_API FTimestamp
{
private:
	int64 MicrosSinceEpoch;

public:
	FTimestamp();

	explicit FTimestamp(int64 InMicros);

	int64 GetMicros() const;

	static FTimestamp Now();

	bool operator==(const FTimestamp& Other) const;

	bool operator!=(const FTimestamp& Other) const;

	bool operator<(const FTimestamp& Other) const;

	bool operator>(const FTimestamp& Other) const;

	bool operator<=(const FTimestamp& Other) const;

	bool operator>=(const FTimestamp& Other) const;

	FDateTime ToDateTime() const;

	static FTimestamp FromDateTime(const FDateTime& DateTime);

	FString ToString() const;
};