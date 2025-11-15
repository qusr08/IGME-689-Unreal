// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
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

UCLASS()
class ASSIGNMENT_4_API AFeatureLayerQuery : public AActor
{
	GENERATED_BODY()

public:
	AFeatureLayerQuery();

	virtual void Tick(float DeltaTime) override;
	void OnConstruction(const FTransform& Transform) override;

	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected);
	virtual void ProcessRequest();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TrackLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TrackName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int TrackLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int ActiveFeatureIndex;

	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SplineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FString weblink = "https://services2.arcgis.com/yL7v93RXrxlqkeDx/arcgis/rest/services/F1_World_Championship_Circuits/FeatureServer/0/query?f=geojson&where=1=1&outfields=*";

};
