#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "FStdbClientBase.h"
#include "UStdbClientProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStdbOnConnect, UObject*, ClientProxy, FString, Identity, FString, Token);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStdbOnConnectError, FString, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStdbOnDisconnect, UObject*, ClientProxy, FString, Error);

/*
 * TODO: Set up to actually be used...
 */
UCLASS(BlueprintType)
class SPACETIMEDB_API UStdbClientProxy : public UObject
{
    GENERATED_BODY()
public:
    // Blueprint events
    UPROPERTY(BlueprintAssignable, Category="SpacetimeDB|Events")
    FStdbOnConnect OnConnect;

    UPROPERTY(BlueprintAssignable, Category="SpacetimeDB|Events")
    FStdbOnConnectError OnConnectError;

    UPROPERTY(BlueprintAssignable, Category="SpacetimeDB|Events")
    FStdbOnDisconnect OnDisconnect;

    // Native pointer to the underlying client
    TSharedPtr<FStdbClientBase> NativeClient;

    // Helper to create a proxy from a native client
    static UStdbClientProxy* Create(UObject* Outer, TSharedPtr<FStdbClientBase> NativeClient);
};
