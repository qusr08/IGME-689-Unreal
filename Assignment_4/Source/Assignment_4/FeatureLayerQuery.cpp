#include "FeatureLayerQuery.h"

#include "Kismet/GameplayStatics.h"

AFeatureLayerQuery::AFeatureLayerQuery()
{
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if (SplineComponent)
	{
		SetRootComponent(SplineComponent);
	}
}

void AFeatureLayerQuery::BeginPlay()
{
	Super::BeginPlay();

	ProcessRequest();
}

void AFeatureLayerQuery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFeatureLayerQuery::OnConstruction(const FTransform& Transform)
{

}

void AFeatureLayerQuery::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected)
{
	// Make sure the data was successfully received
	if (!IsConnected)
	{
		return;
	}

	// Make sure the spline mesh is not null
	if (!SplineMesh)
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

		// Make sure the active index is valid
		if (ActiveFeatureIndex < 0 || ActiveFeatureIndex >= featureObjects.Num())
		{
			return;
		}

		// Set track properties
		auto featureProperties = featureObjects[ActiveFeatureIndex]->AsObject()->GetObjectField(TEXT("properties"));
		TrackLocation = featureProperties->GetStringField(TEXT("location"));
		TrackName = featureProperties->GetStringField(TEXT("name"));
		TrackLength = featureProperties->GetIntegerField(TEXT("length"));

		// Set track geometry points
		auto featureGeometry = featureObjects[ActiveFeatureIndex]->AsObject()->GetObjectField(TEXT("geometry"))->GetArrayField(TEXT("coordinates"));
		for (int i = 0; i < featureGeometry.Num(); i++)
		{
			FVector featureCoordinate;
			featureCoordinate.X = featureGeometry[i]->AsArray()[0]->AsNumber();
			featureCoordinate.Y = featureGeometry[i]->AsArray()[1]->AsNumber();
			SplineComponent->AddSplinePoint(featureCoordinate, ESplineCoordinateSpace::Local, true);

			if (i == 0) {
				/*auto origin = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(-74.054921, 40.691242, 3000, UArcGISSpatialReference::WGS84());
				const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
				const auto mapComponent = Cast<AArcGISMapActor>(mapComponentActor)->GetMapComponent();
				mapComponent->SetOrigin()*/
			}
		}

		// Create spline mesh components
		for (int i = 0; i < (SplineComponent->GetNumberOfSplinePoints() - 1); i++)
		{
			USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());

			SplineMeshComponent->SetStaticMesh(SplineMesh);
			SplineMeshComponent->SetMobility(EComponentMobility::Movable);
			SplineMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
			SplineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

			const FVector StartPoint = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
			const FVector EndPoint = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
			const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

			SplineMeshComponent->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);
			SplineMeshComponent->SetForwardAxis(ForwardAxis);
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

