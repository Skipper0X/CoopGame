#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHpChangeSignature, USHealthComponent*, HpComponent, float, Hp, float, HpDelta,
                                             const UDamageType*, DamageType, AController*, InstigatedBy, AActor*, DamageCauser);


UCLASS(ClassGroup = (COOP), meta = (BlueprintSpawnableComponent))
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
	                  AActor* DamageCauser);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHp = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HealthComponent")
	float CurrentHp = 100.0f;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHpChangeSignature OnHpChange;
};
