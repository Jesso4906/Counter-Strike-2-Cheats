#include <string>
#include "mathStructs.h"
#include "memoryTools.h"
#include "directx11.h"
#include "player.h"

bool IsCursorInWindow();

Player* GetPlayer(int index);

bool IsValidPlayer(Player* player);

bool CanAimbotPlayer(Player* player);

void PredictPosition(Player* targetPlayer, Vector3& out);

Vector2 GetPlayerScreenPos(Player* player);

Player* GetClosestPlayer();

void MoveViewAngles(float targetPitch, float targetYaw, float tolerance);

void Aimbot(Player* targetPlayer);

void ESP(ImDrawList* drawList);