// Copyright Epic Games, Inc. All Rights Reserved.


#include "Action/ActionComponent.h"


// Sets default values for this component's properties
UActionComponent::UActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UActionComponent::SubmitButtonInput(EActionInputButton button, bool bPressed)
{
	m_rawInput.SetButtonState(button, bPressed);
}

void UActionComponent::SubmitDirectionInput(FVector2D inputAxis)
{
	m_rawInput.SetInputAxis(inputAxis);
}

