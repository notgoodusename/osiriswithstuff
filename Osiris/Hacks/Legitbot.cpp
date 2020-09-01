#include "Aimbot.h"
#include "Animations.h"
#include "Backtrack.h"
#include "Legitbot.h"

#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"

#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/WeaponData.h"
#include "../SDK/StudioRender.h"
#include "../SDK/ModelInfo.h"

#include <deque>

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

std::vector<Vector> Multipoint(Entity* entity, matrix3x4 matrix[256], StudioHdr* hdr, int iHitbox, int weaponIndex)
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

    if (config->legitbot[weaponIndex].multipoint <= 0)
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

    float multiPoint = (std::min(config->legitbot[weaponIndex].multipoint, 95)) * 0.01f;

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

void Legitbot::run(UserCmd* cmd) noexcept
{
    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex2());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass1(activeWeapon->itemDefinitionIndex2());
    if (!config->legitbot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->legitbot[weaponIndex].enabled)
        weaponIndex = 0;

    if (!config->legitbot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->legitbot[weaponIndex].onKey) {
        if (!config->legitbot[weaponIndex].keyMode) {
            if (!GetAsyncKeyState(config->legitbot[weaponIndex].key))
                return;
        }
        else {
            static bool toggle = true;
            if (GetAsyncKeyState(config->legitbot[weaponIndex].key) & 1)
                toggle = !toggle;
            if (!toggle)
                return;
        }
    }

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    if (config->legitbot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->legitbot[weaponIndex].aimlock)) {

        if (config->legitbot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
            return;

        std::array<bool, 19> hitboxs{ false };

        for (int i = 0; i < ARRAYSIZE(config->legitbot[weaponIndex].hitboxes); i++)
        {
            switch (i)
            {
            case 0: //Head
                hitboxs[Hitboxes::HEAD] = config->legitbot[weaponIndex].hitboxes[i];
                break;
            case 1: //Chest
                hitboxs[Hitboxes::UPPER_CHEST] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::CHEST] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::LOWER_CHEST] = config->legitbot[weaponIndex].hitboxes[i];
                break;
            case 2: //Stomach
                hitboxs[Hitboxes::STOMACH] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::PELVIS] = config->legitbot[weaponIndex].hitboxes[i];
                break;
            case 3: //Arms
                hitboxs[Hitboxes::RIGHT_UPPER_ARM] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::RIGHT_FOREARM] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::LEFT_UPPER_ARM] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::LEFT_FOREARM] = config->legitbot[weaponIndex].hitboxes[i];
                break;
            case 4: //Legs
                hitboxs[Hitboxes::RIGHT_CALF] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::RIGHT_THIGH] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::LEFT_CALF] = config->legitbot[weaponIndex].hitboxes[i];
                hitboxs[Hitboxes::LEFT_THIGH] = config->legitbot[weaponIndex].hitboxes[i];
                break;
            default:
                break;
            }
        }

        auto bestFov = config->legitbot[weaponIndex].fov;
        Vector bestTarget{ };
        auto localPlayerEyePosition = localPlayer->getEyePosition();

        const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) && !config->legitbot[weaponIndex].friendlyFire || entity->gunGameImmunity())
                continue;
            for (int j = 0; j < 19; j++)
            {
                if (!(hitboxs[j]))
                    continue;

                const Model* mod = entity->getModel();
                if (!mod)
                    continue;

                StudioHdr* hdr = interfaces->modelInfo->getStudioModel(mod);

                for (auto bonePosition : Multipoint(entity, Animations::data.player[i].matrix, hdr, j, weaponIndex))
                {
                    if (!entity->isVisible(bonePosition))
                        continue;

                    const auto angle{ Aimbot::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch) };
                    const auto fov{ angle.length2D() };

                    if (fov < bestFov) {
                        bestFov = fov;
                        bestTarget = Aimbot::velocityExtrapolate(entity, bonePosition);
                    }
                }

                if (!config->legitbot[weaponIndex].backtrack)
                    continue;

                auto records = &Backtrack::records[i];
                if (!records || records->empty() || !Backtrack::valid(records->front().simulationTime) || records->size() <= 3 || !config->backtrack.enabled)
                    continue;

                int bestRecord{ };

                for (size_t p = 0; p < Backtrack::records[i].size(); p++) {
                    auto& record = Backtrack::records[i][p];
                    if (!Backtrack::valid(record.simulationTime))
                        continue;

                    auto angle = Aimbot::calculateRelativeAngle(localPlayerEyePosition, record.head, cmd->viewangles + aimPunch);
                    auto fov = std::hypotf(angle.x, angle.y);
                    if (fov < bestFov) {
                        bestFov = fov;
                        bestRecord = p;
                    }
                }

                auto currentRecord = Backtrack::records[i][bestRecord];
                for (auto bonePosition : Multipoint(entity, currentRecord.matrix, currentRecord.hdr, j, weaponIndex))
                {
                    if (!entity->isVisible(bonePosition))
                        continue;

                    const auto angle{ Aimbot::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch) };
                    const auto fov{ angle.length2D() };

                    if (fov < bestFov) {
                        bestFov = fov;
                        bestTarget = Aimbot::velocityExtrapolate(entity, bonePosition);
                    }
                }
            }
        }

        static float lastTime{ 0.0f };
        if (bestTarget && (config->legitbot[weaponIndex].ignoreSmoke || !memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), bestTarget, 1))) {
            static Vector lastAngles{ cmd->viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->commandNumber - 1 && lastAngles && config->legitbot[weaponIndex].silent)
                cmd->viewangles = lastAngles;

            auto angle = Aimbot::calculateRelativeAngle(localPlayer->getEyePosition(), bestTarget, cmd->viewangles + aimPunch);
            bool clamped{ false };

            if (fabs(angle.x) > config->misc.maxAngleDelta || fabs(angle.y) > config->misc.maxAngleDelta) {
                angle.x = std::clamp(angle.x, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);
                angle.y = std::clamp(angle.y, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);
                clamped = true;
            }

            if (memory->globalVars->serverTime() - lastTime >= config->legitbot[weaponIndex].reactionTime / 1000.0f)
            {
                angle /= config->legitbot[weaponIndex].smooth;
                cmd->viewangles += angle;
                if (!config->legitbot[weaponIndex].silent)
                    interfaces->engine->setViewAngles(cmd->viewangles);
            }

            if (clamped)
                cmd->buttons &= ~UserCmd::IN_ATTACK;

            if (clamped || config->legitbot[weaponIndex].smooth > 1.0f) lastAngles = cmd->viewangles;
            else lastAngles = Vector{ };

            lastCommand = cmd->commandNumber;
        }
        if(!bestTarget)
            lastTime = memory->globalVars->serverTime();
    }
}
