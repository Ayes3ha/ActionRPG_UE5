#include "Input/ActionInputTypes.h"

bool FActionInputFrame::WasPressed(EActionInputButton Button)
{
	const uint64 Mask = uint64{1} << static_cast<uint8>(Button);
	return (PressedMask & Mask) != 0;
}

void FActionInputFrame::CopyFrom(FActionInputFrame& InputFrame)
{
	this->FrameId = InputFrame.FrameId;
	this->HeldMask = InputFrame.HeldMask;
	this->PressedMask = InputFrame.PressedMask;
	this->ReleasedMask = InputFrame.ReleasedMask;
	this->MoveAxis = InputFrame.MoveAxis;
	this->LastInputSequence = InputFrame.LastInputSequence;
}

void FActionInputFrame::Reset()
{
	this->FrameId = 0;
	this->HeldMask = 0;
	this->PressedMask = 0;
	this->ReleasedMask = 0;
	this->MoveAxis = FVector2D::ZeroVector;
	this->LastInputSequence = 0;
}
