// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.


#include "Framework/FpsPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


AFpsPlayerCharacter::AFpsPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Создаем компонент камеры
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetRootComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Примерная высота глаз
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Создаем меш для первого лица (руки)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetOnlyOwnerSee(true); // Виден только владельцу
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->bCastDynamicShadow = false;
	
	// Для удобства сохраняем ссылку на ThirdPersonMesh
	ThirdPersonMesh = GetMesh();

	// Настройка меша для третьего лица (тело)
	ThirdPersonMesh->SetOwnerNoSee(true); // Владелец не видит этот меш
	ThirdPersonMesh->SetupAttachment(GetRootComponent());
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AFpsPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AFpsPlayerCharacter, bIsCrouch);
}

void AFpsPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HealthComponent)
	{
		OnTakeAnyDamage.AddDynamic(HealthComponent, &UHealthComponent::HandleAnyDamage);
		OnTakePointDamage.AddDynamic(HealthComponent, &UHealthComponent::HandlePointDamage);
		OnTakeRadialDamage.AddDynamic(HealthComponent, &UHealthComponent::HandleRadialDamage);
		
		HealthComponent->OnDeath.BindUObject(this, &AFpsPlayerCharacter::OnCharacterDied);
	}
}

void AFpsPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Настраиваем видимость мешей
	UpdateMeshVisibility(true);
}

void AFpsPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AFpsPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFpsPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Добавляем контекст ввода для локального игрока
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AFpsPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Настраиваем Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFpsPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFpsPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFpsPlayerCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFpsPlayerCharacter::StopJump);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AFpsPlayerCharacter::ToggleCrouch);
	}
	else
	{
		// Почти PrintString
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Hello! =)");
		
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! "
							  "This template is built to use the Enhanced Input system. "
							  "If you intend to use the legacy system, then you will need to update this C++ file."),
							  *GetNameSafe(this));
	}
}

void AFpsPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent)
	{
		HealthComponent->OnDeath.Unbind();
	}

	Super::EndPlay(EndPlayReason);
}

void AFpsPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFpsPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFpsPlayerCharacter::StartJump()
{
	Jump();
}

void AFpsPlayerCharacter::StopJump()
{
	StopJumping();
}

void AFpsPlayerCharacter::ToggleCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AFpsPlayerCharacter::UpdateMeshVisibility(const bool bAlive)
{
	if (IsLocallyControlled())
	{
		FirstPersonMesh->SetOnlyOwnerSee(bAlive);
		FirstPersonMesh->SetVisibility(bAlive);
	
		ThirdPersonMesh->SetOwnerNoSee(bAlive);
		ThirdPersonMesh->SetVisibility(!bAlive);
	}
	else
	{
		FirstPersonMesh->SetVisibility(false);
		ThirdPersonMesh->SetVisibility(true);
	}
}

void AFpsPlayerCharacter::OnCharacterDied()
{
	// Останавливаем движение
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Multicast Может вызывать только сервер
	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_PlayDeathEffects();
	}
	
	// Выключаем контроллер
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetCinematicMode(true, false, true, true, true);
	}
}

void AFpsPlayerCharacter::Multicast_PlayDeathEffects_Implementation()
{
	UpdateMeshVisibility(false);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

	// Можно проиграть анимацию смерти, звук, камеру и т.д.
	//UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
}
