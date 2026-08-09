// Copyright Epic Games, Inc. All Rights Reserved.


#include "Input/InputHistory.h"

FInputHistory::FInputHistory(int32 InCapacityFrames)
{
	CapacityFrames = InCapacityFrames;
	InputFrames.SetNum(CapacityFrames);

	Reset();
}

void FInputHistory::Reset()
{
	NumFrames = 0;
	CurFrameId = 0;
	WriteIndex = 0;

	for (int i = 0; i < CapacityFrames; ++i)
	{
		FActionInputFrame& inputFrame = InputFrames[i];
		inputFrame.Reset();
	}
}

void FInputHistory::PushFrame(FActionInputFrame&& frame)
{
	++CurFrameId;

	FActionInputFrame& InputFrame = GetWriteSlot();
	WriteIndex = (WriteIndex + 1) % InputFrames.Num();
	NumFrames = FMath::Min(NumFrames + 1, CapacityFrames);

	InputFrame.CopyFrom(frame);
}

int32 FInputHistory::GetNumFrames() const
{
	return NumFrames;
}

int32 FInputHistory::GetNextFrameId() const
{
	return CurFrameId + 1;
}

const FActionInputFrame* FInputHistory::FindFrameById(int32 FrameId) const
{
	if (FrameId < CurFrameId - NumFrames + 1 || FrameId > CurFrameId)
	{
		return nullptr;
	}
	
	const int32 Index = (FrameId + 1) % CapacityFrames;
	return &InputFrames[Index];
}

FActionInputFrame& FInputHistory::GetWriteSlot()
{
	WriteIndex = WriteIndex % CapacityFrames;
	
	return InputFrames[WriteIndex];
}
