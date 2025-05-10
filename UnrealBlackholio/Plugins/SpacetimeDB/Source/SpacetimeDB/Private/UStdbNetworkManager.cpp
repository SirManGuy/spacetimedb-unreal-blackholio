#include "UStdbNetworkManager.h"

#include "LogStdb.h"
#include "Engine/World.h"

void UStdbNetworkManager::RegisterTick()
{
    if (UWorld* World = GetWorld())
    {
        if (ULevel* Level = World->PersistentLevel) {
            TickFunction.Manager = this;
            TickFunction.bCanEverTick = true;
            TickFunction.bStartWithTickEnabled = true;
            TickFunction.RegisterTickFunction(Level);
        }
    }
}

void UStdbNetworkManager::UnregisterTick()
{
    TickFunction.UnRegisterTickFunction();
}

bool UStdbNetworkManager::AddConnection(TSharedPtr<FStdbClientBase> Conn)
{
    if (ActiveConnections.Contains(Conn)) return false;
    ActiveConnections.Add(Conn);
    return true;
}

bool UStdbNetworkManager::RemoveConnection(TSharedPtr<FStdbClientBase> Conn)
{
    return ActiveConnections.Remove(Conn) > 0;
}

void UStdbNetworkManager::ForEachConnection(TFunctionRef<void(TSharedPtr<FStdbClientBase>)> Func)
{
    for (int32 i = ActiveConnections.Num() - 1; i >= 0; --i)
    {
        Func(ActiveConnections[i]);
    }
}


void UStdbNetworkManager::TickNetwork(float DeltaTime)
{
    ForEachConnection([](TSharedPtr<FStdbClientBase> Conn)
    {
        if (Conn.IsValid())
        {
            Conn->FrameTick();
        }
    });
}

void UStdbNetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogStdb, Log, TEXT("UStdbNetworkManager Initialize"));
    Super::Initialize(Collection);
    RegisterTick();
}

void UStdbNetworkManager::Deinitialize()
{
    UE_LOG(LogStdb, Log, TEXT("UStdbNetworkManager Deinitialize"));
    ForEachConnection([](TSharedPtr<FStdbClientBase> Conn)
    {
        if (Conn.IsValid())
        {
            Conn->Shutdown();
        }
    });
    ActiveConnections.Empty();
    UnregisterTick();
}
