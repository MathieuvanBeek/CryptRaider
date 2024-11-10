// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
    Super::BeginPlay();
    
    UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
    if (PhysicsHandle)
    {
        // UE_LOG(LogTemp, Warning, TEXT("Physics Handle found on %s"), *GetOwner()->GetName());
    }
    else
    {
        // UE_LOG(LogTemp, Error, TEXT("No Physics Handle found on %s"), *GetOwner()->GetName());
    }
}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FHitResult HitResult;
    bool bIsItemInReach = GetGrabbableInReach(HitResult);

    // Debug: Log if item is in reach
    if (bIsItemInReach && HitResult.GetActor())
    {
        // UE_LOG(LogTemp, Warning, TEXT("Item in reach: %s"), *HitResult.GetActor()->GetName());
    }

    // Existing code to update the held object’s location
    UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
    if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
    {
        FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
        PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
    }
}

void UGrabber::Grab()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle == nullptr)
	{
		// UE_LOG(LogTemp, Error, TEXT("PhysicsHandle is not available"));
		return;
	}

	FHitResult HitResult;
	bool HasHit = GetGrabbableInReach(HitResult);

	// Debug: Check if we hit something
	UE_LOG(LogTemp, Warning, TEXT("HasHit: %s"), HasHit ? TEXT("true") : TEXT("false"));
	if (HasHit && HitResult.GetActor())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("No actor hit within reach"));
		return;
	}

	if (HasHit)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent->SetSimulatePhysics(true);
		HitComponent->WakeAllRigidBodies();
		AActor* HitActor = HitResult.GetActor();
		HitActor->Tags.Add("Grabbed");
		HitActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent,
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation()
		);
		// UE_LOG(LogTemp, Warning, TEXT("Grabbed Actor: %s"), *HitActor->GetName());
	}
}

void UGrabber::Release()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		AActor* GrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		GrabbedActor->Tags.Remove("Grabbed");
		PhysicsHandle->ReleaseComponent();
		// UE_LOG(LogTemp, Warning, TEXT("Released Actor: %s"), *GrabbedActor->GetName());
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("Nothing to release"));
	}
}

UPhysicsHandleComponent* UGrabber::GetPhysicsHandle() const
{
	UPhysicsHandleComponent* Result = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (Result == nullptr)
	{
		// UE_LOG(LogTemp, Error, TEXT("Grabber requires a UPhysicsHandleComponent"));
	}
	return Result;
}

bool UGrabber::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;

	// Draw Debug: Visualize the line trace and the sphere at the end point
	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);
	// DrawDebugSphere(GetWorld(), End, 5, 5, FColor::Blue, false, 0.1);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	// Perform the trace
	bool bHasHit = GetWorld()->SweepSingleByChannel(
		OutHitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		Sphere
	);

	// Debug: Log if the trace hit something
	if (bHasHit && OutHitResult.GetActor())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Trace hit: %s"), *OutHitResult.GetActor()->GetName());
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("No trace hit within reach"));
	}

	return bHasHit;
}

void UGrabber::AdjustHoldDistance(float ScrollAmount)
{
	HoldDistance = FMath::Clamp(HoldDistance + ScrollAmount * 10, 50.0f, 250.0f);
	// UE_LOG(LogTemp, Warning, TEXT("Hold distance adjusted to: %f"), HoldDistance);
}
