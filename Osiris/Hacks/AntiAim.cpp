#include "Animations.h"
#include "AntiAim.h"
#include "Tickbase.h"

#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/UserCmd.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Surface.h"
#include "../SDK/GameEvent.h"


#include "../Interfaces.h"
#include "../Memory.h"
#include "../Hooks.h"

int RandomInt(int min, int max) noexcept
{
    return (min + 1) + (((int)rand()) / (int)RAND_MAX) * (max - (min + 1));
}

float RandomFloat(float min, float max) noexcept
{
    return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
}

bool RunAA(UserCmd* cmd) noexcept {
    if (!localPlayer || !localPlayer->isAlive())
        return true;

    if (localPlayer.get()->moveType() == MoveType::LADDER || localPlayer.get()->moveType() == MoveType::NOCLIP)
        return true;

    auto activeWeapon = localPlayer.get()->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return false;

    if (activeWeapon->isThrowing())
        return true;

    if (activeWeapon->isGrenade())
        return false;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto() || localPlayer->waitForNoAttack())
        return false;

    if (localPlayer.get()->nextAttack() > memory->globalVars->serverTime())
        return false;

    if (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return false;

    if (activeWeapon->nextSecondaryAttack() > memory->globalVars->serverTime())
        return false;

    if (localPlayer.get()->nextAttack() <= memory->globalVars->serverTime() && (cmd->buttons & (UserCmd::IN_ATTACK)))
        return true;

    if (activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && (cmd->buttons & (UserCmd::IN_ATTACK)))
        return true;

    if (activeWeapon->isKnife())
    {
        if (activeWeapon->nextSecondaryAttack() > memory->globalVars->serverTime())
            return false;

        if (activeWeapon->nextSecondaryAttack() <= memory->globalVars->serverTime() && (cmd->buttons & (UserCmd::IN_ATTACK2)))
            return true;
    }
    if (activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver && activeWeapon->readyTime() > memory->globalVars->serverTime())
        return false;

    if ((activeWeapon->itemDefinitionIndex2() == WeaponId::Famas || activeWeapon->itemDefinitionIndex2() == WeaponId::Glock) && activeWeapon->burstMode() && activeWeapon->burstShotRemaining() > 0)
        return false;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex2());
    if (!weaponIndex)
        return false;

    return false;
}


bool isLbyUpdating() noexcept
{
    static float Update = 0.f;
    if (!(localPlayer->flags() & 1) || !localPlayer->getAnimstate())
        return false;
    if (localPlayer->velocity().length2D() > 0.1f || fabsf(localPlayer->getAnimstate()->UpVelocity) > 100.f)
    {
        Update = memory->globalVars->serverTime() + 0.22f;
    }
    if (Update < memory->globalVars->serverTime())
    {
        Update = memory->globalVars->serverTime() + 1.1f;
        return true;
    }
    return false;
}

bool AutoDir(Entity* entity, Vector eye_) noexcept
{
    constexpr float maxRange{ 8192.0f };

    Vector eye = eye_;
    eye.x = 0.f;
    Vector eyeAnglesLeft45 = eye;
    Vector eyeAnglesRight45 = eye;
    eyeAnglesLeft45.y += 45.f;
    eyeAnglesRight45.y -= 45.f;


    Vector viewAnglesLeft45{ cos(degreesToRadians(eyeAnglesLeft45.x)) * cos(degreesToRadians(eyeAnglesLeft45.y)) * maxRange,
               cos(degreesToRadians(eyeAnglesLeft45.x)) * sin(degreesToRadians(eyeAnglesLeft45.y)) * maxRange,
              -sin(degreesToRadians(eyeAnglesLeft45.x)) * maxRange };

    Vector viewAnglesRight45{ cos(degreesToRadians(eyeAnglesRight45.x)) * cos(degreesToRadians(eyeAnglesRight45.y)) * maxRange,
                       cos(degreesToRadians(eyeAnglesRight45.x)) * sin(degreesToRadians(eyeAnglesRight45.y)) * maxRange,
                      -sin(degreesToRadians(eyeAnglesRight45.x)) * maxRange };

    static Trace traceLeft45;
    static Trace traceRight45;

    Vector headPosition{ localPlayer->getBonePosition(8) };

    interfaces->engineTrace->traceRay({ headPosition, headPosition + viewAnglesLeft45 }, 0x4600400B, { entity }, traceLeft45);
    interfaces->engineTrace->traceRay({ headPosition, headPosition + viewAnglesRight45 }, 0x4600400B, { entity }, traceRight45);

    float distanceLeft45 = sqrt(pow(headPosition.x - traceRight45.endpos.x, 2) + pow(headPosition.y - traceRight45.endpos.y, 2) + pow(headPosition.z - traceRight45.endpos.z, 2));
    float distanceRight45 = sqrt(pow(headPosition.x - traceLeft45.endpos.x, 2) + pow(headPosition.y - traceLeft45.endpos.y, 2) + pow(headPosition.z - traceLeft45.endpos.z, 2));

    float mindistance = std::min(distanceLeft45, distanceRight45);

    if (distanceLeft45 == mindistance) {
        return false;
    }
    return true;
}


void AntiAim::run(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;
    bool lby = isLbyUpdating();
    Animations::data.lby = lby;
    if (config->antiAim.enabled) {
        if (RunAA(cmd))
            return;

        if (cmd->buttons & (UserCmd::IN_USE))
            return;

        static float lastTime{ 0.f };
        static bool invert{ true };
        static bool desyncInvert{ true };
        static float yawOffset{ 0.f };
        static bool flipJitter{ false };
        static bool flipLby{ false };
        static float desyncDelta{ 0 };
        static float lbyRange{ 0 };
        static float deltaMultiplier{ localPlayer->getMaxDesyncAngle() / 58.f };

        if (config->antiAim.mode == 0) {
            cmd->viewangles.x = cmd->viewangles.x;
            yawOffset = 0.f;
        }

        if (config->antiAim.mode == 1 && cmd->viewangles.x == currentViewAngles.x){
            switch (config->antiAim.pitchAngle) {
            case 0: //Viewangles
                cmd->viewangles.x = cmd->viewangles.x;
                break;
            case 1: //Down
                cmd->viewangles.x = 89.f;
                break;
            case 2: //Zero (Always forward)
                cmd->viewangles.x = 0.f;
                break;
            case 3: //Up
                cmd->viewangles.x = -89.f;
                break;
            }
        }

        if (config->antiAim.mode == 1) {
            switch (config->antiAim.yawAngle) {
            case 0: //Forward (Legit)
                yawOffset = 0.f;
                break;
            case 1: //Back
                yawOffset = 180.f;
                break;
            case 2: //Forward Jitter (Legit Jitter)
                yawOffset = flipJitter ? 0.f + config->antiAim.jitterRange : 0.f - config->antiAim.jitterRange;
                break;
            case 3: //Back Jitter (Back Jitter)
                yawOffset = flipJitter ? 180.f + config->antiAim.jitterRange : 180.f - config->antiAim.jitterRange;
                break;
            case 4: //Fast Spin
                yawOffset += 45.f;
                break;
            case 5: //Slow spin
                yawOffset += 15.f;
                break;
            }
        }

        if (GetAsyncKeyState(config->antiAim.invert) && memory->globalVars->realtime - lastTime > 0.5f && !config->antiAim.autodir) {
            invert ^= 1;
            lastTime = memory->globalVars->realtime;
        }
        if (cmd->viewangles.y == currentViewAngles.y) {
            static float sent = 0.f;
            if (sendPacket) {
                flipJitter ^= 1;
                cmd->viewangles.y += yawOffset;
                sent = cmd->viewangles.y;
            }

            if (config->antiAim.autodir) {
                invert = AutoDir(localPlayer.get(), cmd->viewangles);
                if (config->antiAim.mode != 0 && (config->antiAim.yawAngle == 1 || config->antiAim.yawAngle == 3)) //Dirty fix
                    invert ^= 1;
            }

            switch (config->antiAim.lbyDelta) {
            case 0: //Minimum
                lbyRange = 22.f;
                break;
            case 1: //Maximum
                lbyRange = 119.95f;
            }

            if (lby) {
                sendPacket = false;
                if (!config->antiAim.lbySway) {
                    invert ? cmd->viewangles.y = sent - lbyRange : cmd->viewangles.y = sent + lbyRange;
                }
                else {
                    flipLby ? cmd->viewangles.y = sent - lbyRange : cmd->viewangles.y = sent + lbyRange;
                    flipLby ^= 1;
                }
                return;
            }

            desyncDelta = (58.f * 2) * deltaMultiplier;
            if (!sendPacket) {
                invert ? cmd->viewangles.y = sent + desyncDelta : cmd->viewangles.y = sent - desyncDelta;
            }
        }
    }
}

void AntiAim::fakeLag(UserCmd* cmd, bool& sendPacket) noexcept
{
    auto chokedPackets = 0;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    chokedPackets = config->antiAim.enabled ? 2 : 0;

    if (config->antiAim.fakeLag) {
        switch (config->antiAim.fakeLagMode) {
        case 0: //Maximum
            chokedPackets = std::clamp(config->antiAim.fakeLagAmount, 1, 16);
            break;
        case 1: //Adaptive
            chokedPackets = std::clamp(static_cast<int>(std::ceilf(64 / (localPlayer->velocity().length() * memory->globalVars->intervalPerTick))), 1, config->antiAim.fakeLagAmount);
            break;
        case 2: //Random
            chokedPackets = RandomInt(1, config->antiAim.fakeLagAmount);
        }
    }

    if (config->antiAim.fakeDucking)
        chokedPackets = 14;
    Tickbase::tick->fakeLag = chokedPackets;
    if (RunAA(cmd) && !config->antiAim.fakeDucking)
        return;

    sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= chokedPackets;
}

void AntiAim::fakeDuck(UserCmd* cmd) noexcept
{
    static float lastTime{ 0.f };
    if (GetAsyncKeyState(config->antiAim.fakeDuckKey) && memory->globalVars->realtime - lastTime > 0.5f)
    {
        lastTime = memory->globalVars->realtime;
        config->antiAim.fakeDucking = !config->antiAim.fakeDucking;
    }
    if (!localPlayer || !localPlayer->isAlive() || !config->antiAim.fakeDucking)
        return;

    if (interfaces->engine->getNetworkChannel()->chokedPackets > 7)
        cmd->buttons |= UserCmd::IN_DUCK;
    else
        cmd->buttons &= ~UserCmd::IN_DUCK;
}