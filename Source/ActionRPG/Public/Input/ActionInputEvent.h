#pragma once
#include "GameplayTagContainer.h"

enum class EActionInputEventType : uint8
{
	Pressed = 1,
	Released = 2,
	AxisChanged = 3,
	DirectionEntered = 4
};

USTRUCT(BlueprintType)
struct ACTIONRPG_API FActionInputEvent
{
	GENERATED_BODY()
public:
	int InputSequence = 0;
	int CapturedCombatFrame = INDEX_NONE;
	
	FGameplayTag InputTag;
	EActionInputEventType EventType = EActionInputEventType::Pressed;

public:
	FGameplayTag QuantizeDirectionTag(FVector2D direction) const;
};

struct FActionDirectionState
{
	FVector2D RawAxis = FVector2D::ZeroVector;
	FGameplayTag Tag;
	int32 HeldFrames = 0;
};

// FInputState
// 1  = 本帧按下
// 2+ = 持续按住
// -1  = 本帧释放
// -2  = 持续释放