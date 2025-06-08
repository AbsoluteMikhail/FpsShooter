// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameframework/Character.h"
#include "FpsPlayerCharacter.generated.h"

class UHealthComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FPSSHOOTER_API AFpsPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFpsPlayerCharacter();
	
	/** Сетевые функции */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	FORCEINLINE UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Контекст ввода */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* CrouchAction;
	
	/** Обработка движения */
	void Move(const FInputActionValue& Value);
	/** Обработка поворота камеры */
	void Look(const FInputActionValue& Value);
	/** Обработка начала прыжка */
	void StartJump();
	/** Обработка окончания прыжка */
	void StopJump();
	/** Обработка приседания */
	void ToggleCrouch();
	/** Настройка видимости мешей в зависимости от перспективы */
	void UpdateMeshVisibility(const bool bAlive);
	UFUNCTION(meta = (ToolTip = "Реакция на смерть"))
	void OnCharacterDied();
	
private:
	/** Персона компонента камеры */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Скелетный меш для рук (виден только себе) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Скелетный меш для тела (виден другим игрокам) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

	// Свой компонент здоровья
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathEffects();
};
