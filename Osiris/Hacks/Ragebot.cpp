#include "Ragebot.h"
#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/Angle.h"
#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/WeaponData.h"
#include "../SDK/StudioRender.h"
#include "../SDK/ModelInfo.h"
#include "Aimbot.h"
#include "Backtrack.h"
#include "Animations.h"

static float handleBulletPenetration(SurfaceData* enterSurfaceData, const Trace& enterTrace, const Vector& direction, Vector& result, float penetration, float damage) noexcept
{
    Vector end;
    Trace exitTrace;
    __asm {
        mov ecx, end
        mov edx, enterTrace
    }
    if (!memory->traceToExit(enterTrace.endpos.x, enterTrace.endpos.y, enterTrace.endpos.z, direction.x, direction.y, direction.z, exitTrace))
        return -1.0f;

    SurfaceData* exitSurfaceData = interfaces->physicsSurfaceProps->getSurfaceData(exitTrace.surface.surfaceProps);

    float damageModifier = 0.16f;
    float penetrationModifier = (enterSurfaceData->penetrationmodifier + exitSurfaceData->penetrationmodifier) / 2.0f;

    if (enterSurfaceData->material == 71 || enterSurfaceData->material == 89) {
        damageModifier = 0.05f;
        penetrationModifier = 3.0f;
    }
    else if (enterTrace.contents >> 3 & 1 || enterTrace.surface.flags >> 7 & 1) {
        penetrationModifier = 1.0f;
    }

    if (enterSurfaceData->material == exitSurfaceData->material) {
        if (exitSurfaceData->material == 85 || exitSurfaceData->material == 87)
            penetrationModifier = 3.0f;
        else if (exitSurfaceData->material == 76)
            penetrationModifier = 2.0f;
    }

    damage -= 11.25f / penetration / penetrationModifier + damage * damageModifier + (exitTrace.endpos - enterTrace.endpos).squareLength() / 24.0f / penetrationModifier;

    result = exitTrace.endpos;
    return damage;
}

static float canScan(Entity* entity, const Vector& destination, const WeaponInfo* weaponData, bool allowFriendlyFire) noexcept
{
    if (!localPlayer)
        return 0;

    float damage{ static_cast<float>(weaponData->damage) };

    Vector start{ localPlayer->getEyePosition() };
    Vector direction{ destination - start };
    direction /= direction.length();

    int hitsLeft = 4;

    while (damage >= 1.0f && hitsLeft) {
        Trace trace;
        interfaces->engineTrace->traceRay({ start, destination }, 0x4600400B, localPlayer.get(), trace);

        if (!allowFriendlyFire && trace.entity && trace.entity->isPlayer() && !localPlayer->isOtherEnemy(trace.entity))
            return false;

        if (trace.entity && trace.entity->isPlayer() && trace.entity == entity && trace.hitgroup > HitGroup::Generic && trace.hitgroup <= HitGroup::RightLeg) {
            damage = HitGroup::getDamageMultiplier(trace.hitgroup) * damage * powf(weaponData->rangeModifier, trace.fraction * weaponData->range / 500.0f);

            if (float armorRatio{ weaponData->armorRatio / 2.0f }; HitGroup::isArmored(trace.hitgroup, trace.entity->hasHelmet()))
                damage -= (trace.entity->armor() < damage * armorRatio / 2.0f ? trace.entity->armor() * 4.0f : damage) * (1.0f - armorRatio);

            return damage;
        }

        if (trace.fraction == 1.0f)
            break;

        const auto surfaceData = interfaces->physicsSurfaceProps->getSurfaceData(trace.surface.surfaceProps);

        if (surfaceData->penetrationmodifier < 0.1f)
            break;

        damage = handleBulletPenetration(surfaceData, trace, direction, start, weaponData->penetration, damage);
        hitsLeft--;
    }
    return 0;
}

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

std::vector<Vector> Multipoint(Entity* entity, matrix3x4 matrix[256], int iHitbox, int weaponIndex, bool firstScan)
{
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

    auto VectorTransform_Wrapper = [](const Vector& in1, const matrix3x4 in2, Vector& out)
    {
        auto VectorTransform = [](const float* in1, const matrix3x4 in2, float* out)
        {
            auto DotProducts = [](const float* v1, const float* v2)
            {
                return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
            };
            out[0] = DotProducts(in1, in2[0]) + in2[0][3];
            out[1] = DotProducts(in1, in2[1]) + in2[1][3];
            out[2] = DotProducts(in1, in2[2]) + in2[2][3];
        };
        VectorTransform(&in1.x, in2, &out.x);
    };

    const Model* mod = entity->getModel();
    if (!mod)
        return {};
    StudioHdr* hdr = interfaces->modelInfo->getStudioModel(mod);
    if (!hdr)
        return {};
    StudioHitboxSet* set = hdr->getHitboxSet(0);
    if (!set)
        return {};
    StudioBbox* hitbox = set->getHitbox(iHitbox);
    if (!hitbox)
        return {};
    Vector vMin, vMax, vCenter;
    VectorTransform_Wrapper(hitbox->bbMin, matrix[hitbox->bone], vMin);
    VectorTransform_Wrapper(hitbox->bbMax, matrix[hitbox->bone], vMax);
    vCenter = (vMin + vMax) * 0.5f;

    std::vector<Vector> vecArray;

    if (config->ragebot[weaponIndex].multiPoint == 0 || firstScan)
    {
        vecArray.emplace_back(vCenter);
        return vecArray;
    }

    Vector CurrentAngles = Aimbot::calculateRelativeAngle(vCenter, localPlayer->getEyePosition(), Vector{});

    Vector Forward;
    AngleVectors(CurrentAngles, &Forward);

    Vector Right = Forward.Cross(Vector{ 0, 0, 1 });
    Vector Left = Vector{ -Right.x, -Right.y, Right.z };

    Vector Top = Vector{ 0, 0, 1 };
    Vector Bot = Vector{ 0, 0, -1 };

    float multiPoint = (min(config->ragebot[weaponIndex].multiPoint, 95)) * 0.01f;

    switch (iHitbox) {
    case HEAD:
        for (auto i = 0; i < 4; ++i)
            vecArray.emplace_back(vCenter);

        vecArray[1] += Top * (hitbox->capsuleRadius * multiPoint);
        vecArray[2] += Right * (hitbox->capsuleRadius * multiPoint);
        vecArray[3] += Left * (hitbox->capsuleRadius * multiPoint);
        break;
    default://rest
        for (auto i = 0; i < 3; ++i)
            vecArray.emplace_back(vCenter);

        vecArray[1] += Right * (hitbox->capsuleRadius * multiPoint);
        vecArray[2] += Left * (hitbox->capsuleRadius * multiPoint);
        break;
    }
    return vecArray;
}

std::array<bool, 40> shouldRunAutoStop;

void Ragebot::autoStop(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex2());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass2(activeWeapon->itemDefinitionIndex2());
    if (!config->ragebot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->ragebot[weaponIndex].enabled)
        weaponIndex = 0;

    if (!config->ragebot[weaponIndex].autoStop || !shouldRunAutoStop.at(weaponIndex))
        return;

    if (!config->ragebot[weaponIndex].betweenShots && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    if (!config->ragebot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->ragebot[weaponIndex].onKey) {
        if (!config->ragebot[weaponIndex].keyMode) {
            if (!GetAsyncKeyState(config->ragebot[weaponIndex].key))
                return;
        }
        else {
            static bool toggle = true;
            if (GetAsyncKeyState(config->ragebot[weaponIndex].key) & 1)
                toggle = !toggle;
            if (!toggle)
                return;
        }
    }

    if (config->ragebot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->ragebot[weaponIndex].autoShot))
    {
        if (config->ragebot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
            return;

        const auto weaponData = activeWeapon->getWeaponData();
        if (!weaponData)
            return;

        const float maxSpeed = (localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3;

        if (cmd->forwardmove && cmd->sidemove) {
            const float maxSpeedRoot = maxSpeed * static_cast<float>(M_SQRT1_2);
            cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
            cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
        }
        else if (cmd->forwardmove) {
            cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeed : maxSpeed;
        }
        else if (cmd->sidemove) {
            cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeed : maxSpeed;
        }
    }
    shouldRunAutoStop.at(weaponIndex) = false;
}

void Ragebot::run(UserCmd* cmd) noexcept
{
    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex2());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass2(activeWeapon->itemDefinitionIndex2());
    if (!config->ragebot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->ragebot[weaponIndex].enabled)
        weaponIndex = 0;

    if (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    if (!config->ragebot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->ragebot[weaponIndex].onKey) {
        if (!config->ragebot[weaponIndex].keyMode) {
            if (!GetAsyncKeyState(config->ragebot[weaponIndex].key))
                return;
        }
        else {
            static bool toggle = true;
            if (GetAsyncKeyState(config->ragebot[weaponIndex].key) & 1)
                toggle = !toggle;
            if (!toggle)
                return;
        }
    }

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    if (config->ragebot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->ragebot[weaponIndex].autoShot)) {

        auto bestFov = config->ragebot[weaponIndex].fov;
        auto bestDamage = GetAsyncKeyState(config->ragebot[weaponIndex].overrideDamageKey) ? config->ragebot[weaponIndex].overrideDamageAmount : config->ragebot[weaponIndex].minDamage;
        Vector bestTarget{ };
        Vector bestAngle{ };
        Vector finalAngle{ };
        auto localPlayerEyePosition = localPlayer->getEyePosition();

        std::array<bool, 19> hitbox{ false };

        const auto aimPunch = localPlayer->getAimPunch();

        std::vector<Ragebot::Enemies> enemies;
        const auto localPlayerOrigin{ localPlayer->getAbsOrigin() };
        for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
            const auto entity{ interfaces->entityList->getEntity(i) };
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) && !config->ragebot[weaponIndex].friendlyFire || entity->gunGameImmunity())
                continue;

            const auto angle{ Aimbot::calculateRelativeAngle(localPlayerEyePosition, entity->getBonePosition(8), cmd->viewangles + aimPunch) };
            const auto origin{ entity->getAbsOrigin() };
            const auto fov{ angle.length2D() }; //fov
            const auto health{ entity->health() }; //health
            const auto distance{ localPlayerOrigin.distance(origin) }; //distance
            enemies.emplace_back(i, health, distance, fov);
        }
        switch (config->ragebot[weaponIndex].priority)
        {
        case 0:
            std::sort(enemies.begin(), enemies.end(), healthSort);
            break;
        case 1:
            std::sort(enemies.begin(), enemies.end(), distanceSort);
            break;
        case 2:
            std::sort(enemies.begin(), enemies.end(), fovSort);
            break;
        default:
            break;
        }
        for (int i = 0; i < ARRAYSIZE(config->ragebot[weaponIndex].hitboxes); i++)
        {
            switch (i)
            {
            case 0: //Head
                hitbox[Hitboxes::HEAD] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                break;
            case 1: //Chest
                hitbox[Hitboxes::UPPER_CHEST] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 1 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::CHEST] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 1 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::LOWER_CHEST] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 1 : config->ragebot[weaponIndex].hitboxes[i];
                break;
            case 2: //Stomach
                hitbox[Hitboxes::STOMACH] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 1 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::PELVIS] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 1 : config->ragebot[weaponIndex].hitboxes[i];
                break;
            case 3: //Arms
                hitbox[Hitboxes::RIGHT_UPPER_ARM] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::RIGHT_FOREARM] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::LEFT_UPPER_ARM] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::LEFT_FOREARM] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                break;
            case 4: //Legs
                hitbox[Hitboxes::RIGHT_CALF] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::RIGHT_THIGH] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::LEFT_CALF] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                hitbox[Hitboxes::LEFT_THIGH] = GetAsyncKeyState(config->ragebot[weaponIndex].bodyAimKey) ? 0 : config->ragebot[weaponIndex].hitboxes[i];
                break;
            default:
                break;
            }
        }
        for (const auto& target : enemies) {
            const auto entity{ interfaces->entityList->getEntity(target.id) };
            Animations::finishSetup(entity);
            for (int i = 0; i < 19; i++)
            {
                if (!(hitbox[i]))
                    continue;

                bool firstScan = true;
                for (auto bonePosition : Multipoint(entity, Animations::data.player[target.id].matrix, i, weaponIndex, firstScan))
                {
                    firstScan = false;
                    if (config->ragebot[weaponIndex].visibleOnly && !entity->isVisible(bonePosition))
                        continue;

                    auto damage = canScan(entity, bonePosition, activeWeapon->getWeaponData(), config->ragebot[weaponIndex].friendlyFire);
                    if (damage <= 0.f)
                        continue;

                    if (config->ragebot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped() && localPlayer->flags() & 1 && !(cmd->buttons & (UserCmd::IN_JUMP))) {
                        if (config->ragebot[weaponIndex].autoScope)
                            cmd->buttons |= UserCmd::IN_ATTACK2;
                        return;
                    }

                    if (localPlayer->flags() & 1 && !(cmd->buttons & (UserCmd::IN_JUMP)))
                        shouldRunAutoStop.at(weaponIndex) = config->ragebot[weaponIndex].autoStop;

                    const auto angle{ Aimbot::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch) };
                    const auto fov{ angle.length2D() };

                    if (bestDamage <= damage) {
                        bestDamage = damage;
                        bestTarget = bonePosition;
                        bestAngle = angle;
                    }
                    if (fov > bestFov)
                    {
                        bestDamage = GetAsyncKeyState(config->ragebot[weaponIndex].overrideDamageKey) ? config->ragebot[weaponIndex].overrideDamageAmount : config->ragebot[weaponIndex].minDamage;
                        bestTarget = Vector{ };
                        bestAngle = Vector{ };
                    }
                }
            }
            if (bestTarget)
            {
                if (!hitChance(localPlayer.get(), entity, activeWeapon, bestAngle, cmd, config->ragebot[weaponIndex].hitChance))
                    bestTarget = Vector{ };
                if (bestTarget)
                {
                    cmd->tickCount = Backtrack::timeToTicks(entity->simulationTime() + Backtrack::getLerp());
                    break;
                }
            }
            auto record = &Backtrack::records[target.id];
            if (record && !record->empty() && record->size() > 3 && Backtrack::valid(record->front().simulationTime) && config->backtrack.enabled)
            {
                auto currentRecord = record->at(record->size() - 2);
                if (!Backtrack::valid(currentRecord.simulationTime))
                    continue;

                Animations::setup(entity, currentRecord);
                for (int i = 0; i < 19; i++)
                {
                    if (!(hitbox[i]))
                        continue;

                    bool firstScan = true;
                    for (auto bonePosition : Multipoint(entity, currentRecord.matrix, i, weaponIndex, firstScan))
                    {
                        firstScan = false;
                        if (config->ragebot[weaponIndex].visibleOnly && !entity->isVisible(bonePosition))
                            continue;

                        auto damage = canScan(entity, bonePosition, activeWeapon->getWeaponData(), config->ragebot[weaponIndex].friendlyFire);
                        if (damage <= 0.f)
                            continue;

                        if (config->ragebot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped() && localPlayer->flags() & 1 && !(cmd->buttons & (UserCmd::IN_JUMP))) {
                            if (config->ragebot[weaponIndex].autoScope)
                                cmd->buttons |= UserCmd::IN_ATTACK2;
                            return;
                        }

                        if (localPlayer->flags() & 1 && !(cmd->buttons & (UserCmd::IN_JUMP)))
                            shouldRunAutoStop.at(weaponIndex) = config->ragebot[weaponIndex].autoStop;

                        const auto angle{ Aimbot::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch) };
                        const auto fov{ angle.length2D() };

                        if (bestDamage <= damage) {
                            bestDamage = damage;
                            bestTarget = bonePosition;
                            bestAngle = angle;
                        }
                        if (fov > bestFov)
                        {
                            bestDamage = GetAsyncKeyState(config->ragebot[weaponIndex].overrideDamageKey) ? config->ragebot[weaponIndex].overrideDamageAmount : config->ragebot[weaponIndex].minDamage;
                            bestTarget = Vector{ };
                            bestAngle = Vector{ };
                        }
                    }
                }
                if (bestTarget)
                {
                    if (!hitChance(localPlayer.get(), entity, activeWeapon, bestAngle, cmd, config->ragebot[weaponIndex].hitChance))
                        bestTarget = Vector{ };
                    if (bestTarget)
                    {
                        cmd->tickCount = Backtrack::timeToTicks(currentRecord.simulationTime + Backtrack::getLerp());
                        Animations::setup(entity, currentRecord);
                        break;
                    }
                }
                Animations::finishSetup(entity);
            }
        }

        if (bestTarget && (config->ragebot[weaponIndex].ignoreSmoke
            || !memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), bestTarget, 1))) {
            static Vector lastAngles{ cmd->viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->commandNumber - 1 && lastAngles && config->ragebot[weaponIndex].silent)
                cmd->viewangles = lastAngles;

            auto angle = Aimbot::calculateRelativeAngle(localPlayer->getEyePosition(), bestTarget, cmd->viewangles + aimPunch);
            bool clamped{ false };

            if (fabs(angle.x) > config->misc.maxAngleDelta || fabs(angle.y) > config->misc.maxAngleDelta) {
                angle.x = std::clamp(angle.x, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);
                angle.y = std::clamp(angle.y, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);
                clamped = true;
            }
            finalAngle = cmd->viewangles + angle;
            cmd->viewangles = finalAngle;
            if (!config->ragebot[weaponIndex].silent)
                interfaces->engine->setViewAngles(cmd->viewangles);

            if (config->ragebot[weaponIndex].autoShot && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && !clamped)
                cmd->buttons |= UserCmd::IN_ATTACK;

            if (clamped)
                cmd->buttons &= ~UserCmd::IN_ATTACK;

            if (clamped) lastAngles = cmd->viewangles;
            else lastAngles = Vector{ };

            lastCommand = cmd->commandNumber;
        }
    }
}