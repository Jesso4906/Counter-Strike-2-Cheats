#include <string>
#include "mathStructs.h"
#include "memoryTools.h"
#include "directx11.h"
#include "player.h"

bool IsCursorInWindow();

uintptr_t GetPlayerController(int index);

Player* GetPlayer(int index);

bool IsValidPlayer(Player* player);

bool CanAimbotPlayer(Player* player);

void PredictPosition(Player* targetPlayer, Vector3& out);

Vector2 GetPlayerScreenPos(Player* player, bool getHeadPos);

Player* GetClosestPlayer();

void MoveViewAngles(float targetPitch, float targetYaw, float speed, bool useTolerance);

void Aimbot(Player* targetPlayer);

void ESP(ImDrawList* drawList);