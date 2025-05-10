#include "UStdbClientProxy.h"

UStdbClientProxy* UStdbClientProxy::Create(UObject* Outer, TSharedPtr<FStdbClientBase> NativeClient)
{
    UStdbClientProxy* Proxy = NewObject<UStdbClientProxy>(Outer);
    Proxy->NativeClient = NativeClient;
    if (NativeClient)
    {
        /// TODO: Wire up the BlueprintProxy - needs to be added to the FStdbClient
        
        //NativeClient->BlueprintProxy = Proxy;
        // Wire up C++ delegates to Blueprint events
        // NativeClient->OnConnect.BindLambda([Proxy](TSharedPtr<FStdbClient> Client, FString Identity, FString Token)
        // {
        //     if (Proxy)
        //     {
        //         Proxy->OnConnect.Broadcast(Proxy, Identity, Token);
        //     }
        // });
        // NativeClient->OnConnectError.BindLambda([Proxy](const FString& Error)
        // {
        //     if (Proxy)
        //     {
        //         Proxy->OnConnectError.Broadcast(Error);
        //     }
        // });
        // NativeClient->OnDisconnect.BindLambda([Proxy](TSharedPtr<FStdbClient> Client, const FString& Error)
        // {
        //     if (Proxy)
        //     {
        //         Proxy->OnDisconnect.Broadcast(Proxy, Error);
        //     }
        // });
    }
    return Proxy;
}
