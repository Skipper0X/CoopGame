#include "SWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines For Weapns"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));

	MuzzleSocketName = "MuzzleSocket";
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
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

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

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), Location, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}


		PlayMuzzleVfx();
		PlayTracerVfx(TracerEndPoint, MuzzleLocation);


		if (auto Pawn = Cast<APawn>(WeaponOwner))
		{
			if (auto PC = Cast<APlayerController>(Pawn->GetController()))
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}
	}
}

void ASWeapon::PlayMuzzleVfx() const
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}
}

void ASWeapon::PlayTracerVfx(const FVector& TracerEndPoint, const FVector& MuzzleLocation) const
{
	if (TracerEffect)
	{
		UParticleSystemComponent* Tracer = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		Tracer->SetVectorParameter("Target", TracerEndPoint);
	}
}
