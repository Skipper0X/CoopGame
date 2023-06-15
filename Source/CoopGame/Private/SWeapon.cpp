#include "SWeapon.h"

#include "CoopGame/CoopGame.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
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
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	if (auto WeaponOwner = GetOwner())
	{
		FVector Location;
		FRotator Rotation;
		WeaponOwner->GetActorEyesViewPoint(Location, Rotation);

		const FVector TraceEnd = Location + (Rotation.Vector() * 1000);
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

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

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			//- Apply Damage To The Hit Actor...
			float DamageToApply = DamageAmount;
			if (SurfaceType == SURFACE_FLESH_VULNERABLE)
			{
				DamageToApply *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, HitFromDirection, Hit, EventInstigator, this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), Location, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayWeaponEffects(TracerEndPoint);

		if (auto Pawn = Cast<APawn>(WeaponOwner))
		{
			if (auto Controller = Pawn->GetController())
			{
				if (auto PC = Cast<APlayerController>(Controller))
				{
					PC->ClientStartCameraShake(FireCameraShake);
				}
			}
		}

		if (GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	//- Returning True, Not Focusing On Anti Cheat Yet...
	return true;
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayWeaponEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::PlayImpactEffects(const EPhysicalSurface SurfaceType, const FVector& ImpactPoint) const
{
	//- Player Hit Effect On The Surface Type Base...
	UParticleSystem* ImpactEffectToPlay;
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

	if (ImpactEffectToPlay)
	{
		const FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = (ImpactPoint - MuzzleLocation);
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffectToPlay, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::PlayWeaponEffects(const FVector& TracerEndPoint) const
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		const FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

		if (const auto Tracer = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation))
		{
			Tracer->SetVectorParameter("Target", TracerEndPoint);
		}
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
