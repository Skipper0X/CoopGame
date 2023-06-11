#include "SWeapon.h"

#include "CoopGame/CoopGame.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

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

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::StartFiring()
{
	const float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(AutomaticFireTimeHandle, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFiring()
{
	GetWorldTimerManager().ClearTimer(AutomaticFireTimeHandle);
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
		QueryParams.bReturnPhysicalMaterial = true;

		if (FHitResult Hit; GetWorld()->LineTraceSingleByChannel(Hit, Location, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			auto HitActor = Hit.GetActor();
			auto EventInstigator = WeaponOwner->GetInstigatorController();
			auto HitFromDirection = Rotation.Vector();
			TracerEndPoint = Hit.ImpactPoint;

			auto SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			//- Apply Damage To The Hit Actor...
			float DamageToApply = DamageAmount;
			if (SurfaceType == SURFACE_FLESH_VULNERABLE)
			{
				DamageToApply *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, HitFromDirection, Hit, EventInstigator, this, DamageType);

			//- Player Hit Effect On The Surface Type Base...
			UParticleSystem* ImpactEffectToPlay = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESH_DEFAULT:
			case SURFACE_FLESH_VULNERABLE:
				ImpactEffectToPlay = FleshImpactEffect;
				break;

			default:
				ImpactEffectToPlay = DefaultImpactEffect;
				break;
			}

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffectToPlay, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
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

		LastFireTime = GetWorld()->TimeSeconds;
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
