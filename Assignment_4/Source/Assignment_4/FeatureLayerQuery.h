// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "FeatureLayerQuery.generated.h"

USTRUCT(BlueprintType)
struct FCoordinate
{
	GENERATED_BODY();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Longitude;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Latitude;
};

USTRUCT(BlueprintType)
struct FFeature
{
	GENERATED_BODY();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Properties")
	FString ID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Properties")
	FString Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Properties")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Properties")
	int TrackLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FCoordinate> Coordinates;
};

UCLASS()
class ASSIGNMENT_4_API AFeatureLayerQuery : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFeatureLayerQuery();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected);
	virtual void ProcessRequest();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFeature> Features;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString weblink = "";

};
