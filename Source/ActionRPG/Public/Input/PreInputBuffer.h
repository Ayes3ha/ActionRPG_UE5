#pragma once
#include "ActionCommand.h"

class FPreInputBuffer
{
public:
	void NextStep(int64 frameId);
	ActionCommand GetCurFrameCommand();

private:
	int32 m_curFrameId = INDEX_NONE;
};
