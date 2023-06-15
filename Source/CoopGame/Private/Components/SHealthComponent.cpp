#include "Components/SHealthComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// ...
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//- Server...
	if (GetOwnerRole() == ROLE_Authority)
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnTakeDamage);
	}

	CurrentHp = DefaultHp;
}

void USHealthComponent::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy,
                                     AActor* DamageCauser)
{
	if (Damage <= 0) return;

	CurrentHp = FMath::Clamp(CurrentHp - Damage, 0.0f, DefaultHp);
	OnHpChange.Broadcast(this, CurrentHp, Damage, DamageType, InstigatedBy, DamageCauser);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, CurrentHp);
}
