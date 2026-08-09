// Copyright Epic Games, Inc. All Rights Reserved.


#include "Action/ActionCharacterBase.h"


// Sets default values
AActionCharacterBase::AActionCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AActionCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActionCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AActionCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

