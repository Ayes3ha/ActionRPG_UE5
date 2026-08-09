// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Input/ActionInputTypes.h"
#include "Input/PreInputBuffer.h"
#include "Input/RawInputAccumulator.h"
#include "ActionComponent.generated.h"


UCLASS(ClassGroup=(Action), meta=(BlueprintSpawnableComponent))
class ACTIONRPG_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Action|Input")
	void SubmitButtonInput(EActionInputButton button, bool bPressed);

	UFUNCTION(BlueprintCallable, Category = "Action|Input")
	void SubmitDirectionInput(FVector2D inputAxis);

private:
	FRawInputAccumulator m_rawInput;
	FInputHistory m_inputHisotry;
	FPreInputBuffer m_preInput;

	
};
