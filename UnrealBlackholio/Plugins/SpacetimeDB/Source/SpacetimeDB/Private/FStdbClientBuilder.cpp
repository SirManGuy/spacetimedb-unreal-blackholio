#include "FStdbClientBuilder.h"
#include "Kismet/GameplayStatics.h"
#include "UStdbNetworkManager.h"

FStdbClientBuilder::FStdbClientBuilder() {}

FStdbClientBuilder& FStdbClientBuilder::WithUri(const FString& InUri)
{
	Uri = InUri;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::WithModuleName(const FString& InModuleName)
{
	ModuleName = InModuleName;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::WithToken(const FString& InToken)
{
	Token = InToken;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::WithCompression(EStdbCompression InCompression)
{
	Compression = InCompression;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::WithLight(bool bInLight)
{
	bLight = bInLight;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::OnConnect(TFunction<void(FStdbIdentity, FString)> InOnConnect)
{
	OnConnectCb = InOnConnect;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::OnConnectError(TFunction<void(const FString&)> InOnConnectError)
{
	OnConnectErrorCb = InOnConnectError;
	return *this;
}

FStdbClientBuilder& FStdbClientBuilder::OnDisconnect(TFunction<void(const FString&)> InOnDisconnect)
{
	OnDisconnectCb = InOnDisconnect;
	return *this;
}

TSharedPtr<FStdbClientBase> FStdbClientBuilder::Build(UObject* WorldContextObject)
{
	FStdbConnectOptions Options;
	Options.Protocol = TEXT("v1.bsatn.spacetimedb");

	TSharedPtr<FStdbClientBase> Client = MakeShared<FStdbClientBase>(
		Options,
		Token,
		Uri, // Host
		ModuleName,
		Compression,
		bLight
	);

	// Wire up callbacks to FStdbClient (C++ delegates)
	if (OnConnectCb)
	{
		Client->OnConnect.BindLambda([cb = OnConnectCb](FStdbIdentity id, FString token) { cb(id, token); });
	}
	if (OnConnectErrorCb)
	{
		Client->OnConnectError.BindLambda([cb = OnConnectErrorCb](const FString& err) { cb(err); });
	}
	if (OnDisconnectCb)
	{
		Client->OnDisconnect.BindLambda([cb = OnDisconnectCb](const FString& err) { cb(err); });
	}

	if (WorldContextObject)
	{
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
		if (GameInstance)
		{
			UStdbNetworkManager* NetMgr = GameInstance->GetSubsystem<UStdbNetworkManager>();
			if (NetMgr)
			{
				NetMgr->AddConnection(Client);
			}
		}
	}

	Client->Connect();
	return Client;
}

FStdbClientBuilder FStdbClientBuilderEntry::Builder()
{
	return FStdbClientBuilder();
}
