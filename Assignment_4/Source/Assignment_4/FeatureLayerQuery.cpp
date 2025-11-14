// Fill out your copyright notice in the Description page of Project Settings.

// https://services2.arcgis.com/yL7v93RXrxlqkeDx/arcgis/rest/services/F1_World_Championship_Circuits/FeatureServer/0/query?f=geojson&where=1=1&outfields=*

#include "FeatureLayerQuery.h"

// Sets default values
AFeatureLayerQuery::AFeatureLayerQuery()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFeatureLayerQuery::BeginPlay()
{
	Super::BeginPlay();

	ProcessRequest();
}

// Called every frame
void AFeatureLayerQuery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFeatureLayerQuery::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected)
{
	if (!IsConnected)
	{
		return;
	}

	// Create a reader so we can parse the JSON data
	TSharedPtr<FJsonObject> responseObject;
	const auto responseBody = Response->GetContentAsString();
	auto reader = TJsonReaderFactory<>::Create(responseBody);

	if (FJsonSerializer::Deserialize(reader, responseObject))
	{
		// Get a list of the features
		auto featureObjects = responseObject->GetArrayField(TEXT("features"));

		for (auto feature : featureObjects)
		{
			FFeature currentFeature;

			auto featureProperties = feature->AsObject()->GetObjectField(TEXT("properties"));
			currentFeature.ID = featureProperties->GetStringField(TEXT("id"));
			currentFeature.Location = featureProperties->GetStringField(TEXT("location"));
			currentFeature.Name = featureProperties->GetStringField(TEXT("name"));
			currentFeature.TrackLength = featureProperties->GetIntegerField(TEXT("length"));

			auto featureGeometry = feature->AsObject()->GetObjectField(TEXT("geometry"))->GetArrayField(TEXT("coordinates"));
			for (auto coordinate : featureGeometry)
			{
				FCoordinate featureCoordinate;
				featureCoordinate.Longitude = coordinate->AsArray()[0]->AsNumber();
				featureCoordinate.Latitude = coordinate->AsArray()[1]->AsNumber();
				currentFeature.Coordinates.Add(featureCoordinate);
			}

			Features.Add(currentFeature);
		}
	}
}

void AFeatureLayerQuery::ProcessRequest()
{
	// Send a request to get weblink data
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayerQuery::OnResponseReceived);
	request->SetURL(weblink);
	request->SetVerb("Get");
	request->ProcessRequest();
}

