#pragma once
#include "mathstructs.h"

// offsets valid for the February 6th, 2025 game update
const uintptr_t inMatchOffset = 0x17DF8E8; // offset from client.dll; accessed in client.dll at \xC6\x46\x64\x00\x39\x5E\x28 + 0x4
const uintptr_t entityListOffset = 0x1B5C6D8;
const uintptr_t localPlayerOffset = 0x1889F30; // offset from client.dll; accessed in client.dll at \x48\x39\xBE\x80\x01\x00\x00
const uintptr_t localPlayerViewAnglesOffset = 0x547378; // offset from engine2.dll; accessed in client.dll at \xF2\x0F\x10\x00\xF2\x0F\x11\x46\x0C + 0x4

const unsigned int playerNameOffset = 0x660;
const unsigned int pawnHandleOffset = 0x80C;
const int maxPlayerCount = 64;

const unsigned int healthOffset = 0x344;
const unsigned int velocityOffset = 0x3F0;
const unsigned int headHeightOffset = 0xBA4;
const unsigned int teamOffset = 0xE68;
const unsigned int zoomOffset = 0x131C;
const unsigned int posOffset = 0x1324;
const unsigned int rotXOffset = 0x1394;
const unsigned int rotYOffset = 0x1398;

const float maxHeadHeight = 72;

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
};