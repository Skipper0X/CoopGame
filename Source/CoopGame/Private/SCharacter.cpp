#include "SCharacter.h"

#include "Components/CapsuleComponent.h"
#include "CoopGame/CoopGame.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ACharacter::GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HpComponent"));
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultCameraFov = CameraComponent->FieldOfView;

	SpawnWeapon();

	HealthComponent->OnHpChange.AddDynamic(this, &ASCharacter::OnHpChange);
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float TargetFov = ZoomInCamera ? ZoomCameraFov : DefaultCameraFov;
	const float FovToSet = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFov, DeltaTime, ZoomInterpSpeed);

	CameraComponent->SetFieldOfView(FovToSet);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComponent->GetComponentLocation();
}

void ASCharacter::MoveForward(const float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(const float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	ZoomInCamera = true;
}

void ASCharacter::EndZoom()
{
	ZoomInCamera = false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ASCharacter::StartFiring()
{
	if (CurrentWeapon) CurrentWeapon->StartFiring();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ASCharacter::StopFiring()
{
	if (CurrentWeapon) CurrentWeapon->StopFiring();
}

void ASCharacter::OnHpChange(USHealthComponent* HpComponent, float Hp, float HpDelta, const UDamageType* DamageType,
                             AController* InstigatedBy, AActor* DamageCauser)
{
	if (Hp <= 0.0f && IsDead == false)
	{
		IsDead = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

void ASCharacter::SpawnWeapon()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponRef, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	CurrentWeapon->SetOwner(this);
	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
}


// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);

	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFiring);
}
