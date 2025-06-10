// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.


#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::TakeDamage(const float Damage)
{
	if (Damage <= KINDA_SMALL_NUMBER || !GetOwner() || !IsAlive()) return;

	if (GetOwner()->HasAuthority())
	{
		Server_TakeDamage(Damage);
	}
}

void UHealthComponent::TakeHeal(const float HealAmount)
{
	if (HealAmount <= KINDA_SMALL_NUMBER || !GetOwner() || !IsAlive()) return;
	
	if (GetOwner()->HasAuthority())
	{
		Server_Heal(HealAmount);
	}
}

void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	TakeDamage(Damage);
}

void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy,
	FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const class UDamageType* DamageType, AActor* DamageCauser)
{
	TakeDamage(Damage);
}

void UHealthComponent::HandleRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser)
{
	TakeDamage(Damage);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeHealth();
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, bIsDead);
}

void UHealthComponent::InitializeHealth()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		bIsDead = false;
		SetCurrentHealth(MaxHealth);
	}
}

void UHealthComponent::Server_TakeDamage_Implementation(const float Damage)
{
	if (!IsAlive()) return;
	
	const float NewHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	SetCurrentHealth(NewHealth);
}

bool UHealthComponent::Server_TakeDamage_Validate(const float Damage)
{
	return Damage >= KINDA_SMALL_NUMBER && IsAlive();
}

void UHealthComponent::Server_Heal_Implementation(const float HealAmount)
{
	const float NewHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	SetCurrentHealth(NewHealth);
}

void UHealthComponent::SetCurrentHealth(const float NewHealth)
{
	if (!GetOwner() || !IsAlive()) return; // Запрещаем изменения после смерти

	if (GetOwner()->HasAuthority())
	{
		const float Delta = NewHealth - CurrentHealth;
		if (FMath::Abs(Delta) > KINDA_SMALL_NUMBER) // Игнорируем микроизменения
		{
			CurrentHealth = NewHealth;
			OnHealthChanged.Broadcast(CurrentHealth, Delta);
		}
    
		if (CurrentHealth <= 0.0f && !bIsDead)
		{
			bIsDead = true;
			(void)OnDeath.ExecuteIfBound();
		}
	}
}

void UHealthComponent::OnRep_CurrentHealth()
{
	if (FMath::IsNearlyEqual(LastServerHealth, CurrentHealth, KINDA_SMALL_NUMBER))
	{
		return; // Изменений нет или они слишком малы
	}
	
	const float Delta = CurrentHealth - LastServerHealth;
	OnHealthChanged.Broadcast(CurrentHealth, Delta);
	LastServerHealth = CurrentHealth;
	
	// если здоровье снизилось, но булевый смерти ещё не реплицировался...
	if (CurrentHealth <= 0.0f && !bIsDead) 
	{
		bIsDead = true;
	}
}

void UHealthComponent::OnRep_IsDead()
{
	(void)OnDeath.ExecuteIfBound();
}
