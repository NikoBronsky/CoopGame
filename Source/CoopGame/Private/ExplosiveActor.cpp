// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveActor.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"



// Sets default values
AExplosiveActor::AExplosiveActor()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveActor::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set to physics body to let radial component affect us (eg. when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;

	ExplosionImpulse = 400;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AExplosiveActor::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		//nothingLeft to do, already exploded
		return;
	}

	if (Health <= 0.0f)
	{
		// Explode
		bExploded = true;
		OnRep_Exploded();

		// boost the barrel upwards
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// @TODO: Apply radial damage
		TArray<AActor*> IgnoredActros;
		IgnoredActros.Add(this);

		UGameplayStatics::ApplyRadialDamage(this, 120 , GetActorLocation(), 200, nullptr, IgnoredActros, this, GetInstigatorController(), true);
// 		if (DebugTrackerBotDrawing)
// 		{
// 			DrawDebugSphere(GetWorld(), GetActorLocation(), 200, 12, FColor::Red, false, 2.0f, 0, 1.0f);
// 		}
	}
}

void AExplosiveActor::OnRep_Exploded()
{
	// Play FX and change self material to black
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Override material on mesh with blackened version
	MeshComp->SetMaterial(0, ExplodedMaterial);

}

void AExplosiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosiveActor,bExploded);
}