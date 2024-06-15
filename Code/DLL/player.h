#pragma once
#include "mathstructs.h"

const uintptr_t inMatchOffset = 0x1774b58; // offset from client.dll; accessed at \xC6\x46\x64\x00\x39\x5E\x28 + 0x4
const uintptr_t entityListOffset = 0x19B49B8; // offset from client.dll; accessed at \x48\x81\xEC\x20\x01\x00\x00\x0F\x29\x70\xB8 + 0x37
const uintptr_t localPlayerOffset = 0x181A998; // offset from client.dll; accessed at \x48\x39\xBE\x48\x01\x00\x00
const uintptr_t localPlayerViewAnglesOffset = 0x5ECC48; // offset from engine2.dll; accessed at \xF2\x0F\x10\x00\xF2\x0F\x11\x46\x0C + 0x4

const unsigned int playerNameOffset = 0x630;
const unsigned int pawnHandleOffset = 0x7DC;
const int maxPlayerCount = 64;

const unsigned int healthOffset = 0x324;
const unsigned int velocityOffset = 0x3D0;
const unsigned int headHeightOffset = 0xC58;
const unsigned int teamOffset = 0xDC8;
const unsigned int zoomOffset = 0x126C;
const unsigned int posOffset = 0x1274;
const unsigned int rotXOffset = 0x12E0;
const unsigned int rotYOffset = 0x12E4;
const unsigned int shotsFiredOffset = 0x22A4;

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

	char pad5[zoomOffset - teamOffset - sizeof(team)];
	float zoom;

	char pad6[posOffset - zoomOffset - sizeof(zoom)];
	Vector3 pos;

	char pad7[rotXOffset - posOffset - sizeof(pos)];
	float rotX;
	float rotY;

	char pad8[shotsFiredOffset - rotYOffset - sizeof(rotY)];
	int shotsFired;
};