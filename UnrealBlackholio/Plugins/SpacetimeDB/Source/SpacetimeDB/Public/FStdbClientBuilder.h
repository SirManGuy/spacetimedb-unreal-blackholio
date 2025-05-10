#pragma once

#include "CoreMinimal.h"
#include "StdbTypes.h"
#include "FStdbClientBase.h"
#include "UStdbNetworkManager.h"
#include "Kismet/GameplayStatics.h"

/**
 * FStdbClientBuilder: Fluent builder for FStdbClient, mirroring the C#/TS SDKs... it's kind of ugly in Unreal
 */
class SPACETIMEDB_API FStdbClientBuilder
{
public:
	FStdbClientBuilder();

	// Chainable config methods
	FStdbClientBuilder& WithUri(const FString& InUri);
	FStdbClientBuilder& WithModuleName(const FString& InModuleName);
	FStdbClientBuilder& WithToken(const FString& InToken);
	FStdbClientBuilder& WithCompression(EStdbCompression InCompression);
	FStdbClientBuilder& WithLight(bool bInLight);

	// Chainable callback methods
	FStdbClientBuilder& OnConnect(TFunction<void(FStdbIdentity /*Identity*/, FString /*Token*/)> InOnConnect);
	FStdbClientBuilder& OnConnectError(TFunction<void(const FString&)> InOnConnectError);
	FStdbClientBuilder& OnDisconnect(TFunction<void(const FString&)> InOnDisconnect);

	// Build and register with the network manager
	TSharedPtr<FStdbClientBase> Build(UObject* WorldContextObject);

private:
	FString Uri;
	FString ModuleName;
	FString Token;
	EStdbCompression Compression = EStdbCompression::None;
	bool bLight = false;
	
	TFunction<void(FStdbIdentity, FString)> OnConnectCb;
	TFunction<void(const FString&)> OnConnectErrorCb;
	TFunction<void(const FString&)> OnDisconnectCb;
};

class FStdbClientBase;
struct FStdbClientBuilderEntry
{
	static FStdbClientBuilder Builder();
};
