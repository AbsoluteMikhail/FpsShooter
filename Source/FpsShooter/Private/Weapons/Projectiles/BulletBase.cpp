// Copyright Notice © 2025, Mikhail Efremov. All rights reserved.


#include "Weapons/Projectiles/BulletBase.h"

#include "Components/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// сфера в качестве простого представления столкновений
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(10.0f);
	CollisionSphere->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionSphere->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.0f));
	CollisionSphere->CanCharacterStepUpOn = ECB_No;

	// компонент ProjectileMovementComponent для управления движением этого снаряда
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->ProjectileGravityScale = 0.2f;

	// По умолчанию умирает через несколько секунд
	InitialLifeSpan = DefaultLifeSpan;
}

void ABulletBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CollisionSphere->OnComponentHit.AddDynamic(this, &ABulletBase::BulletHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABulletBase::BulletOverlapBegin);
}

void ABulletBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Репликация не обязательна, но пусть будет
	DOREPLIFETIME(ABulletBase, Damage);
}

void ABulletBase::BulletOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Hello! =)");
		
}

void ABulletBase::BulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;
		
	//if (Cast<AFpsPlayerCharacter>(OtherActor)) 
	if (OtherActor->GetComponentByClass(UHealthComponent::StaticClass()))
	{
		const auto OwnerActor = GetOwner();
		AController* DamageInstigator = nullptr;
        
		if (const auto OwnerPawn = Cast<APawn>(OwnerActor))
		{
			DamageInstigator = OwnerPawn->GetController();
		}

		UGameplayStatics::ApplyPointDamage(OtherActor, Damage,
			Hit.TraceStart, Hit, DamageInstigator, OwnerActor, nullptr);

		PlayImpactEffects_Multicast(Hit);
		Destroy();
	}
	
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 50.0f, GetActorLocation());

		PlayImpactEffects_Multicast(Hit);
		Destroy();
	}

	PlayImpactEffects_Multicast(Hit);
}

void ABulletBase::PlayImpactEffects_Multicast_Implementation(const FHitResult& Hit)
{
	// Дополнительные эффекты попадания в блюпринте
	PlayImpactEffects_BP(Hit);
	
	// Здесь можно добавить воспроизведение звуков и партиклов
	// Например:
	// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, Hit.Location, Hit.Normal.Rotation());
	// UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.Location);
}
