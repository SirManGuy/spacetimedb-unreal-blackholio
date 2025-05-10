// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASpacetimeDbTester.generated.h"

class FStdbClientBase;

UCLASS()
class UNREALBLACKHOLIO_API AASpacetimeDbTester : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AASpacetimeDbTester();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	TSharedPtr<FStdbClientBase> Conn;
};
