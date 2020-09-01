#include <functional>

#include "Chams.h"
#include "../Config.h"
#include "../Hooks.h"
#include "../Interfaces.h"
#include "Backtrack.h"
#include "Animations.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/StudioRender.h"
#include "../SDK/KeyValues.h"
#include "../SDK/RenderView.h"

Chams::Chams() noexcept
{
    normal = interfaces->materialSystem->createMaterial("normal", KeyValues::fromString("VertexLitGeneric", nullptr));
    flat = interfaces->materialSystem->createMaterial("flat", KeyValues::fromString("UnlitGeneric", nullptr));
    chrome = interfaces->materialSystem->createMaterial("chrome", KeyValues::fromString("VertexLitGeneric", "$envmap env_cubemap"));
    glow = interfaces->materialSystem->createMaterial("glow", KeyValues::fromString("VertexLitGeneric", "$additive 1 $envmap models/effects/cube_white $envmapfresnel 1 $alpha .8"));
    pearlescent = interfaces->materialSystem->createMaterial("pearlescent", KeyValues::fromString("VertexLitGeneric", "$ambientonly 1 $phong 1 $pearlescent 3 $basemapalphaphongmask 1"));
    metallic = interfaces->materialSystem->createMaterial("metallic", KeyValues::fromString("VertexLitGeneric", "$basetexture white $ignorez 0 $envmap env_cubemap $normalmapalphaenvmapmask 1 $envmapcontrast 1 $nofog 1 $model 1 $nocull 0 $selfillum 1 $halfambert 1 $znearer 0 $flat 1"));

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$envmap editor/cube_vertigo $envmapcontrast 1 $basetexture dev/zone_warning proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.6 texturescrollangle 90 } }");
        kv->setString("$envmaptint", "[.7 .7 .7]");
        animated = interfaces->materialSystem->createMaterial("animated", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture models/player/ct_fbi/ct_fbi_glass $envmap env_cubemap");
        kv->setString("$envmaptint", "[.4 .6 .7]");
        platinum = interfaces->materialSystem->createMaterial("platinum", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture detail/dt_metal1 $additive 1 $envmap editor/cube_vertigo");
        kv->setString("$color", "[.05 .05 .05]");
        glass = interfaces->materialSystem->createMaterial("glass", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture black $bumpmap effects/flat_normal $translucent 1 $envmap models/effects/crystal_cube_vertigo_hdr $envmapfresnel 0 $phong 1 $phongexponent 16 $phongboost 2");
        kv->setString("$phongtint", "[.2 .35 .6]");
        crystal = interfaces->materialSystem->createMaterial("crystal", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 2 $phongexponent 8");
        kv->setString("$color2", "[.05 .05 .05]");
        kv->setString("$envmaptint", "[.2 .2 .2]");
        kv->setString("$phongfresnelranges", "[.7 .8 1]");
        kv->setString("$phongtint", "[.8 .9 1]");
        silver = interfaces->materialSystem->createMaterial("silver", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 6 $phongexponent 128 $phongdisablehalflambert 1");
        kv->setString("$color2", "[.18 .15 .06]");
        kv->setString("$envmaptint", "[.6 .5 .2]");
        kv->setString("$phongfresnelranges", "[.7 .8 1]");
        kv->setString("$phongtint", "[.6 .5 .2]");
        gold = interfaces->materialSystem->createMaterial("gold", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture black $bumpmap models/inventory_items/trophy_majors/matte_metal_normal $additive 1 $envmap editor/cube_vertigo $envmapfresnel 1 $normalmapalphaenvmapmask 1 $phong 1 $phongboost 20 $phongexponent 3000 $phongdisablehalflambert 1");
        kv->setString("$phongfresnelranges", "[.1 .4 1]");
        kv->setString("$phongtint", "[.8 .9 1]");
        plastic = interfaces->materialSystem->createMaterial("plastic", kv);
    }
}

bool Chams::render(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    appliedChams = false;

    this->ctx = ctx;
    this->state = state;
    this->info = &info;
    this->customBoneToWorld = customBoneToWorld;

    if (std::string_view{ info.model->name }.starts_with("models/weapons/v_")) {
        // info.model->name + 17 -> small optimization, skip "models/weapons/v_"
        if (std::strstr(info.model->name + 17, "sleeve"))
            renderSleeves();
        else if (std::strstr(info.model->name + 17, "arms"))
            renderHands();
        else if (!std::strstr(info.model->name + 17, "tablet")
            && !std::strstr(info.model->name + 17, "parachute")
            && !std::strstr(info.model->name + 17, "fists"))
            renderWeapons();
    }
    else {
        const auto entity = interfaces->entityList->getEntity(info.entityIndex);
        if (entity && !entity->isDormant() && entity->isPlayer())
            renderPlayer(entity);
    }

    return appliedChams;
}

void Chams::renderPlayer(Entity* player) noexcept
{
    if (!localPlayer)
        return;

    const auto health = player->health();

    if (player == localPlayer.get()) {
        if (localPlayer->isScoped())
            interfaces->renderView->setBlend((100.f - config->visuals.thirdpersonTransparency) / 100.f);
        applyChams(config->chams[LOCALPLAYER].materials, false, health);
        if (Animations::data.gotMatrix && config->antiAim.enabled) {
            for (auto& i : Animations::data.fakematrix)
            {
                i[0][3] += info->origin.x;
                i[1][3] += info->origin.y;
                i[2][3] += info->origin.z;
            }
            if (!appliedChams)
                hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
            if (localPlayer->isScoped())
                interfaces->renderView->setBlend((100.f - config->visuals.thirdpersonTransparency) / 100.f);
            applyChams(config->chams[DESYNC].materials, false, health, Animations::data.fakematrix);
            interfaces->studioRender->forcedMaterialOverride(nullptr);
            for (auto& i : Animations::data.fakematrix)
            {
                i[0][3] -= info->origin.x;
                i[1][3] -= info->origin.y;
                i[2][3] -= info->origin.z;
            }
        }
    }
    else if (localPlayer->isOtherEnemy(player)) {
        applyChams(config->chams[ENEMIES_OCCLUDED].materials, true, health);
        applyChams(config->chams[ENEMIES_VISIBLE].materials, false, health);

        if (config->backtrack.enabled) {
            auto record = &Backtrack::records[player->index()];
            if (record && record->size() && Backtrack::valid(record->front().simulationTime)) {
                if (!appliedChams)
                    hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
                applyChams(config->chams[BACKTRACK].materials, false, health, record->back().matrix);
                interfaces->studioRender->forcedMaterialOverride(nullptr);
            }
        }
    }
    else {
        applyChams(config->chams[ALLIES_OCCLUDED].materials, true, health);
        applyChams(config->chams[ALLIES_VISIBLE].materials, false, health);
    }
}

void Chams::renderWeapons() noexcept
{
    if (!localPlayer || !localPlayer->isAlive() || localPlayer->isScoped())
        return;

    applyChams(config->chams[WEAPONS].materials, false, localPlayer->health());
}

void Chams::renderHands() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(config->chams[HANDS].materials, false, localPlayer->health());
}

void Chams::renderSleeves() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(config->chams[SLEEVES].materials, false, localPlayer->health());
}

void Chams::applyChams(const std::vector<Config::Chams::Material>& chams, bool ignorez, int health, matrix3x4* customMatrix) noexcept
{
    for (const auto& cham : chams) {
        if (!cham.enabled || !ignorez)
            continue;

        const auto material = dispatchMaterial(cham.material);
        if (!material)
            continue;

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal) {
            if (cham.healthBased && health) {
                material->findVar("$envmaptint")->setVectorValue(1.0f - health / 100.0f, health / 100.0f, 0.0f);
            }
            else if (cham.rainbow) {
                const auto [r, g, b] { rainbowColor(memory->globalVars->realtime, cham.rainbowSpeed) };
                material->findVar("$envmaptint")->setVectorValue(r, g, b);
            }
            else {
                material->findVar("$envmaptint")->setVectorValue(cham.color[0], cham.color[1], cham.color[2]);
            }
        }
        else {
            if (cham.healthBased && health) {
                material->colorModulate(1.0f - health / 100.0f, health / 100.0f, 0.0f);
            }
            else if (cham.rainbow) {
                const auto [r, g, b] { rainbowColor(memory->globalVars->realtime, cham.rainbowSpeed) };
                material->colorModulate(r, g, b);
            }
            else {
                material->colorModulate(cham.color[0], cham.color[1], cham.color[2]);
            }
        }

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, true);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    }

    for (const auto& cham : chams) {
        if (!cham.enabled || ignorez)
            continue;

        const auto material = dispatchMaterial(cham.material);
        if (!material)
            continue;

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal) {
            if (cham.healthBased && health) {
                material->findVar("$envmaptint")->setVectorValue(1.0f - health / 100.0f, health / 100.0f, 0.0f);
            }
            else if (cham.rainbow) {
                const auto [r, g, b] { rainbowColor(memory->globalVars->realtime, cham.rainbowSpeed) };
                material->findVar("$envmaptint")->setVectorValue(r, g, b);
            }
            else {
                material->findVar("$envmaptint")->setVectorValue(cham.color[0], cham.color[1], cham.color[2]);
            }
        }
        else {
            if (cham.healthBased && health) {
                material->colorModulate(1.0f - health / 100.0f, health / 100.0f, 0.0f);
            }
            else if (cham.rainbow) {
                const auto [r, g, b] { rainbowColor(memory->globalVars->realtime, cham.rainbowSpeed) };
                material->colorModulate(r, g, b);
            }
            else {
                material->colorModulate(cham.color[0], cham.color[1], cham.color[2]);
            }
        }

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, false);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        appliedChams = true;
    }
}