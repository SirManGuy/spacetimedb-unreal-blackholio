// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealBlackholio/Public/ASpacetimeDbTester.h"

#include "FStdbClientBuilder.h"
#include "FStdbIdentity.h"
#include "UnrealBlackholio/UnrealBlackholio.h"

class FStdbClientBuilder;
// Sets default values
AASpacetimeDbTester::AASpacetimeDbTester()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AASpacetimeDbTester::BeginPlay()
{
	Super::BeginPlay();

	// Use the builder to configure and start a connection
	Conn = FStdbClientBuilder()
		.WithUri(TEXT("ws://127.0.0.1:3000"))
		.WithModuleName(TEXT("unrealblackholio"))
		.WithToken(TEXT(""))
		.WithCompression(EStdbCompression::None)
		.OnConnect([this](FStdbIdentity Identity, FString Token) {
			UE_LOG(LogUbo, Log, TEXT("Connected! Token: %s"), *Token);
			Conn->LegacySubscribe();
		})
		// .OnConnectError([](const FString& Error) {
		// 	UE_LOG(LogTemp, Error, TEXT("Connection error: %s"), *Error);
		// })
		// .OnDisconnect([](TSharedPtr<FStdbClient> Client, const FString& Error) {
		// 	UE_LOG(LogTemp, Warning, TEXT("Disconnected: %s"), *Error);
		// })
		.Build(this);
}

void AASpacetimeDbTester::Destroyed()
{
	if (Conn.IsValid())
	{
		Conn->Shutdown();
		Conn.Reset();
	}
	Super::Destroyed();
}

// Called every frame
void AASpacetimeDbTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

