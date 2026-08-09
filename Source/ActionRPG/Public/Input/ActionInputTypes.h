#pragma once

UENUM(BlueprintType)
enum class EActionInputButton : uint8
{
	Attack = 0,			// 普攻
	SpecialAttack = 1,	// 特
	Dodge = 2,			// 闪避
	SpecialAbility = 3, // 特殊能力，比如肉鸽里的邦布技能
	LB = 4,				// 切人-左
	RB = 5,				// 切人-右
	LT = 6,				// 
	RT = 7,				// 终结技
	LX = 8,				// 待定
	RX = 9,				// 手动锁定

	Count UMETA(Hidden)
};

struct FActionInputFrame
{
	int32 FrameId = 0;

	uint64 HeldMask = 0;
	uint64 PressedMask = 0;
	uint64 ReleasedMask = 0;

	FVector2D MoveAxis = FVector2D::ZeroVector;
	uint64 LastInputSequence = 0;

public:
	bool WasPressed(EActionInputButton button);
};
