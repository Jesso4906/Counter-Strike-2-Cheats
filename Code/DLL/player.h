#pragma once
#include "mathstructs.h"

const uintptr_t inMatchOffset = 0x17ABCA8; // offset from client.dll

const uintptr_t entityListOffset = 0x19A63A0; // offset from client.dll
const unsigned int playerNameOffset = 0x630;
const unsigned int pawnHandleOffset = 0x7DC;
const int maxPlayerCount = 64;

const uintptr_t localPlayerOffset = 0x1810FA8; // offset from client.dll

const unsigned int healthOffset = 0x324;
const unsigned int velocityOffset = 0x3D0;
const unsigned int headHeightOffset = 0xC58;
const unsigned int teamOffset = 0xDC8;
const unsigned int pitchOffset = 0x119C;
const unsigned int yawOffset = 0x11A0;
const unsigned int zoomOffset = 0x126C;
const unsigned int posOffset = 0x1274;
const unsigned int rotXOffset = 0x12E0;
const unsigned int rotYOffset = 0x12E4;

const float maxHeadHeight = 64;

enum Team
{
	Terrorist = 2,
	CounterTerrorist = 3
};

struct Player
{
	char pad1[healthOffset];
	int health;

	char pad2[velocityOffset - healthOffset - sizeof(health)];
	Vector3 velocity;

	char pad3[headHeightOffset - velocityOffset - sizeof(velocity)];
	float headHeight;

	char pad4[teamOffset - headHeightOffset - sizeof(headHeight)];
	int team;

	char pad5[pitchOffset - teamOffset - sizeof(team)];
	float pitch;
	float yaw;

	char pad6[zoomOffset - yawOffset - sizeof(yaw)];
	float zoom;

	char pad7[posOffset - zoomOffset - sizeof(zoom)];
	Vector3 pos;

	char pad8[rotXOffset - posOffset - sizeof(pos)];
	float rotX;
	float rotY;
};