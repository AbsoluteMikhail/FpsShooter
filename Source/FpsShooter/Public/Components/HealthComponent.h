// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// @param Health - текущее значение здоровья
// @param AmountChange - изменение здоровья (положительное - лечение, отрицательное - урон)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, Health, float, AmountChange);
DECLARE_DELEGATE(FOnDeathSignature); // Только один подписчик

// Компонент отвечает за обработку здоровья
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSSHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	// Делегат изменения здоровья для блюпринтов
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChangedSignature OnHealthChanged;
	// Делегат смерти для использования в С++
	FOnDeathSignature OnDeath;
	
	// Получить текущее здоровье
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	// Получить максимально возможное здоровье
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	// Проверить, жив ли владелец
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Health")
	FORCEINLINE bool IsAlive() const { return !bIsDead; }

	/**
	 * @brief Наносит урон владельцу компонента
	 * @param Damage - величина урона (должна быть > 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(const float Damage);

	// Лечение
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeHeal(const float HealAmount);

	// Функции обёртки для подвязки на делегаты
	UFUNCTION()
	void HandleAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser );
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser );
	UFUNCTION()
	void HandleRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser );
	
protected:
	virtual void BeginPlay() override;
	
	// Сетевые функции
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Максимальное здоровье
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.0f;
	// Текущее здоровье
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth = 100.0f;
	// Флаг смерти
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;
	
	// Хранит предыдущее значение здоровья после последней репликации, чтобы рассчитать дельту
	float LastServerHealth = 100.0f;

	// Начальная инициализация
	void InitializeHealth();

	// Чисто серверная логика урона с валидацией
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TakeDamage(const float Damage);
	// Чисто серверная логика лечения
	UFUNCTION(Server, Reliable)
	void Server_Heal(const float HealAmount);

	// Установить текущее здоровье
	void SetCurrentHealth(const float NewHealth);
	
	// Репликация здоровья
	UFUNCTION()
	void OnRep_CurrentHealth();

	// Репликация флага смерти
	UFUNCTION()
	void OnRep_IsDead();
	
};
