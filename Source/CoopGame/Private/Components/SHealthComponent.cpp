#include "Components/SHealthComponent.h"

#include "GameFramework/GameModeBase.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// ...
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHp = DefaultHp;
	// ...
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeDamage);
}

void USHealthComponent::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
                                     AActor* DamageCauser)
{
	if (Damage <= 0) return;

	CurrentHp = FMath::Clamp(CurrentHp - Damage, 0, DefaultHp);
	OnHpChange.Broadcast(this, CurrentHp, Damage, DamageType, InstigatedBy, DamageCauser);
}
