#pragma once

#include "VirtualMethod.h"

#include "Vector.h"
#include "UserCmd.h"

class VerifiedUserCmd
{
public:
	UserCmd cmd;
	unsigned long crc;
};

class Input {
public:
    std::byte pad[12];
    bool isTrackIRAvailable;
    bool isMouseInitialized;
    bool isMouseActive;
    std::byte pad1[158];
    bool isCameraInThirdPerson;
    std::byte pad2;
    Vector cameraOffset;
	bool CameraDistanceMove;
	int CameraOldX;
	int CameraOldY;
	int CameraX;
	int CameraY;
	bool CameraIsOrthographic;
	Vector PreviousViewAngles;
	Vector PreviousViewAnglesTilt;
	float LastForwardMove;
	int ClearInputState;
	char pad_0xE4[0x8];
	UserCmd* commands{};
	VerifiedUserCmd* verified_commands{};

	UserCmd* GetUserCmd(int sequence_number)
	{
		return VirtualMethod::call<UserCmd*, 8>(this, 0, sequence_number);
	}

	UserCmd* GetUserCmd(int nSlot, int sequence_number)
	{
		return VirtualMethod::call<UserCmd*, 8>(this, nSlot, sequence_number);
	}

	VerifiedUserCmd* GetVerifiedCmd(int sequence_number)
	{
		auto verifiedCommands = *(VerifiedUserCmd**)(reinterpret_cast<uint32_t>(this) + 0xF8);
		return &verifiedCommands[sequence_number % 150];
	}
};
