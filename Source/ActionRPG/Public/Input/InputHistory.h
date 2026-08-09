#pragma once

#include "ActionInputTypes.h"

class ACTIONRPG_API FInputHistory
{
public:
	explicit FInputHistory(int32 InCapacityFrames = 10);
	void Reset();
	void PushFrame(FActionInputFrame&& frame);
	int32 GetNumFrames() const;
	int32 GetNextFrameId() const;

	const FActionInputFrame* FindFrameById(int32 FrameId) const;
	FActionInputFrame& GetWriteSlot();

private:
	TArray<FActionInputFrame> InputFrames;
	int32 CapacityFrames = 10;
	int32 WriteIndex = 0;
	int32 NumFrames = 0;

	int32 CurFrameId = 0;
};
