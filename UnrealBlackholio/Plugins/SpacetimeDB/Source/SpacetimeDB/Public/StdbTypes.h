#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Templates/SharedPointer.h"
#include "StdbTypes.generated.h"

USTRUCT()
struct SPACETIMEDB_API FStdbConnectOptions
{
	GENERATED_BODY()
	UPROPERTY()
	FString Protocol;
};

UENUM()
enum class EStdbCompression : uint8
{
	None,
	Gzip,
	// Brotli unsupported
};

// Thread-safe queue alias (Unreal's TQueue is thread-safe in MPSC mode)
template<typename T>
using FThreadSafeQueue = TQueue<T, EQueueMode::Mpsc>;
