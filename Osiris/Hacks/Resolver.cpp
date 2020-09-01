#include "Aimbot.h"
#include "Animations.h"
#include "Resolver.h"

#include "../SDK/GameEvent.h"
#include "../SDK/FrameStage.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/ModelInfo.h"

#include "../Interfaces.h"


std::array<Resolver::Info, 65> Resolver::player;
std::deque<Resolver::Ticks> Resolver::bulletImpacts;
std::deque<Resolver::Ticks> Resolver::tick;
std::deque<Resolver::Tick> Resolver::shot[65];

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

static Entity* canScan(const Vector& startPosition, const Vector& destination, const WeaponInfo* weaponData, int minDamage) noexcept
{
    if (!localPlayer)
        return nullptr;

    float damage{ static_cast<float>(weaponData->damage) };

    Vector start{ startPosition };
    Vector direction{ destination - start };
    direction /= direction.length();

    int hitsLeft = 4;

    while (damage >= 1.0f && hitsLeft) {
        Trace trace;
        interfaces->engineTrace->traceRay({ start, destination }, 0x4600400B, localPlayer.get(), trace);

        if (trace.fraction == 1.0f)
            break;

        if (trace.entity && trace.entity->isPlayer() && trace.hitgroup > HitGroup::Generic && trace.hitgroup <= HitGroup::RightLeg) {
            damage = HitGroup::getDamageMultiplier(trace.hitgroup) * damage * powf(weaponData->rangeModifier, trace.fraction * weaponData->range / 500.0f);

            if (float armorRatio{ weaponData->armorRatio / 2.0f }; HitGroup::isArmored(trace.hitgroup, trace.entity->hasHelmet()))
                damage -= (trace.entity->armor() < damage * armorRatio / 2.0f ? trace.entity->armor() * 4.0f : damage) * (1.0f - armorRatio);

            if (damage >= minDamage)
                return trace.entity;
        }
        const auto surfaceData = interfaces->physicsSurfaceProps->getSurfaceData(trace.surface.surfaceProps);

        if (surfaceData->penetrationmodifier < 0.1f)
            break;

        damage = handleBulletPenetration(surfaceData, trace, direction, start, weaponData->penetration, damage);
        hitsLeft--;
    }
    return nullptr;
}


int AutoDirection(Entity* entity) noexcept
{
    //TODO: improve this
    constexpr float maxRange{ 8192.0f };

    Vector eye = entity->eyeAngles();
    eye.x = 0.f;
    Vector startPosition{ entity->getAbsOrigin() };
    std::array<float, 4> distance;

    for (int i = 0; i < 4; i++)
    {
        Vector eyeAngle = eye;
        switch (i)
        {
        case 0: // right, front
            eyeAngle.y -= 45.f;
            break;
        case 1: // right, back
            eyeAngle.y -= 135.f;
            break;
        case 2: // left, front
            eyeAngle.y += 45.f;
            break;
        case 3: // left, back
            eyeAngle.y += 135.f;
            break;
        default:
            break;
        }
        eyeAngle.normalize();

        Vector viewAngles{ cos(degreesToRadians(eyeAngle.x)) * cos(degreesToRadians(eyeAngle.y)) * maxRange,
           cos(degreesToRadians(eyeAngle.x)) * sin(degreesToRadians(eyeAngle.y)) * maxRange,
          -sin(degreesToRadians(eyeAngle.x)) * maxRange };

        Trace trace;

        interfaces->engineTrace->traceRay({ startPosition, startPosition + viewAngles }, 0x4600400B, { entity }, trace);

        distance[i] = sqrt(pow(startPosition.x - trace.endpos.x, 2) + pow(startPosition.y - trace.endpos.y, 2) + pow(startPosition.z - trace.endpos.z, 2));
    }

    float minimalDistance = std::min(distance[3], std::min(distance[2], std::min(distance[1], distance[0])));
    if (distance[0] == minimalDistance || distance[1] == minimalDistance)
    {
        return 1;
    }
    return 2;
}

void Resolver::Update(GameEvent* event) noexcept
{
    if (!localPlayer)
        return;

    if (event) 
    {
        Entity* entity = nullptr;
        switch (fnv::hashRuntime(event->getName()))
        {
        case fnv::hash("round_start"):
            //reset on round start
            for (size_t i = 0; i < player.size(); i++)
            {
                player[i].misses = 0;
                player[i].hit = false;
            }
            break;
        case fnv::hash("player_death"):
            //get who died and set misses to 0
            entity = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));
            if (entity)
            {
                player[entity->index()].misses = 0;
                player[entity->index()].hit = false;
            }
            entity = nullptr;
            break;
        case fnv::hash("weapon_fire"):
            //just shot, reset hit status on every entity
            if (event->getInt("userid") != localPlayer.get()->getUserId())
                break;
            for (size_t i = 0; i < player.size(); i++)
                player[i].hit = false;
            break;
        case fnv::hash("player_hurt"):
            if (!config->ragebotExtra.resolver)
                break;
            //if hit avoid doing misses++
            if (event->getInt("attacker") != localPlayer.get()->getUserId() || event->getInt("userid") == localPlayer.get()->getUserId()) //dont count for localplayer
                break;
            entity = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));
            if (entity)
                player[entity->index()].hit = true;
            entity = nullptr;
            break;
        case fnv::hash("bullet_impact"):
            if (!config->ragebotExtra.resolver)
                break;
            //get impact
            if (event->getInt("userid") != localPlayer.get()->getUserId())
                break;
            Ticks bullet{ };
            bullet.position = Vector{ event->getFloat("x"), event->getFloat("y"), event->getFloat("z") };
            bullet.time = memory->globalVars->serverTime();
            bulletImpacts.push_front(bullet);
            break;
        }
    }
}

float Resolver::CalculateFeet(Entity* entity) noexcept
{
    for (size_t i = 0; i < player.size();i++)
    {
        if (player[i].misses > 2)
            player[i].misses = 0;
    }
    PlayerInfo playerInfo;
    interfaces->engine->getPlayerInfo(entity->index(), playerInfo);
    if (playerInfo.fakeplayer || entity->moveType() == MoveType::LADDER || entity->moveType() == MoveType::NOCLIP)
        return entity->getAnimstate()->GoalFeetYaw;

    float eyeAngle = entity->eyeAngles().y;
    float delta = eyeAngle - entity->lby();
    delta = std::isfinite(delta) ? std::remainder(delta, 360.0f) : 0.0f;
    float finalFeetYaw = 0.f;
    float desync = entity->getMaxDesyncAngle();
    int misses = player[entity->index()].misses; //get missed shots here
    int side = 0;
    bool setToEye = false;
    bool invert = false;

    //only works for legit aa, otherwise it isnt p
    //might remove it when the autodirection works better
    if (entity->velocity().length2D() <= 2.f) 
    {
        if (delta > 35.f)
        {
            side = 1;
        }
        else if (delta < -35.f)
        {
            side = 2;
        }
        else
        {
            side = AutoDirection(entity); //freestand
        }
        setToEye = false;
    }
    else
    {
        side = AutoDirection(entity); //freestand
    }

    switch (misses)
    {
    case 0: //use first guess
        invert = false;
        break;
    case 1: //invert side
        invert = true;
        break;
    case 2: //set to eye
        setToEye = true;
    default:
        break;
    }

    if (setToEye)
    {
        finalFeetYaw = eyeAngle;
    }
    else
    {
        if (!invert)
        {
            switch (side)
            {
            case 1:
                finalFeetYaw = eyeAngle - desync;
                break;
            case 2:
                finalFeetYaw = eyeAngle + desync;
                break;
            default:
                finalFeetYaw = entity->getAnimstate()->GoalFeetYaw;
                break;
            }
        }
        else
        {
            switch (side)
            {
            case 1:
                finalFeetYaw = eyeAngle + desync;
                break;
            case 2:
                finalFeetYaw = eyeAngle - desync;
                break;
            default:
                finalFeetYaw = entity->getAnimstate()->GoalFeetYaw;
                break;
            }
        }
    }
    return finalFeetYaw;
}

Resolver::Ticks Resolver::getClosest(const float time) noexcept
{
    //works pretty good right now, no need to change it
    Resolver::Ticks record{ };
    record.position = Vector{};
    record.time = -1.0f;

    int bestTick = -1;
    float bestTime = FLT_MAX;

    for (size_t i = 0; i < tick.size(); i++)
    {
        if (tick.at(i).time > time)
            continue;

        float diff = time - tick.at(i).time;
        if (diff < 0.f)
            continue;

        if (diff < bestTime)
        {
            bestTime = diff;
            bestTick = i;
        }
    }
    if (bestTick == -1)
        return record;
    return tick[bestTick];
}

bool didShoot(UserCmd* cmd) noexcept
{
    if (!(cmd->buttons & (UserCmd::IN_ATTACK)))
        return false;

    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return false;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return false;

    if (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return false;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return false;

    return true;
}

void Resolver::UpdateShots(UserCmd* cmd) noexcept
{
    //trace a ray
    //if didnt hit anything then return (spread)
    //if did hit but no damage then misses++ (resolver)

    if (!localPlayer)
        return;

    if (!localPlayer->isAlive())
    {
        tick.clear();
        bulletImpacts.clear();
        for (auto& record : shot)
            record.clear();
        return;
    }

    if (didShoot(cmd))
    {
        Ticks shoot{ };
        shoot.position = Vector{ localPlayer->getEyePosition() };
        shoot.time = memory->globalVars->serverTime();
        tick.push_front(shoot);
    }
    if (config->ragebotExtra.resolver)
    {
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
        {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity())
                continue;
            Tick record{ };
            record.time = memory->globalVars->serverTime();
            record.origin = entity->origin();
            record.mins = entity->getCollideable()->obbMins();
            record.max = entity->getCollideable()->obbMaxs();
            entity->setupBones(record.matrix, 256, 0x7FF00, memory->globalVars->currenttime);
            shot[i].push_front(record);
            Animations::finishSetup(entity);
        }

    }
    auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    auto weaponData = activeWeapon->getWeaponData();
    if (!activeWeapon->getWeaponData())
        return;

    if (bulletImpacts.empty() || tick.empty() || !config->ragebotExtra.resolver)
        return;

    while (!bulletImpacts.empty())
    {
        auto record = getClosest(bulletImpacts.back().time);
        if (record.time == -1.0f)
        {
            bulletImpacts.pop_back();
            continue;
        }

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
        {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity() || shot[i].empty())
                continue;

            auto bestTick = -1;
            for (size_t j = 0; j < shot[i].size(); j++)
            {
                if (shot[i].at(j).time == record.time)
                {
                    bestTick = j;
                }
            }
            if (bestTick == -1)
                continue;

            Animations::setupResolver(entity, shot[i].at(bestTick));
        }

        Vector start{ record.position };
        if (auto entity = canScan(start, bulletImpacts.back().position, weaponData, 1); entity)
        {
            if (player[entity->index()].hit)
            {
                bulletImpacts.pop_back();
                break;
            }
            player[entity->index()].misses++; //resolver miss
        }
        bulletImpacts.pop_back();
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
        {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity())
                continue;
            Animations::finishSetup(entity);
        }
    }
}