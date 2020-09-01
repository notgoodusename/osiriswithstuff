#include <mutex>
#include <numeric>
#include <sstream>
#include <stdlib.h>
#include <time.h>

#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../Netvars.h"
#include "../Hooks.h"
#include "Misc.h"
#include "../SDK/ConVar.h"
#include "../SDK/Surface.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/WeaponData.h"
#include "EnginePrediction.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/GameEvent.h"
#include "../SDK/FrameStage.h"
#include "../SDK/Client.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/WeaponSystem.h"
#include "../SDK/WeaponData.h"
#include "../GUI.h"

static constexpr bool worldToScreen(const Vector& in, Vector& out) noexcept
{
    const auto& matrix = interfaces->engine->worldToScreenMatrix();
    float w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;

    if (w > 0.001f) {
        const auto [width, height] = interfaces->surface->getScreenSize();
        out.x = width / 2 * (1 + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w);
        out.y = height / 2 * (1 - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w);
        out.z = 0.0f;
        return true;
    }
    return false;
}

void Misc::removeClientSideChokeLimit() noexcept //may cause vaccccc
{
    static bool once = false;
    if (!once)
    {
        auto clMoveChokeClamp = memory->chokeLimit;

        unsigned long protect = 0;

        VirtualProtect((void*)clMoveChokeClamp, 4, PAGE_EXECUTE_READWRITE, &protect);
        *(std::uint32_t*)clMoveChokeClamp = 62;
        VirtualProtect((void*)clMoveChokeClamp, 4, protect, &protect);
        once = true;
    }
}

void Misc::edgejump(UserCmd* cmd) noexcept
{
    if (!config->misc.edgejump || !GetAsyncKeyState(config->misc.edgejumpkey))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    if ((EnginePrediction::getFlags() & 1) && !(localPlayer->flags() & 1))
        cmd->buttons |= UserCmd::IN_JUMP;
}

void Misc::slowwalk(UserCmd* cmd) noexcept
{
    if (!config->misc.slowwalk || !GetAsyncKeyState(config->misc.slowwalkKey))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    const float maxSpeed = (localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3;

    if (cmd->forwardmove && cmd->sidemove) {
        const float maxSpeedRoot = maxSpeed * static_cast<float>(M_SQRT1_2);
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
    } else if (cmd->forwardmove) {
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeed : maxSpeed;
    } else if (cmd->sidemove) {
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeed : maxSpeed;
    }
}

void Misc::fastStop(UserCmd* cmd) noexcept
{
    if (!config->misc.fastStop)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER || !(localPlayer->flags() & 1) || cmd->buttons & UserCmd::IN_JUMP)
        return;

    if (cmd->buttons & (UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT | UserCmd::IN_FORWARD | UserCmd::IN_BACK))
        return;

    auto VectorAngles = [](const Vector& forward, Vector& angles)
    {
        if (forward.y == 0.0f && forward.x == 0.0f)
        {
            angles.x = (forward.z > 0.0f) ? 270.0f : 90.0f;
            angles.y = 0.0f;
        }
        else
        {
            angles.x = atan2(-forward.z, forward.length2D()) * -180.f / M_PI;
            angles.y = atan2(forward.y, forward.x) * 180.f / M_PI;

            if (angles.y > 90)
                angles.y -= 180;
            else if (angles.y < 90)
                angles.y += 180;
            else if (angles.y == 90)
                angles.y = 0;
        }

        angles.z = 0.0f;
    };
    auto AngleVectors = [](const Vector& angles, Vector* forward)
    {
        float	sp, sy, cp, cy;

        sy = sin(degreesToRadians(angles.y));
        cy = cos(degreesToRadians(angles.y));

        sp = sin(degreesToRadians(angles.x));
        cp = cos(degreesToRadians(angles.x));

        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    };

    Vector velocity = localPlayer->velocity();
    Vector direction;
    VectorAngles(velocity, direction);
    float speed = velocity.length2D();
    if (speed < 15.f)
        return;

    direction.y = cmd->viewangles.y - direction.y;

    Vector forward;
    AngleVectors(direction, &forward);

    Vector negated_direction = forward * speed;

    cmd->forwardmove = negated_direction.x;
    cmd->sidemove = negated_direction.y;
}

void Misc::inverseRagdollGravity() noexcept
{
    static auto ragdollGravity = interfaces->cvar->findVar("cl_ragdoll_gravity");
    ragdollGravity->setValue(config->visuals.inverseRagdollGravity ? -600 : 600);
}

void Misc::clanTag() noexcept
{
    static std::string clanTag;
    static std::string oldClanTag;
    
    if (config->misc.clanTag) {
        switch (int(memory->globalVars->currenttime * 2.4) % 31)
        {
            //put your p2c clantag here
        }
    }
    
    if (!config->misc.clanTag)
        clanTag = "";
    
    if (oldClanTag != clanTag) {
        memory->setClanTag(clanTag.c_str(), clanTag.c_str());
        oldClanTag = clanTag;
    }
}

void Misc::spectatorList() noexcept
{
    if (!config->misc.spectatorList)
        return;

    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::SetNextWindowSize({ 250.f, 0.f }, ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints({ 250.f, 0.f }, { 250.f, 1000.f });
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (!gui->open)
        windowFlags |= ImGuiWindowFlags_NoInputs;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
    ImGui::Begin("Spectator list", nullptr, windowFlags);
    ImGui::PopStyleVar();

    if (interfaces->engine->isInGame() && localPlayer)
    {
        if (localPlayer->isAlive())
        {
            for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
                const auto entity = interfaces->entityList->getEntity(i);
                if (!entity || entity->isDormant() || entity->isAlive() || entity->getObserverTarget() != localPlayer.get())
                    continue;

                PlayerInfo playerInfo;

                if (!interfaces->engine->getPlayerInfo(i, playerInfo))
                    continue;

                auto obsMode{ "" };

                switch (entity->getObserverMode()) {
                case ObsMode::InEye:
                    obsMode = "1st";
                    break;
                case ObsMode::Chase:
                    obsMode = "3rd";
                    break;
                case ObsMode::Roaming:
                    obsMode = "Freecam";
                    break;
                default:
                    continue;
                }

                ImGui::TextWrapped("%s - %s", playerInfo.name, obsMode);
            }
        }
        else if (auto observer = localPlayer->getObserverTarget(); !localPlayer->isAlive() && observer && observer->isAlive())
        {
            for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
                const auto entity = interfaces->entityList->getEntity(i);
                if (!entity || entity->isDormant() || entity->isAlive() || entity == localPlayer.get() || entity->getObserverTarget() != observer)
                    continue;

                PlayerInfo playerInfo;

                if (!interfaces->engine->getPlayerInfo(i, playerInfo))
                    continue;

                auto obsMode{ "" };

                switch (entity->getObserverMode()) {
                case ObsMode::InEye:
                    obsMode = "1st";
                    break;
                case ObsMode::Chase:
                    obsMode = "3rd";
                    break;
                case ObsMode::Roaming:
                    obsMode = "Freecam";
                    break;
                default:
                    continue;
                }

                ImGui::TextWrapped("%s - %s", playerInfo.name, obsMode);
            }
        }
    }
    ImGui::End();
}



void Misc::debugwindow() noexcept
{
    if (!config->misc.debugwindow)
        return;

}



void Misc::sniperCrosshair() noexcept
{
    static auto showSpread = interfaces->cvar->findVar("weapon_debug_spread_show");
    showSpread->setValue(config->misc.sniperCrosshair && localPlayer && !localPlayer->isScoped() ? 3 : 0);
}

void Misc::recoilCrosshair() noexcept
{
    static auto recoilCrosshair = interfaces->cvar->findVar("cl_crosshair_recoil");
    recoilCrosshair->setValue(config->misc.recoilCrosshair ? 1 : 0);
}

void Misc::watermark() noexcept
{
    if (!config->misc.watermark)
        return;
    
    ImGui::SetNextWindowBgAlpha(1.f);
    ImGui::SetNextWindowSizeConstraints({ 0.f, 0.f }, { 1000.f, 1000.f });
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
    ImGui::Begin("Watermark", nullptr, windowFlags);
    ImGui::PopStyleVar();
    
    const auto [screenWidth, screenHeight] = interfaces->surface->getScreenSize();

    static auto frameRate = 1.0f;
    frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;

    float latency = 0.0f;
    if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f) {
        latency = networkChannel->getLatency(0);
    }

    std::string fps{  std::to_string(static_cast<int>(1 / frameRate)) + " fps" };
    std::string ping{ interfaces->engine->isConnected() ? std::to_string(static_cast<int>(latency * 1000)) + " ms" : "Not connected" };

    ImGui::Text("Osiris | %s | %s", fps.c_str(), ping.c_str());
    ImGui::SetWindowPos({ screenWidth - ImGui::GetWindowSize().x - 15, 15 });
    
    ImGui::End();
}

void Misc::prepareRevolver(UserCmd* cmd) noexcept
{
    constexpr auto timeToTicks = [](float time) {  return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick); };
    constexpr float revolverPrepareTime{ 0.234375f };

    static float readyTime;
    if (config->misc.prepareRevolver && localPlayer && (!config->misc.prepareRevolverKey || GetAsyncKeyState(config->misc.prepareRevolverKey))) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver) {
            if (!readyTime) readyTime = memory->globalVars->serverTime() + revolverPrepareTime;
            auto ticksToReady = timeToTicks(readyTime - memory->globalVars->serverTime() - interfaces->engine->getNetworkChannel()->getLatency(0));
            if (ticksToReady > 0 && ticksToReady <= timeToTicks(revolverPrepareTime))
                cmd->buttons |= UserCmd::IN_ATTACK;
            else
                readyTime = 0.0f;
        }
    }
}

void Misc::fastPlant(UserCmd* cmd) noexcept
{
    if (config->misc.fastPlant) {
        static auto plantAnywhere = interfaces->cvar->findVar("mp_plant_c4_anywhere");

        if (plantAnywhere->getInt())
            return;

        if (!localPlayer || !localPlayer->isAlive() || localPlayer->inBombZone())
            return;

        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (!activeWeapon || activeWeapon->getClientClass()->classId != ClassId::C4)
            return;

        cmd->buttons &= ~UserCmd::IN_ATTACK;

        constexpr float doorRange{ 200.0f };
        Vector viewAngles{ cos(degreesToRadians(cmd->viewangles.x)) * cos(degreesToRadians(cmd->viewangles.y)) * doorRange,
                           cos(degreesToRadians(cmd->viewangles.x)) * sin(degreesToRadians(cmd->viewangles.y)) * doorRange,
                          -sin(degreesToRadians(cmd->viewangles.x)) * doorRange };
        Trace trace;
        interfaces->engineTrace->traceRay({ localPlayer->getEyePosition(), localPlayer->getEyePosition() + viewAngles }, 0x46004009, localPlayer.get(), trace);

        if (!trace.entity || trace.entity->getClientClass()->classId != ClassId::PropDoorRotating)
            cmd->buttons &= ~UserCmd::IN_USE;
    }
}

void Misc::drawBombTimer() noexcept
{
    if (config->misc.bombTimer.enabled) {
        for (int i = interfaces->engine->getMaxClients(); i <= interfaces->entityList->getHighestEntityIndex(); i++) {
            Entity* entity = interfaces->entityList->getEntity(i);
            if (!entity || entity->isDormant() || entity->getClientClass()->classId != ClassId::PlantedC4 || !entity->c4Ticking())
                continue;

            interfaces->surface->setTextFont(hooks->verdanaExtraBoldAA);

            auto drawPositionY{ interfaces->surface->getScreenSize().second / 7 };
            auto bombText{ (std::wstringstream{ } << L"Bomb " << (!entity->c4BombSite() ? 'A' : 'B')).str() };
            auto bombText2{ (std::wstringstream{ } << std::fixed << std::showpoint << std::setprecision(1) << (std::max)(entity->c4BlowTime() - memory->globalVars->currenttime, 0.0f) << L" s").str() };
            const auto bombTextX{ interfaces->surface->getScreenSize().first / 2 - static_cast<int>((interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, bombText.c_str())).first / 2) };
            const auto bombText2X{ interfaces->surface->getScreenSize().first / 2 - static_cast<int>((interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, bombText2.c_str())).first / 2) };

            interfaces->surface->setTextPosition(bombTextX, drawPositionY);
            drawPositionY += interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, bombText.c_str()).second;
            interfaces->surface->setTextColor(242, 242, 199);
            interfaces->surface->printText(bombText.c_str());

            interfaces->surface->setTextPosition(bombText2X, drawPositionY);
            drawPositionY += interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, bombText2.c_str()).second;
            interfaces->surface->setTextColor(255, 255, 255);
            interfaces->surface->printText(bombText2.c_str());

            if (entity->c4Defuser() != -1) {
                if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(interfaces->entityList->getEntityFromHandle(entity->c4Defuser())->index(), playerInfo)) {
                    if (wchar_t name[128];  MultiByteToWideChar(CP_UTF8, 0, playerInfo.name, -1, name, 128)) {

                        drawPositionY += (interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, L" ").second)*2;

                        const auto defusingText{ (std::wstringstream{ } << L"Defusing").str() };
                        const auto defusingText2{ (std::wstringstream{ } << std::fixed << std::showpoint << std::setprecision(1) << (std::max)(entity->c4DefuseCountDown() - memory->globalVars->currenttime, 0.0f) << L" s").str() };
                        
                        interfaces->surface->setTextPosition((interfaces->surface->getScreenSize().first - interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, defusingText.c_str()).first) / 2, drawPositionY);
                        drawPositionY += interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, L" ").second;
                        interfaces->surface->setTextColor(242, 141, 141);
                        interfaces->surface->printText(defusingText.c_str());

                        interfaces->surface->setTextPosition((interfaces->surface->getScreenSize().first - interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, defusingText2.c_str()).first) / 2, drawPositionY);
                        drawPositionY += interfaces->surface->getTextSize(hooks->verdanaExtraBoldAA, L" ").second;
                        interfaces->surface->setTextColor(242, 141, 141);
                        interfaces->surface->printText(defusingText2.c_str());
                    }
                }
            }
            break;
        }
    }
}

void Misc::stealNames() noexcept
{
    if (!config->misc.nameStealer)
        return;

    if (!localPlayer)
        return;

    static std::vector<int> stolenIds;

    for (int i = 1; i <= memory->globalVars->maxClients; ++i) {
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get())
            continue;

        PlayerInfo playerInfo;
        if (!interfaces->engine->getPlayerInfo(entity->index(), playerInfo))
            continue;

        if (playerInfo.fakeplayer || std::find(stolenIds.cbegin(), stolenIds.cend(), playerInfo.userId) != stolenIds.cend())
            continue;

        if (changeName(false, (std::string{ playerInfo.name } +'\x1').c_str(), 1.0f))
            stolenIds.push_back(playerInfo.userId);

        return;
    }
    stolenIds.clear();
}

void Misc::disablePanoramablur() noexcept
{
    static auto blur = interfaces->cvar->findVar("@panorama_disable_blur");
    blur->setValue(config->misc.disablePanoramablur);
}

bool Misc::changeName(bool reconnect, const char* newName, float delay) noexcept
{
    static auto exploitInitialized{ false };

    static auto name{ interfaces->cvar->findVar("name") };

    if (reconnect) {
        exploitInitialized = false;
        return false;
    }

    if (!exploitInitialized && interfaces->engine->isInGame()) {
        if (PlayerInfo playerInfo; localPlayer && interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo) && (!strcmp(playerInfo.name, "?empty") || !strcmp(playerInfo.name, "\n\xAD\xAD\xAD"))) {
            exploitInitialized = true;
        } else {
            name->onChangeCallbacks.size = 0;
            name->setValue("\n\xAD\xAD\xAD");
            return false;
        }
    }

    static auto nextChangeTime{ 0.0f };
    if (nextChangeTime <= memory->globalVars->realtime) {
        name->setValue(newName);
        nextChangeTime = memory->globalVars->realtime + delay;
        return true;
    }
    return false;
}

void Misc::bunnyHop(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    static auto wasLastTimeOnGround{ localPlayer->flags() & 1 };

    if (config->misc.bunnyHop && !(localPlayer->flags() & 1) && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround)
        cmd->buttons &= ~UserCmd::IN_JUMP;

    wasLastTimeOnGround = localPlayer->flags() & 1;
}

void Misc::fakeBan(bool set) noexcept
{
    static bool shouldSet = false;

    if (set)
        shouldSet = set;

    if (shouldSet && interfaces->engine->isInGame() && changeName(false, std::string{ "\x1\xB" }.append(std::string{ static_cast<char>(config->misc.banColor + 1) }).append(config->misc.banText).append("\x1").c_str(), 5.0f))
        shouldSet = false;
}

void Misc::nadeTrajectory() noexcept
{
    static auto trajectoryVar{ interfaces->cvar->findVar("sv_grenade_trajectory") };
    static auto trajectoryTimeVar{ interfaces->cvar->findVar("sv_grenade_trajectory_time") };

    static auto timeBackup = trajectoryTimeVar->getFloat();

    trajectoryVar->onChangeCallbacks.size = 0;
    trajectoryVar->setValue(config->misc.nadeTrajectory);
    trajectoryTimeVar->onChangeCallbacks.size = 0;
    trajectoryTimeVar->setValue(config->misc.nadeTrajectory ? 4 : timeBackup);
}

void Misc::showImpacts() noexcept
{
    static auto impactsVar{ interfaces->cvar->findVar("sv_showimpacts") };

    impactsVar->onChangeCallbacks.size = 0;
    impactsVar->setValue(config->misc.showImpacts);
}

void Misc::fakePrime() noexcept
{
    static bool lastState = false;

    if (config->misc.fakePrime != lastState) {
        lastState = config->misc.fakePrime;

        if (DWORD oldProtect; VirtualProtect(memory->fakePrime, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            constexpr uint8_t patch[]{ 0x74, 0xEB };
            *memory->fakePrime = patch[config->misc.fakePrime];
            VirtualProtect(memory->fakePrime, 1, oldProtect, nullptr);
        }
    }
}

void Misc::killSay(GameEvent& event) noexcept
{
    if (!config->misc.killSay)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    std::string cheatNames[] = {
        "osiris",
        "otc",
        "skeet by aze",
        "texashook",
        "texascheats",
        "texaslan",
        "x22",
        "aimjunkies",
        "ayyware",
        "ezfrags",
        "indigo",
        "hentaiware",
        "zapped",
        "gamesneeze",
        "zadey's indigo paste"
    };

    srand(time(NULL));
    auto randomMessage = rand() % 3;

    std::string killMessage = "";
    switch (randomMessage) {
    case 0:
        killMessage = "u sell that " + cheatNames[rand() % ARRAYSIZE(cheatNames)] + "?";
        break;
    case 1:
        killMessage = "nice " + cheatNames[rand() % ARRAYSIZE(cheatNames)];
        break;
    case 2:
        killMessage = "get good. get " + cheatNames[rand() % ARRAYSIZE(cheatNames)];
        break;
    }

    std::string cmd = "say \"";
    cmd += killMessage;
    cmd += '"';
    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::chickenDeathSay(GameEvent& event) noexcept
{
    if (!config->misc.chickenDeathSay)
        return;

    if (!localPlayer)
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") == localUserId)
        return;

    std::string deathMessages[] = {
        "Chick-fil-A Chicken Sandwich $3.05",
        "Chick-fil-A Chicken Sandwich - Combo $5.95",
        "Chick-fil-A Chicken Deluxe Sandwich $3.65",
        "Chick-fil-A Chicken Deluxe Sandwich Combo $6.55",
        "Spicy Chicken Sandwich $3.29",
        "Spicy Chicken Sandwich – Combo $6.19",
        "Spicy Chicken Deluxe Sandwich $3.89",
        "Spicy Chicken Deluxe Sandwich – Combo $6.79",
        "Chick-fil-A Nuggets 8 Pc. $3.05",
        "Chick-fil-A Nuggets 12 Pc. $4.45",
        "Chick-fil-A Nuggets – Combo 8 Pc. $5.95",
        "Chick-fil-A Nuggets – Combo 12 Pc. $8.59",
        "Chick-fil-A Nuggets (Grilled) 8 Pc. $3.85",
        "Chick-fil-A Nuggets (Grilled) 12 Pc. $5.75",
        "Chick-fil-A Nuggets (Grilled) – Combo 8 Pc. $6.75",
        "Chick-fil-A Nuggets (Grilled) – Combo 12 Pc. $8.59",
        "Chick-n-Strips 3 Pc. $3.35",
        "Chick-n-Strips 4 Pc. $4.39",
        "Chick-n-Strips – Combo 3 Pc. $6.25",
        "Chick-n-Strips – Combo 4 Pc. $7.25",
        "Grilled Chicken Sandwich $4.39",
        "Grilled Chicken Sandwich – Combo $7.19",
        "Grilled Chicken Club Sandwich $5.59",
        "Grilled Chicken Club Sandwich – Combo $8.39",
        "Chicken Salad Sandwich $3.99",
        "Chicken Salad Sandwich – Combo $6.79",
        "Grilled Chicken Cool Wrap $5.19",
        "Grilled Chicken Cool Wrap – Combo $8.15",
        "Soup & Salad (Large Chicken Soup and Side Salad) $8.35",
        "Chilled Grilled Chicken Sub Sandwich (Limited Time) $4.79"
    };

    srand(time(NULL));
    int randomMessage = rand() % ARRAYSIZE(deathMessages);

    std::string cmd = "say \"";
    cmd += deathMessages[randomMessage];
    cmd += '"';
    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::fixMovement(UserCmd* cmd, float yaw) noexcept
{
    float oldYaw = yaw + (yaw < 0.0f ? 360.0f : 0.0f);
    float newYaw = cmd->viewangles.y + (cmd->viewangles.y < 0.0f ? 360.0f : 0.0f);
    float yawDelta = newYaw < oldYaw ? fabsf(newYaw - oldYaw) : 360.0f - fabsf(newYaw - oldYaw);
    yawDelta = 360.0f - yawDelta;

    const float forwardmove = cmd->forwardmove;
    const float sidemove = cmd->sidemove;
    cmd->forwardmove = std::cos(degreesToRadians(yawDelta)) * forwardmove + std::cos(degreesToRadians(yawDelta + 90.0f)) * sidemove;
    cmd->sidemove = std::sin(degreesToRadians(yawDelta)) * forwardmove + std::sin(degreesToRadians(yawDelta + 90.0f)) * sidemove;
}

void Misc::antiAfkKick(UserCmd* cmd) noexcept
{
    if (config->misc.antiAfkKick && cmd->commandNumber % 2)
        cmd->buttons |= 1 << 26;
}

void Misc::autoPistol(UserCmd* cmd) noexcept
{
    if (config->misc.autoPistol && localPlayer) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->isPistol() && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime()) {
            if (activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver)
                cmd->buttons &= ~UserCmd::IN_ATTACK2;
            else
                cmd->buttons &= ~UserCmd::IN_ATTACK;
        }
    }
}

void Misc::revealRanks(UserCmd* cmd) noexcept
{
    if (config->misc.revealRanks && cmd->buttons & UserCmd::IN_SCORE)
        interfaces->client->dispatchUserMessage(50, 0, 0, nullptr);
}

float Misc::autoStrafe(UserCmd* cmd, const Vector& currentViewAngles) noexcept
{
    static float angle = 0.f;
    static float direction = 0.f;
    static float AutoStrafeAngle = 0.f;
    if (!config->misc.autoStrafe)
    {
        angle = 0.f;
        direction = 0.f;
        AutoStrafeAngle = 0.f;
        return AutoStrafeAngle;
    }
    if (!localPlayer || !localPlayer->isAlive())
        return 0.f;
    if (localPlayer->velocity().length2D() < 5.f)
    {
        angle = 0.f;
        direction = 0.f;
        AutoStrafeAngle = 0.f;
        return AutoStrafeAngle;
    }
    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER || (!(cmd->buttons & UserCmd::IN_JUMP)))
    {
        angle = 0.f;
        if (cmd->buttons & UserCmd::IN_FORWARD)
        {
            angle = 0.f;
            if (cmd->buttons & UserCmd::IN_MOVELEFT)
            {
                angle = 45.f;
            }
            else if (cmd->buttons & UserCmd::IN_MOVERIGHT)
            {
                angle = -45.f;
            }
        }
        if (!(cmd->buttons & (UserCmd::IN_FORWARD | UserCmd::IN_BACK)))
        {
            if (cmd->buttons & UserCmd::IN_MOVELEFT)
            {
                angle = 90.f;
            }
            if (cmd->buttons & UserCmd::IN_MOVERIGHT)
            {
                angle = -90.f;
            }
        }
        if (cmd->buttons & UserCmd::IN_BACK)
        {
            angle = 180.f;
            if (cmd->buttons & UserCmd::IN_MOVELEFT)
            {
                angle = 135.f;
            }
            else if (cmd->buttons & UserCmd::IN_MOVERIGHT)
            {
                angle = -135.f;
            }
        }
        direction = angle;
        AutoStrafeAngle = 0.f;
        return AutoStrafeAngle;
    }
    Vector base;
    interfaces->engine->getViewAngles(base);
    float delta = std::clamp(radiansToDegrees(std::atan2(15.f, localPlayer->velocity().length2D())), 0.f, 45.f);

    static bool flip = true;
    if (cmd->buttons & (UserCmd::IN_FORWARD | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT | UserCmd::IN_BACK))
    {
        cmd->forwardmove = 0;
        cmd->sidemove = 0;
        cmd->upmove = 0;
    }
    angle = 0.f;
    if (cmd->buttons & UserCmd::IN_FORWARD)
    {
        angle = 0.f;
        if (cmd->buttons & UserCmd::IN_MOVELEFT)
        {
            angle = 45.f;
        }
        else if (cmd->buttons & UserCmd::IN_MOVERIGHT)
        {
            angle = -45.f;
        }
    }
    if (!(cmd->buttons & (UserCmd::IN_FORWARD | UserCmd::IN_BACK)))
    {
        if (cmd->buttons & UserCmd::IN_MOVELEFT)
        {
            angle = 90.f;
        }
        if (cmd->buttons & UserCmd::IN_MOVERIGHT)
        {
            angle = -90.f;
        }
    }
    if (cmd->buttons & UserCmd::IN_BACK)
    {
        angle = 180.f;
        if (cmd->buttons & UserCmd::IN_MOVELEFT)
        {
            angle = 135.f;
        }
        else if (cmd->buttons & UserCmd::IN_MOVERIGHT)
        {
            angle = -135.f;
        }
    }
    if (std::abs(direction - angle) <= 180)
    {
        if (direction < angle)
        {
            direction += delta;
        }
        else
        {
            direction -= delta;
        }
    }
    else {
        if (direction < angle)
        {
            direction -= delta;
        }
        else
        {
            direction += delta;
        }
    }
    direction = std::isfinite(direction) ? std::remainder(direction, 360.0f) : 0.0f;
    if (cmd->mousedx < 0)
    {
        cmd->sidemove = -450.0f;
    }
    else if (cmd->mousedx > 0)
    {
        cmd->sidemove = 450.0f;
    }
    flip ? base.y += direction + delta : base.y += direction - delta;
    flip ? AutoStrafeAngle = direction + delta : AutoStrafeAngle = direction - delta;
    if (cmd->viewangles.y == currentViewAngles.y)
    {
        cmd->viewangles.y = base.y;
    }
    flip ? cmd->sidemove = 450.f : cmd->sidemove = -450.f;
    flip = !flip;
    return AutoStrafeAngle;
}

struct customCmd
{
    float forwardmove;
    float sidemove;
    float upmove;
};

bool hasShot;
Vector quickPeekStartPos;
Vector drawPos;
std::vector<customCmd>usercmdQuickpeek;
int qpCount;

void Misc::drawQuickPeekStartPos() noexcept
{
    if (!worldToScreen(quickPeekStartPos, drawPos))
        return;

    if (quickPeekStartPos != Vector{ 0, 0, 0 }) {
        interfaces->surface->setDrawColor(255, 255, 255);
        interfaces->surface->drawCircle(drawPos.x, drawPos.y, 0, 10);
    }
}

void gotoStart(UserCmd* cmd) {
    if (usercmdQuickpeek.empty()) return;
    if (hasShot)
    {
        if (qpCount > 0)
        {
            cmd->upmove = -usercmdQuickpeek.at(qpCount).upmove;
            cmd->sidemove = -usercmdQuickpeek.at(qpCount).sidemove;
            cmd->forwardmove = -usercmdQuickpeek.at(qpCount).forwardmove;
            qpCount--;
        }
    }
    else
    {
        qpCount = usercmdQuickpeek.size();
    }
}

void Misc::quickPeek(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive()) return;
    if (GetAsyncKeyState(config->misc.quickpeekkey)) {
        if (quickPeekStartPos == Vector{ 0, 0, 0 }) {
            quickPeekStartPos = localPlayer->getAbsOrigin();
        }
        else {
            customCmd tempCmd = {};
            tempCmd.forwardmove = cmd->forwardmove;
            tempCmd.sidemove = cmd->sidemove;
            tempCmd.upmove = cmd->upmove;

            if (cmd->buttons & UserCmd::IN_ATTACK) hasShot = true;
            gotoStart(cmd);

            if (!hasShot)
                usercmdQuickpeek.push_back(tempCmd);
        }
    }
    else {
        hasShot = false;
        quickPeekStartPos = Vector{ 0, 0, 0 };
        usercmdQuickpeek.clear();
    }
}

void Misc::removeCrouchCooldown(UserCmd* cmd) noexcept
{
    if (config->misc.fastDuck)
        cmd->buttons |= UserCmd::IN_BULLRUSH;
}

void Misc::moonwalk(UserCmd* cmd) noexcept
{
    if (config->misc.moonwalk && localPlayer && localPlayer->moveType() != MoveType::LADDER)
        cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
}

void Misc::playHitSound(GameEvent& event) noexcept
{
    if (!config->misc.hitSound)
        return;

    if (!localPlayer)
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    constexpr std::array hitSounds{
        "play physics/metal/metal_solid_impact_bullet2",
        "play buttons/arena_switch_press_02",
        "play training/timer_bell",
        "play physics/glass/glass_impact_bullet1"
    };

    if (static_cast<std::size_t>(config->misc.hitSound - 1) < hitSounds.size())
        interfaces->engine->clientCmdUnrestricted(hitSounds[config->misc.hitSound - 1]);
    else if (config->misc.hitSound == 5)
        interfaces->engine->clientCmdUnrestricted(("play " + config->misc.customHitSound).c_str());
}

void Misc::killSound(GameEvent& event) noexcept
{
    if (!config->misc.killSound)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    constexpr std::array killSounds{
        "play physics/metal/metal_solid_impact_bullet2",
        "play buttons/arena_switch_press_02",
        "play training/timer_bell",
        "play physics/glass/glass_impact_bullet1"
    };

    if (static_cast<std::size_t>(config->misc.killSound - 1) < killSounds.size())
        interfaces->engine->clientCmdUnrestricted(killSounds[config->misc.killSound - 1]);
    else if (config->misc.killSound == 5)
        interfaces->engine->clientCmdUnrestricted(("play " + config->misc.customKillSound).c_str());
}

void Misc::purchaseList(GameEvent* event) noexcept
{
    static std::mutex mtx;
    std::scoped_lock _{ mtx };

    static std::unordered_map<std::string, std::pair<std::vector<std::string>, int>> purchaseDetails;
    static std::unordered_map<std::string, int> purchaseTotal;
    static int totalCost;

    static auto freezeEnd = 0.0f;

    if (event) {
        switch (fnv::hashRuntime(event->getName())) {
        case fnv::hash("item_purchase"): {
            const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));

            if (player && localPlayer && memory->isOtherEnemy(player, localPlayer.get())) {
                const auto weaponName = event->getString("weapon");
                auto& purchase = purchaseDetails[player->getPlayerName(true)];

                if (const auto definition = memory->itemSystem()->getItemSchema()->getItemDefinitionByName(weaponName)) {
                    if (const auto weaponInfo = memory->weaponSystem->getWeaponInfo(definition->getWeaponId())) {
                        purchase.second += weaponInfo->price;
                        totalCost += weaponInfo->price;
                    }
                }
                std::string weapon = weaponName;

                if (weapon.starts_with("weapon_"))
                    weapon.erase(0, 7);
                else if (weapon.starts_with("item_"))
                    weapon.erase(0, 5);

                if (weapon.starts_with("smoke"))
                    weapon.erase(5);
                else if (weapon.starts_with("m4a1_s"))
                    weapon.erase(6);
                else if (weapon.starts_with("usp_s"))
                    weapon.erase(5);

                purchase.first.push_back(weapon);
                ++purchaseTotal[weapon];
            }
            break;
        }
        case fnv::hash("round_start"):
            freezeEnd = 0.0f;
            purchaseDetails.clear();
            purchaseTotal.clear();
            totalCost = 0;
            break;
        case fnv::hash("round_freeze_end"):
            freezeEnd = memory->globalVars->realtime;
            break;
        }
    } else {
        if (!config->misc.purchaseList.enabled)
            return;

        static const auto mp_buytime = interfaces->cvar->findVar("mp_buytime");

        if ((!interfaces->engine->isInGame() || freezeEnd != 0.0f && memory->globalVars->realtime > freezeEnd + (!config->misc.purchaseList.onlyDuringFreezeTime ? mp_buytime->getFloat() : 0.0f) || purchaseDetails.empty() || purchaseTotal.empty()) && !gui->open)
            return;

        ImGui::SetNextWindowSize({ 200.0f, 200.0f }, ImGuiCond_Once);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!gui->open)
            windowFlags |= ImGuiWindowFlags_NoInputs;
        if (config->misc.purchaseList.noTitleBar)
            windowFlags |= ImGuiWindowFlags_NoTitleBar;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
        ImGui::Begin("Purchases", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (config->misc.purchaseList.mode == PurchaseList::Details) {
            for (const auto& [playerName, purchases] : purchaseDetails) {
                std::string s = std::accumulate(purchases.first.begin(), purchases.first.end(), std::string{ }, [](std::string s, const std::string& piece) { return s += piece + ", "; });
                if (s.length() >= 2)
                    s.erase(s.length() - 2);

                if (config->misc.purchaseList.showPrices)
                    ImGui::TextWrapped("%s $%d: %s", playerName.c_str(), purchases.second, s.c_str());
                else
                    ImGui::TextWrapped("%s: %s", playerName.c_str(), s.c_str());
            }
        } else if (config->misc.purchaseList.mode == PurchaseList::Summary) {
            for (const auto& purchase : purchaseTotal)
                ImGui::TextWrapped("%d x %s", purchase.second, purchase.first.c_str());

            if (config->misc.purchaseList.showPrices && totalCost > 0) {
                ImGui::Separator();
                ImGui::TextWrapped("Total: $%d", totalCost);
            }
        }
        ImGui::End();
    }
}

void Misc::autoBuy(GameEvent* event) noexcept
{
    std::array<std::string, 17> primary = {
        "",
        "buy mac10;buy mp9;",
        "buy mp7;",
        "buy ump45;",
        "buy p90;",
        "buy bizon;",
        "buy galilar;buy famas;",
        "buy ak47;buy m4a1;",
        "buy ssg08;",
        "buy sg556;buy aug;",
        "buy awp;",
        "buy g3sg1; buy scar20;",
        "buy nova;",
        "buy xm1014;",
        "buy sawedoff;buy mag7;",
        "buy m249;",
        "buy negev;"
    };
    std::array<std::string, 6> secondary = {
        "",
        "buy glock;buy hkp2000",
        "buy elite;",
        "buy p250;",
        "buy tec9;buy fiveseven;",
        "buy deagle;buy revolver;"
    };
    std::array<std::string, 3> armor = {
        "",
        "buy vest;",
        "buy vesthelm;",
    };
    std::array<std::string, 2> utility = {
        "buy defuser;",
        "buy taser;"
    };
    std::array<std::string, 5> nades = {
        "buy hegrenade;",
        "buy smokegrenade;",
        "buy molotov;buy incgrenade;",
        "buy flashbang;buy flashbang;",
        "buy decoy;"
    };

    if (!config->misc.autoBuy.enabled)
        return;

    std::string cmd = "";

    if (event) {
        if (fnv::hashRuntime(event->getName()) == fnv::hash("round_start")) {
            cmd += primary[config->misc.autoBuy.primaryWeapon];
            cmd += secondary[config->misc.autoBuy.secondaryWeapon];
            cmd += armor[config->misc.autoBuy.armor];

            for (int i = 0; i < ARRAYSIZE(config->misc.autoBuy.utility); i++)
            {
                if (config->misc.autoBuy.utility[i])
                    cmd += utility[i];
            }

            for (int i = 0; i < ARRAYSIZE(config->misc.autoBuy.grenades); i++)
            {
                if (config->misc.autoBuy.grenades[i])
                    cmd += nades[i];
            }

            interfaces->engine->clientCmdUnrestricted(cmd.c_str());
        }
    }
}



void Misc::oppositeHandKnife(FrameStage stage) noexcept
{
    if (!config->misc.oppositeHandKnife)
        return;

    if (!localPlayer)
        return;

    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static const auto cl_righthand = interfaces->cvar->findVar("cl_righthand");
    static bool original;

    if (stage == FrameStage::RENDER_START) {
        original = cl_righthand->getInt();

        if (const auto activeWeapon = localPlayer->getActiveWeapon()) {
            if (const auto classId = activeWeapon->getClientClass()->classId; classId == ClassId::Knife || classId == ClassId::KnifeGG)
                cl_righthand->setValue(!original);
        }
    }
    else {
        cl_righthand->setValue(original);
    }
}



void Misc::preserveDeathNotices(GameEvent* event) noexcept
{
    bool freezeTime = true;
    static int* deathNotice;
    static bool reallocatedDeathNoticeHUD{ false };

    /*

    if (!strcmp(event->getName(), "round_prestart") || !strcmp(event->getName(), "round_end")) {
        if (!strcmp(event->getName(), "round_end")) {
            freezeTime = false;
        } 
        else {
            freezeTime = true;
        }
    }

    if (!strcmp(event->getName(), "round_freeze_end"))
        freezeTime = false;

    if (!strcmp(event->getName(), "round_end"))
        freezeTime = true;

    if (!reallocatedDeathNoticeHUD)
    {
        reallocatedDeathNoticeHUD = true;
        deathNotice = memory->findHudElement(memory->hud, "CCSGO_HudDeathNotice");
    }
    else
    {
        if (deathNotice)
        {
            if (!freezeTime) {
                float* localDeathNotice = (float*)((DWORD)deathNotice + 0x50);
                if (localDeathNotice)
                    *localDeathNotice = config->misc.preserveDeathNotices ? FLT_MAX : 1.5f;
            }

            if (freezeTime && deathNotice - 20)
            {
                if (memory->clearDeathNotices)
                    memory->clearDeathNotices(((DWORD)deathNotice - 20));
            }
        }
    }
    */

}

void Misc::autoDisconnect(GameEvent* event) noexcept
{
    if (!config->misc.autoDisconnect)
        return;

    interfaces->engine->clientCmdUnrestricted("disconnect;");
}