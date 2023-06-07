#include "SWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));

	MuzzleSocketName = "MuzzleSocket";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ASWeapon::Fire()
{
	if (auto WeaponOwner = GetOwner())
	{
		FVector Location;
		FRotator Rotation;
		WeaponOwner->GetActorEyesViewPoint(Location, Rotation);

		const FVector TraceEnd = Location + (Rotation.Vector() * 1000);
		FVector TracerEndPoint = TraceEnd;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		if (FHitResult Hit; GetWorld()->LineTraceSingleByChannel(Hit, Location, TraceEnd, ECC_Visibility, QueryParams))
		{
			auto HitActor = Hit.GetActor();
			auto EventInstigator = WeaponOwner->GetInstigatorController();
			auto HitFromDirection = Rotation.Vector();
			TracerEndPoint = Hit.ImpactPoint;

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, HitFromDirection, Hit, EventInstigator, this, DamageType);

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		DrawDebugLine(GetWorld(), Location, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
		}

		if (TracerEffect)
		{
			FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
			UParticleSystemComponent* Tracer = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			Tracer->SetVectorParameter("Target", TracerEndPoint);
		}
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
