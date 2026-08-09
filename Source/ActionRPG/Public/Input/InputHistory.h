#pragma once

#include "ActionInputTypes.h"

class ACTIONRPG_API FInputHistory
{
public:
	explicit FInputHistory(int32 InCapacityFrames = 10);
	void Reset();
	void PushFrame(FActionInputFrame&& frame);

	const FActionInputFrame* GetFrame(int32 ActionFrame) const;
private:
	TArray<FActionInputFrame> m_inputFrames;
	int32 m_capacityFrames = 10;
	int32 m_writeIndex = 0;
	int32 m_numFrames = 0;
};
