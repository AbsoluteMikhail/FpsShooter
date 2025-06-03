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
	if (Damage <= KINDA_SMALL_NUMBER || !IsAlive() || !GetOwner()) return;

	if (GetOwner()->HasAuthority())
	{
		Server_TakeDamage(Damage);
	}
}

void UHealthComponent::TakeHeal(const float HealAmount)
{
	if (HealAmount <= KINDA_SMALL_NUMBER || !IsAlive() || !GetOwner()) return;
	
	if (GetOwner()->HasAuthority())
	{
		Server_Heal(HealAmount);
	}
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
	return Damage >= KINDA_SMALL_NUMBER && Damage <= MaxHealth && IsAlive();
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
		const float Delta = CurrentHealth - NewHealth;
		if (FMath::Abs(Delta) > KINDA_SMALL_NUMBER) // Игнорируем микроизменения
		{
			CurrentHealth = NewHealth;
			// Делегат здоровье изменилось для сервера
		}
    
		if (CurrentHealth <= 0.0f)
		{
			bIsDead = true;
			// Делегат смерти для сервера
		}
	}
}

void UHealthComponent::OnRep_CurrentHealth()
{
	// Делегат здоровье изменилось для клиента

	if (CurrentHealth <= 0.0f && !bIsDead) 
	{
		bIsDead = true;
	}
}

void UHealthComponent::OnRep_IsDead()
{
	// Делегат смерти для клиента
}
