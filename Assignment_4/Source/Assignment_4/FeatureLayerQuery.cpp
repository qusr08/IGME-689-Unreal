#include "FeatureLayerQuery.h"

AFeatureLayerQuery::AFeatureLayerQuery()
{
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if (SplineComponent)
	{
		SetRootComponent(SplineComponent);
		SplineComponent->SetClosedLoop(true);
	}
}

void AFeatureLayerQuery::BeginPlay()
{
	Super::BeginPlay();

	if (WidgetBlueprintClass)
	{
		UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetBlueprintClass);

		if (WidgetInstance)
		{
			WidgetInstance->AddToViewport();
			widgetTree = WidgetInstance->WidgetTree;
			timerText = (UTextBlock*)(widgetTree->FindWidget("TimerText"));
			checkpointText = (UTextBlock*)(widgetTree->FindWidget("CheckpointText"));
			winLossText = (UTextBlock*)(widgetTree->FindWidget("WinLossText"));
		}
	}

	CheckpointIndex = 1;

	ProcessRequest();
}

void AFeatureLayerQuery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MapComponent || !CarActor || !isLoaded || isFinished)
	{
		return;
	}

	// If the car has moved, start the timer
	if (CarActor->GetActorLocation().Length() >= 30)
	{
		ElapsedTime += DeltaTime;
	}

	if (timerText)
	{
		timerText->SetText(FText::FromString(UKismetStringLibrary::TimeSecondsToString(ElapsedTime)));
	}

	// Get the distance between the car and the current checkpoint position
	// Each checkpoint is just a seem on the spline
	float distance = (MapComponent->TransformPointToEnginePosition(TrackCoordinates[CheckpointIndex]) - CarActor->GetActorLocation()).Length();

	// If the player is close enough, increment to the next checkpoint
	if (distance < 1000)
	{
		// If they have finished all the checkpoints, then stop the timer and show their time on the UI
		if (CheckpointIndex == 0)
		{
			if (ElapsedTime >= 105)
			{
				winLossText->SetText(FText::FromString("You Lose!"));
			}
			else
			{
				winLossText->SetText(FText::FromString("You Win!"));
			}

			isFinished = true;
			return;
		}

		CheckpointIndex = (CheckpointIndex + 1) % TrackCoordinates.Num();

		if (checkpointText)
		{
			int visibleCheckpointIndex = CheckpointIndex - 1;
			checkpointText->SetText(FText::FromString(FString::Printf(TEXT("Checkpoint %d / %d"), (visibleCheckpointIndex == -1 ? TrackCoordinates.Num() : visibleCheckpointIndex), TrackCoordinates.Num())));
		}
	}
}

void AFeatureLayerQuery::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool IsConnected)
{
	// Make sure the data was successfully received
	if (!IsConnected)
	{
		return;
	}

	// Make sure the spline mesh is not null
	if (!SplineMesh || !CarActor || !MapActor)
	{
		return;
	}

	// Set map component and callback
	MapComponent = MapActor->GetMapComponent();
	//MapComponent->OnOriginPositionChanged.AddDynamic(this, &AFeatureLayerQuery::UpdateTrack);

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
		for (int i = 0; i < (featureGeometry.Num() - 1); i++)
		{
			UArcGISSpatialReference* spatialReference = UArcGISSpatialReference::WGS84();
			UArcGISPoint* coordinate = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(featureGeometry[i]->AsArray()[0]->AsNumber(), featureGeometry[i]->AsArray()[1]->AsNumber(), 0.25, spatialReference);
			TrackCoordinates.Add(coordinate);
		}

		// Set the map origin
		MapComponent->SetOriginPosition(TrackCoordinates[0]);

		// Update the track after a delay so the map component can update it's position
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AFeatureLayerQuery::UpdateTrack, 1, false);
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

void AFeatureLayerQuery::UpdateTrack()
{
	// Set the car rotation to face down the track
	FVector direction = MapComponent->TransformPointToEnginePosition(TrackCoordinates[1]) - MapComponent->TransformPointToEnginePosition(TrackCoordinates[0]);
	float angleRadians = atan2(direction.Y, direction.X);
	CarActor->SetActorLocation(FVector::TVector(0, 0, 25));
	CarActor->SetActorRotation(FQuat::MakeFromEuler(FVector::TVector(0, 0, angleRadians * 57.2957795131)));

	// When generating the spline, the seems are all messed up
	// I think this is because I am doing it in C++ and not inside blueprints, as I can't find anyone who has a similar issue
	// For some reason as well my splines always have at least 1 point and I can't get rid of it, which causes the rest of them to get messed up if I try to change it

	// Create spline mesh components
	//SplineComponent->ClearSplinePoints(false);
	/*SplineComponent->SetLocationAtSplinePoint(0, MapComponent->TransformPointToEnginePosition(TrackCoordinates[0]), ESplineCoordinateSpace::Local, true);*/
	SplineComponent->AddSplinePoint(MapComponent->TransformPointToEnginePosition(TrackCoordinates[0]), ESplineCoordinateSpace::Local, true);
	for (int i = 1; i <= TrackCoordinates.Num(); i++)
	{
		if (i < TrackCoordinates.Num())
		{
			SplineComponent->AddSplinePoint(MapComponent->TransformPointToEnginePosition(TrackCoordinates[i]), ESplineCoordinateSpace::Local, true);
		}

		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		//SplineMeshComponent->SetRelativeScale3D(FVector::TVector(4, 4, 1));

		SplineMeshComponent->SetStaticMesh(SplineMesh);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
		SplineMeshComponent->RegisterComponentWithWorld(GetWorld());
		SplineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		const FVector StartPoint = SplineComponent->GetLocationAtSplinePoint(i - 1, ESplineCoordinateSpace::Local);
		const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(i - 1, ESplineCoordinateSpace::Local);
		const FVector EndPoint = SplineComponent->GetLocationAtSplinePoint((i == TrackCoordinates.Num() ? 0 : i), ESplineCoordinateSpace::Local);
		const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint((i == TrackCoordinates.Num() ? 0 : i), ESplineCoordinateSpace::Local);

		SplineMeshComponent->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);
		SplineMeshComponent->SetStartScale(FVector2D::TVector2(4, 1));
		SplineMeshComponent->SetEndScale(FVector2D::TVector2(4, 1));
		SplineMeshComponent->SetForwardAxis(ESplineMeshAxis::X);
	}

	isLoaded = true;

	if (checkpointText)
	{
		checkpointText->SetText(FText::FromString(FString::Printf(TEXT("Checkpoint 0 / %d"), TrackCoordinates.Num())));
	}
}
