#include <Windows.h>

#include "Aimbot.h"
#include "Animations.h"
#include "Backtrack.h"
#include "Triggerbot.h"

#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/UserCmd.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/Angle.h"
#include "../SDK/StudioRender.h"
#include "../SDK/ModelInfo.h"

#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"

enum Hitboxes
{
    HEAD,
    NECK,
    PELVIS,
    STOMACH,
    LOWER_CHEST,
    CHEST,
    UPPER_CHEST,
    RIGHT_THIGH,
    LEFT_THIGH,
    RIGHT_CALF,
    LEFT_CALF,
    RIGHT_FOOT,
    LEFT_FOOT,
    RIGHT_HAND,
    LEFT_HAND,
    RIGHT_UPPER_ARM,
    RIGHT_FOREARM,
    LEFT_UPPER_ARM,
    LEFT_FOREARM,
    MAX
};

static void setRandomSeed(int seed) noexcept
{
    using randomSeedFn = void(*)(int);
    static auto randomSeed{ reinterpret_cast<randomSeedFn>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed")) };
    randomSeed(seed);
}

static float getRandom(float min, float max) noexcept
{
    using randomFloatFn = float(*)(float, float);
    static auto randomFloat{ reinterpret_cast<randomFloatFn>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat")) };
    return randomFloat(min, max);
}

static bool hitChance(Entity* localPlayer, Entity* entity, Entity* weaponData, const Vector& destination, const UserCmd* cmd, const int hitChance) noexcept
{
    if (!hitChance)
        return true;

    constexpr int maxSeed = 256;

    const Angle angles(destination + cmd->viewangles);

    int hits = 0;
    const int hitsNeed = static_cast<int>(static_cast<float>(maxSeed) * (static_cast<float>(hitChance) / 100.f));

    const auto weapSpread = weaponData->getSpread();
    const auto weapInaccuracy = weaponData->getInaccuracy();
    const auto localEyePosition = localPlayer->getEyePosition();
    const auto range = weaponData->getWeaponData()->range;

    for (int i = 0; i < maxSeed; ++i)
    {
        setRandomSeed(i + 1);
        float inaccuracy = getRandom(0.f, 1.f);
        float spread = getRandom(0.f, 1.f);
        const float spreadX = getRandom(0.f, 2.f * static_cast<float>(M_PI));
        const float spreadY = getRandom(0.f, 2.f * static_cast<float>(M_PI));

        const auto weaponIndex = weaponData->itemDefinitionIndex2();
        const auto recoilIndex = weaponData->recoilIndex();
        if (weaponIndex == WeaponId::Revolver)
        {
            if (cmd->buttons & UserCmd::IN_ATTACK2)
            {
                inaccuracy = 1.f - inaccuracy * inaccuracy;
                spread = 1.f - spread * spread;
            }
        }
        else if (weaponIndex == WeaponId::Negev && recoilIndex < 3.f)
        {
            for (int i = 3; i > recoilIndex; --i)
            {
                inaccuracy *= inaccuracy;
                spread *= spread;
            }

            inaccuracy = 1.f - inaccuracy;
            spread = 1.f - spread;
        }

        inaccuracy *= weapInaccuracy;
        spread *= weapSpread;

        Vector spreadView{ (cosf(spreadX) * inaccuracy) + (cosf(spreadY) * spread),
                           (sinf(spreadX) * inaccuracy) + (sinf(spreadY) * spread) };
        Vector direction{ (angles.forward + (angles.right * spreadView.x) + (angles.up * spreadView.y)) * range };

        static Trace trace;
        interfaces->engineTrace->clipRayToEntity({ localEyePosition, localEyePosition + direction }, 0x4600400B, entity, trace);
        if (trace.entity == entity)
            ++hits;

        if (hits >= hitsNeed)
            return true;

        if ((maxSeed - i + hits) < hitsNeed)
            return false;
    }
    return false;
}

void Triggerbot::run(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive() || localPlayer->nextAttack() > memory->globalVars->serverTime())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip() || activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex2());
    if (!weaponIndex)
        return;

    if (!config->triggerbot[weaponIndex].enabled)
        weaponIndex = getWeaponClass1(activeWeapon->itemDefinitionIndex2());

    if (!config->triggerbot[weaponIndex].enabled)
        weaponIndex = 0;

    const auto& cfg = config->triggerbot[weaponIndex];

    if (!cfg.enabled)
        return;

    static auto lastTime = 0.0f;

    const auto now = memory->globalVars->realtime;


    if (cfg.onKey && !GetAsyncKeyState(cfg.key))
        return;

    if (now - lastTime < cfg.shotDelay / 1000.0f)
        return;

    if (!cfg.ignoreFlash && localPlayer->isFlashed())
        return;

    if (cfg.scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    const auto aimPunch = localPlayer->getAimPunch();

    const Vector viewAngles{ std::cos(degreesToRadians(cmd->viewangles.x + aimPunch.x)) * std::cos(degreesToRadians(cmd->viewangles.y + aimPunch.y)) * weaponData->range,
                             std::cos(degreesToRadians(cmd->viewangles.x + aimPunch.x)) * std::sin(degreesToRadians(cmd->viewangles.y + aimPunch.y)) * weaponData->range,
                            -std::sin(degreesToRadians(cmd->viewangles.x + aimPunch.x)) * weaponData->range };

    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + viewAngles;

    Trace trace;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, localPlayer.get(), trace);

    lastTime = now;

    if (!cfg.ignoreSmoke && memory->lineGoesThroughSmoke(startPos, endPos, 1))
        return;

    if (config->backtrack.enabled && (!trace.entity || !trace.entity->isPlayer()))
    {
        auto bestFov{ 255.f };
        Vector bestTargetHead{ };
        int bestRecord = -1;
        int bestTargetIndex{ };
        Entity* bestTarget{ };

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()))
                continue;

            Animations::finishSetup(entity);

            auto head = entity->getBonePosition(8);

            auto angle = Aimbot::calculateRelativeAngle(localPlayer->getEyePosition(), head, cmd->viewangles + aimPunch);
            auto fov = std::hypotf(angle.x, angle.y);
            if (fov < bestFov) {
                bestFov = fov;
                bestTarget = entity;
                bestTargetIndex = i;
                bestTargetHead = head;
            }
        }

        if (bestTarget) {
            if (Backtrack::records[bestTargetIndex].size() <= 3 || (!config->backtrack.ignoreSmoke && memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), bestTargetHead, 1)))
                return;

            bestFov = 255.f;

            for (size_t i = 0; i < Backtrack::records[bestTargetIndex].size(); i++) {
                auto& record = Backtrack::records[bestTargetIndex][i];
                if (!Backtrack::valid(record.simulationTime))
                    continue;

                auto angle = Aimbot::calculateRelativeAngle(localPlayer->getEyePosition(), record.head, cmd->viewangles + aimPunch);
                auto fov = std::hypotf(angle.x, angle.y);
                if (fov < bestFov) {
                    bestFov = fov;
                    bestRecord = i;
                }
            }
        }

        if (bestRecord != -1) {
            auto record = Backtrack::records[bestTargetIndex][bestRecord];
            Animations::setup(bestTarget, record);
            cmd->tickCount = Backtrack::timeToTicks(record.simulationTime + Backtrack::getLerp());
            interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, localPlayer.get(), trace);
        }
    }

    if (!trace.entity || !trace.entity->isPlayer())
        return;

    if (!cfg.friendlyFire && !localPlayer->isOtherEnemy(trace.entity))
        return;

    if (trace.entity->gunGameImmunity())
        return;

    bool hitbox[19]{ false };
    for (int i = 0; i < ARRAYSIZE(config->triggerbot[weaponIndex].hitboxes); i++)
    {
        switch (i)
        {
        case 0: //Head
            hitbox[Hitboxes::HEAD] = config->triggerbot[weaponIndex].hitboxes[i];
            break;
        case 1: //Chest
            hitbox[Hitboxes::UPPER_CHEST] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::CHEST] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::LOWER_CHEST] = config->triggerbot[weaponIndex].hitboxes[i];
            break;
        case 2: //Stomach
            hitbox[Hitboxes::STOMACH] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::PELVIS] = config->triggerbot[weaponIndex].hitboxes[i];
            break;
        case 3: //Arms
            hitbox[Hitboxes::RIGHT_UPPER_ARM] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::RIGHT_FOREARM] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::LEFT_UPPER_ARM] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::LEFT_FOREARM] = config->triggerbot[weaponIndex].hitboxes[i];
            break;
        case 4: //Legs
            hitbox[Hitboxes::RIGHT_CALF] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::RIGHT_THIGH] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::LEFT_CALF] = config->triggerbot[weaponIndex].hitboxes[i];
            hitbox[Hitboxes::LEFT_THIGH] = config->triggerbot[weaponIndex].hitboxes[i];
            break;
        default:
            break;
        }
    }
    float damage = -1;
    bool hitchance = false;
    for (int x = 0; x < 19; x++)
    {
        if (!(hitbox[x]))
            continue;
        if (trace.hitbox != x)
            continue;
        damage = (activeWeapon->itemDefinitionIndex2() != WeaponId::Taser ? HitGroup::getDamageMultiplier(trace.hitgroup) : 1.0f) * weaponData->damage * std::pow(weaponData->rangeModifier, trace.fraction * weaponData->range / 500.0f);
        hitchance = hitChance(localPlayer.get(), trace.entity, activeWeapon, Aimbot::calculateRelativeAngle(localPlayer->getEyePosition(), trace.endpos, cmd->viewangles + aimPunch), cmd, config->triggerbot[weaponIndex].hitChance);
    }
    Animations::finishSetup(trace.entity);
    if (damage <= -1 || !hitchance)
        return;

    if (float armorRatio{ weaponData->armorRatio / 2.0f }; activeWeapon->itemDefinitionIndex2() != WeaponId::Taser && HitGroup::isArmored(trace.hitgroup, trace.entity->hasHelmet()))
        damage -= (trace.entity->armor() < damage * armorRatio / 2.0f ? trace.entity->armor() * 4.0f : damage) * (1.0f - armorRatio);

    if (damage >= (cfg.killshot ? trace.entity->health() : cfg.minDamage)) {
        cmd->buttons |= UserCmd::IN_ATTACK;
        lastTime = 0.0f;
    }
}
