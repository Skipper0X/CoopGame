#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "SCharacter.generated.h"

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FVector GetPawnViewLocation() const override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void BeginZoom();

	void EndZoom();

	void StartFiring();

	void StopFiring();

	UFUNCTION()
	void OnHpChange(USHealthComponent* HpComponent, float Hp, float HpDelta, const UDamageType* DamageType, AController* InstigatedBy,
	                AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomCameraFov = 65.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0))
	float ZoomInterpSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> WeaponRef;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName = "WeaponSocket";

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool IsDead = false;

private:
	void SpawnWeapon();

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	bool ZoomInCamera = false;
	float DefaultCameraFov = 90.0f;
};
