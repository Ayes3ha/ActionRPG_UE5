#pragma once

#include "InputHistory.h"

class FRawInputAccumulator
{
public:
	void SetButtonState(EActionInputButton key, bool bPressed);
	void SetInputAxis(const FVector2D& axis);
	FActionInputFrame ConsumeFrame(int32 frameId);

private:
	uint64 HeldMask = 0;
	uint64 LatchedPressedMask = 0;
	uint64 LatchedReleasedMask = 0;

	FVector2D LatestMoveAxis = FVector2D::ZeroVector;
	uint64 LastInputSequence = 0;
};
