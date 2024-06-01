#include "dllmain.h"

uintptr_t clientDll = 0;

bool inMatch = false;

Player* localPlayer = nullptr;

int screenHeight = 0;
int screenWidth = 0;
bool isCursorInWindow = false;

bool hideMenu = false;

float aimbotTolerance = 0.05;
int maxTimer = 200;

bool enableAimbot = true;
bool holdToUseAimbot = false;
bool headShots = true;
bool targetClosestToCrosshair = true;
float aimbotStrength = 5;

bool esp = false;
bool showPlayerNames = true;
bool hideEspInfo = true;
bool enableCrosshair = true;

bool targetSameTeam = false;

DWORD WINAPI Thread(LPVOID param)
{
	clientDll = (uintptr_t)GetModuleHandle(L"client.dll");

	if (clientDll == 0 || !HookPresent()) // hooking directx
	{
		FreeLibraryAndExitThread((HMODULE)param, 0);
		return 0;
	}

	Player* aimbotTargetPlayer = nullptr;

	int timer = 0;
	bool aimbot = false;
	while (!GetAsyncKeyState(VK_INSERT)) // exit when ins key is pressed
	{
		inMatch = *(bool*)(clientDll + inMatchOffset);
		if (!inMatch) { continue; }
		
		timer++;

		if (GetAsyncKeyState(VK_F1) & 1)
		{
			hideMenu = !hideMenu;
		}

		if (!isCursorInWindow || !enableAimbot) { aimbot = false; continue; }

		Player** localPlayerPtr = (Player**)(clientDll + localPlayerOffset);
		if (localPlayerPtr == nullptr) { localPlayer = nullptr; continue; }
		localPlayer = *localPlayerPtr;

		if (!IsValidPlayer(localPlayer)) { continue; }

		if ((GetAsyncKeyState(VK_LSHIFT) & 1))
		{
			aimbot = !aimbot;

			if (aimbot) { aimbotTargetPlayer = GetClosestPlayer(); }
		}

		float zoomModifier = localPlayer->zoom;
		if (localPlayer->zoom < 0.2) { zoomModifier = 0.1; }

		if ((!holdToUseAimbot || (holdToUseAimbot && GetAsyncKeyState(VK_LSHIFT))) && aimbot && timer > maxTimer * zoomModifier)
		{
			timer = 0;
			
			if (!CanAimbotPlayer(aimbotTargetPlayer))
			{ 
				aimbot = false;
				continue; 
			}

			Aimbot(aimbotTargetPlayer);
		}
	}

	UnhookPresent();

	Sleep(100);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
	if (p_context) { p_context->Release(); p_context = NULL; }
	if (p_device) { p_device->Release(); p_device = NULL; }
	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

	FreeLibraryAndExitThread((HMODULE)param, 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{ 
		CreateThread(0, 0, Thread, hModule, 0, 0); 
	}
	
	return TRUE;
}

void Draw() // called in DetourPresent()
{
	ImGuiIO& io = ImGui::GetIO();
	screenHeight = io.DisplaySize.y;
	screenWidth = io.DisplaySize.x;
	isCursorInWindow = IsCursorInWindow();
	
	if (!hideMenu) 
	{
		ImU32 primaryTeamColor;
		ImU32 secondaryTeamColor;

		int team = inMatch && IsValidPlayer(localPlayer) ? localPlayer->team : -1;
		switch (team)
		{
		case Terrorist:
			primaryTeamColor = IM_COL32(230, 180, 90, 255);
			secondaryTeamColor = IM_COL32(130, 100, 50, 255);
			break;
		case CounterTerrorist:
			primaryTeamColor = IM_COL32(90, 150, 250, 255);
			secondaryTeamColor = IM_COL32(50, 100, 150, 255);
			break;
		default:
			primaryTeamColor = IM_COL32(150, 150, 150, 255);
			secondaryTeamColor = IM_COL32(80, 80, 80, 255);
			break;
		}

		ImGui::PushStyleColor(ImGuiCol_CheckMark, primaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, primaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, primaryTeamColor);

		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, secondaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBg, secondaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, secondaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, secondaryTeamColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, secondaryTeamColor);

		ImGui::Begin("CS2 Jesso Cheats", nullptr, ImGuiWindowFlags_NoMove);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(400, 315), ImGuiCond_Always);

		ImGui::Text("Ins - uninject");
		ImGui::Text("F1 - hide this menu");
		ImGui::Text("Left Shift - use aimbot");

		ImGui::Checkbox("Enable aimbot", &enableAimbot);
		ImGui::Checkbox("Hold to use aimbot", &holdToUseAimbot);
		ImGui::Checkbox("Aim for heads", &headShots);
		ImGui::Checkbox("Target player closest to crosshair", &targetClosestToCrosshair);
		ImGui::SliderFloat("Aimbot strength", &aimbotStrength, 0.1, 5);

		ImGui::Checkbox("ESP", &esp);
		ImGui::Checkbox("Show player names", &showPlayerNames);
		ImGui::Checkbox("Hide ESP info", &hideEspInfo);
		ImGui::Checkbox("Enable centered crosshair", &enableCrosshair);

		ImGui::Checkbox("ESP/Aimbot players on same team", &targetSameTeam);

		ImGui::End();

		ImGui::PopStyleColor(8);
	}

	if(esp)
	{
		if (!inMatch || !IsValidPlayer(localPlayer)) { return; }
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("invis window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(screenWidth, screenHeight), ImGuiCond_Always);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* drawList = window->DrawList;

		if (enableCrosshair)
		{
			drawList->AddCircle(ImVec2(screenWidth / 2, screenHeight / 2), 3, IM_COL32(255, 0, 0, 255), 0, 2);
		}

		ESP(drawList);

		window->DrawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
}

bool IsCursorInWindow()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (viewport == nullptr) { return false; }

	ImVec2 windowPos = viewport->Pos;
	ImVec2 windowSize = viewport->Size;
	ImVec2 cursorPos = ImGui::GetIO().MousePos;

	if (cursorPos.x < windowPos.x) { return false; }
	if (cursorPos.x > windowPos.x + windowSize.x) { return false; }

	if (cursorPos.y < windowPos.y) { return false; }
	if (cursorPos.y > windowPos.y + windowSize.y) { return false; }

	return true;
}

uintptr_t GetPlayerController(int index) 
{
	if (index < 0 || index >= maxPlayerCount) { return 0; }

	uintptr_t* entitySystemPtr = (uintptr_t*)(clientDll + entityListOffset);
	if (entitySystemPtr == nullptr || (*entitySystemPtr) == 0) { return 0; }
	uintptr_t entitySystem = *entitySystemPtr;

	uintptr_t* entityListPtr = (uintptr_t*)(entitySystem + 0x10);
	if (entityListPtr == nullptr || (*entityListPtr) == 0) { return 0; }
	uintptr_t entityList = *entityListPtr;

	uintptr_t* playerControllerPtr = (uintptr_t*)(entityList + ((0x78 * index) + 0x78));
	if (playerControllerPtr == nullptr) { return 0; }
	uintptr_t playerController = *playerControllerPtr;

	return playerController;
}

Player* GetPlayer(int index)
{
	if (index < 0 || index >= maxPlayerCount) { return nullptr; }

	uintptr_t playerController = GetPlayerController(index);
	if (playerController == 0) { return nullptr; }

	int pawnHandle = *(int*)(playerController + pawnHandleOffset);
	if (pawnHandle == 0) { return nullptr; }

	uintptr_t* entitySystemPtr = (uintptr_t*)(clientDll + entityListOffset);
	if (entitySystemPtr == nullptr || (*entitySystemPtr) == 0) { return 0; }
	uintptr_t entitySystem = *entitySystemPtr;
	
	uintptr_t* listEntityPtr = (uintptr_t*)(entitySystem + (0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10));
	if (listEntityPtr == nullptr || (*listEntityPtr) == 0) { return nullptr; }
	uintptr_t listEntity = *listEntityPtr;

	Player** playerPtr = (Player**)(listEntity + (0x78 * (pawnHandle & 0x1FF)));
	if (playerPtr == nullptr || (*playerPtr) == 0) { return nullptr; }
	Player* player = *playerPtr;

	return player;
}

bool IsValidPlayer(Player* player)
{
	if ((uintptr_t)player < 0x10000) { return false; }
	if (player->health < 1 || player->health > 100) { return false; }
	if (player->headHeight < 1 || player->headHeight > 100) { return false; }
	if (player->team != Terrorist && player->team != CounterTerrorist) { return false; }
	if (player->zoom == 0) { return false; }
	if (player->pos.x == 0 || player->pos.y == 0 || player->pos.z == 0) { return false; }

	return true;
}

bool CanAimbotPlayer(Player* player)
{
	if (!IsValidPlayer(player) || player == localPlayer) { return false; }
	if (!targetSameTeam && player->team == localPlayer->team) { return false; }

	return true;
}

void PredictPosition(Player* targetPlayer, Vector3& out)
{
	if (!IsValidPlayer(localPlayer) || !IsValidPlayer(targetPlayer)) { return; }

	Vector3 velocity = targetPlayer->velocity - localPlayer->velocity;

	float devisor = 35 * localPlayer->zoom;
	if (devisor == 0) { return; }
	if (devisor < 15) { devisor = 15; }

	out.x += velocity.x / devisor;
	out.y += velocity.y / devisor;
	out.z += velocity.z / devisor;
}

Vector2 GetPlayerScreenPos(Player* player)
{
	Vector2 result = {};
	
	Vector3 localPlayerPos = localPlayer->pos;
	localPlayerPos.z += localPlayer->headHeight - maxHeadHeight;

	Vector3 targetPlayerPos = player->pos;
	targetPlayerPos.z += player->headHeight - maxHeadHeight + 8;

	PredictPosition(player, targetPlayerPos);

	Vector3 diff = localPlayerPos - targetPlayerPos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return result; }

	float pitchToPlayer = -(asin((targetPlayerPos.z - localPlayerPos.z) / distance) * rToD);
	float yawToPlayer = (atan2(targetPlayerPos.y - localPlayerPos.y, targetPlayerPos.x - localPlayerPos.x) * rToD);

	float relativePitch = pitchToPlayer - localPlayer->pitch;
	float relativeYaw = localPlayer->yaw - yawToPlayer;

	if (relativeYaw > 180) { relativeYaw = -(360 - relativeYaw); }
	if (relativeYaw < -180) { relativeYaw = (360 + relativeYaw); }

	float xFov = (-0.015 * relativeYaw * relativeYaw) + (0.01 * abs(relativeYaw)) + 150; // https://www.desmos.com/calculator/tc3c5mtds2
	float yFov = (0.005 * relativePitch * relativePitch) + (-abs(relativePitch)) + 100; // https://www.desmos.com/calculator/amal9mekga

	if (xFov == 0 || yFov == 0) { return result; }

	float screenY = relativePitch / (yFov * 0.5 * localPlayer->zoom);
	screenY = (screenY + 1) / 2;
	screenY *= screenHeight;

	float screenX = relativeYaw / (xFov * 0.5 * localPlayer->zoom);

	screenX = (screenX + 1) / 2;
	screenX *= screenWidth;

	result.x = screenX;
	result.y = screenY;

	return result;
}

Player* GetClosestPlayer()
{
	if (!IsValidPlayer(localPlayer)) { return nullptr; }

	float minDistance = 999999999999.0f;

	Player* targetPlayer = 0;

	for (int i = 0; i < maxPlayerCount; i++)
	{
		Player* player = GetPlayer(i);
		if (!CanAimbotPlayer(player)) { continue; }

		float distance;
		if (targetClosestToCrosshair)
		{
			Vector2 crosshair = { screenWidth / 2, screenHeight / 2 };
			Vector2 diff = crosshair - GetPlayerScreenPos(player);
			distance = sqrt((diff.x * diff.x) + (diff.y * diff.y));
		}
		else 
		{
			Vector3 diff = localPlayer->pos - player->pos;
			distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
		}

		if (distance < minDistance) 
		{
			minDistance = distance;
			targetPlayer = player;
		}
	}

	return targetPlayer;
}

void MoveViewAngles(float targetPitch, float targetYaw)
{
	INPUT input;
	input.type = INPUT_MOUSE;
	MOUSEINPUT mouseInput;
	mouseInput.dwFlags = MOUSEEVENTF_MOVE;

	float deltaPitch = targetPitch - localPlayer->pitch;
	float deltaYaw = localPlayer->yaw - targetYaw;

	if (deltaYaw > 180) { deltaYaw = -(360 - deltaYaw); }
	if (deltaYaw < -180) { deltaYaw = (360 + deltaYaw); }

	float deltaY = deltaPitch * aimbotStrength;
	float deltaX = deltaYaw * aimbotStrength;

	if (deltaY > 0 && deltaY < 1) { deltaY = 1; }
	if (deltaY < 0 && deltaY > -1) { deltaY = -1; }
	if (deltaX > 0 && deltaX < 1) { deltaX = 1; }
	if (deltaX < 0 && deltaX > -1) { deltaX = -1; }

	if (deltaPitch > -aimbotTolerance && deltaPitch < aimbotTolerance) { deltaY = 0; }
	if (deltaYaw > -aimbotTolerance && deltaYaw < aimbotTolerance) { deltaX = 0; }

	mouseInput.dy = deltaY;
	mouseInput.dx = deltaX;

	input.mi = mouseInput;
	SendInput(1, &input, sizeof(INPUT));
}

void Aimbot(Player* targetPlayer)
{
	Vector3 localPlayerPos = localPlayer->pos;
	localPlayerPos.z += localPlayer->headHeight - maxHeadHeight;

	Vector3 targetPlayerPos = targetPlayer->pos;
	targetPlayerPos.z += targetPlayer->headHeight - (headShots ? maxHeadHeight : 90);
	if (headShots) 
	{
		targetPlayerPos.x += targetPlayer->rotX * 4;
		targetPlayerPos.y += targetPlayer->rotY * 4;
	}

	PredictPosition(targetPlayer, targetPlayerPos);

	Vector3 diff = localPlayerPos - targetPlayerPos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return; }

	float pitch = -(asin((targetPlayerPos.z - localPlayerPos.z) / distance) * rToD);
	float yaw = (atan2(targetPlayerPos.y - localPlayerPos.y, targetPlayerPos.x - localPlayerPos.x) * rToD);

	MoveViewAngles(pitch, yaw);
}

void ESP(ImDrawList* drawList)
{
	if (drawList == nullptr) { return; }

	for (int i = 0; i < maxPlayerCount; i++)
	{
		Player* player = GetPlayer(i);

		if (!IsValidPlayer(player) || player == localPlayer || (!targetSameTeam && player->team == localPlayer->team)) { continue; }
		
		Vector2 screenPos = GetPlayerScreenPos(player);

		Vector3 diff = localPlayer->pos - player->pos;
		float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
		if (distance == 0) { continue; }

		float depth = (distance * 0.01 * localPlayer->zoom);
		float sizeX = (screenWidth * 0.05) / depth;
		float sizeY = ((screenHeight * 0.4) + player->headHeight) / depth;

		if ((screenPos.y < -sizeY || screenPos.y > screenHeight) || (screenPos.x < -sizeX || screenPos.x > screenWidth)) { continue; }

		ImU32 color;
		if (player->health > 80) { color = IM_COL32(0, 255, 0, 255); }
		else if (player->health > 30) { color = IM_COL32(255, 255, 0, 255); }
		else { color = IM_COL32(255, 0, 0, 255); }

		if (!hideEspInfo)
		{
			std::string healthStr = "Health: " + std::to_string(player->health);
			drawList->AddText(ImVec2(screenPos.x - sizeX, screenPos.y - 35), color, healthStr.c_str());

			std::string distStr = "Distance: " + std::to_string((int)distance);
			drawList->AddText(ImVec2(screenPos.x - sizeX, screenPos.y - 25), color, distStr.c_str());
		}

		if (showPlayerNames) 
		{
			const char* playerName = (const char*)(GetPlayerController(i) + playerNameOffset);
			drawList->AddText(ImVec2(screenPos.x - sizeX, screenPos.y - 15), color, playerName);
		}

		drawList->AddRect(ImVec2(screenPos.x - sizeX, screenPos.y), ImVec2(screenPos.x + sizeX, screenPos.y + sizeY), color);
	}
}