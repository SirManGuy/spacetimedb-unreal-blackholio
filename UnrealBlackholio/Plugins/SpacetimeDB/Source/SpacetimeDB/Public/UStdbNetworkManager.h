#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "FStdbClientBase.h"
#include "UStdbNetworkManager.generated.h"

UCLASS()
class SPACETIMEDB_API UStdbNetworkManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    bool AddConnection(TSharedPtr<FStdbClientBase> Conn);
    bool RemoveConnection(TSharedPtr<FStdbClientBase> Conn);

    void ForEachConnection(TFunctionRef<void(TSharedPtr<FStdbClientBase>)> Func);

    void TickNetwork(float DeltaTime);

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    // Custom tick function struct
    struct FStdbTickFunction : public FTickFunction
    {
        UStdbNetworkManager* Manager = nullptr;
        virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override
        {
            if (Manager)
            {
                Manager->TickNetwork(DeltaTime);
            }
        }
        virtual FString DiagnosticMessage() override { return TEXT("UStdbNetworkManager::FStdbTickFunction"); }
    };

    FStdbTickFunction TickFunction;

    void RegisterTick();
    void UnregisterTick();

private:
    TArray<TSharedPtr<FStdbClientBase>> ActiveConnections;
};
