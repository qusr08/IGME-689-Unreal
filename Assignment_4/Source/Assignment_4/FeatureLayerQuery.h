#pragma once

#include <cmath>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/GeoCoord/GeoPosition.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetStringLibrary.h"
#include "FeatureLayerQuery.generated.h"

UCLASS()
class ASSIGNMENT_4_API AFeatureLayerQuery : public AActor
{
	GENERATED_BODY()

public:
	AFeatureLayerQuery();

	virtual void Tick(float DeltaTime) override;

	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected);
	virtual void ProcessRequest();
	virtual void UpdateTrack();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TrackLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TrackName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int TrackLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UArcGISPoint*> TrackCoordinates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int ActiveFeatureIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CheckpointIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ElapsedTime;

	UPROPERTY(VisibleAnywhere)
	USplineComponent* SplineComponent;

	UPROPERTY(VisibleAnywhere)
	UArcGISMapComponent* MapComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SplineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* CarActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AArcGISMapActor* MapActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetBlueprintClass;

protected:
	virtual void BeginPlay() override;

private:
	FString weblink = "https://services2.arcgis.com/yL7v93RXrxlqkeDx/arcgis/rest/services/F1_World_Championship_Circuits/FeatureServer/0/query?f=geojson&where=1=1&outfields=*";
	bool isLoaded = false;

	bool isFinished = false;

	UWidgetTree* widgetTree;
	UTextBlock* timerText;
	UTextBlock* checkpointText;
	UTextBlock* winLossText;
};
