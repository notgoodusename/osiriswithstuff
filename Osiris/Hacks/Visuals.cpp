#include "../fnv.h"
#include "Visuals.h"

#include "Animations.h"
#include "Resolver.h"
#include "Ragebot.h"

#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/NetworkStringTable.h"
#include "../SDK/RenderContext.h"
#include "../SDK/Surface.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/Beams.h"
#include "../Hooks.h"

#include <array>

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

struct Color {
    std::array<float, 4> color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

void Visuals::playerModel(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static int originalIdx = 0;

    if (!localPlayer) {
        originalIdx = 0;
        return;
    }

    constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
        constexpr std::array models{
        "models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
        "models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
        "models/player/custom_player/legacy/ctm_sas_variantf.mdl",
        "models/player/custom_player/legacy/ctm_st6_variante.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantg.mdl",
        "models/player/custom_player/legacy/ctm_st6_varianti.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantk.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantm.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantf.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantg.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianth.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianti.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantj.mdl",
        "models/player/custom_player/legacy/tm_leet_variantf.mdl",
        "models/player/custom_player/legacy/tm_leet_variantg.mdl",
        "models/player/custom_player/legacy/tm_leet_varianth.mdl",
        "models/player/custom_player/legacy/tm_leet_varianti.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
        "models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
        
        "models/player/custom_player/legacy/tm_pirate.mdl",
        "models/player/custom_player/legacy/tm_pirate_varianta.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantb.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantc.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantd.mdl",
        "models/player/custom_player/legacy/tm_anarchist.mdl",
        "models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantd.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianta.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantb.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantc.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantd.mdl",
        "models/player/custom_player/legacy/tm_balkan_variante.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl"
        };

        switch (team) {
        case 2: return static_cast<std::size_t>(config->visuals.playerModelT - 1) < models.size() ? models[config->visuals.playerModelT - 1] : nullptr;
        case 3: return static_cast<std::size_t>(config->visuals.playerModelCT - 1) < models.size() ? models[config->visuals.playerModelCT - 1] : nullptr;
        default: return nullptr;
        }
    };

    if (const auto model = getModel(localPlayer->team())) {
        if (stage == FrameStage::RENDER_START) {
            originalIdx = localPlayer->modelIndex();
            if (const auto modelprecache = interfaces->networkStringTableContainer->findTable("modelprecache")) {
                modelprecache->addString(false, model);
                const auto viewmodelArmConfig = memory->getPlayerViewmodelArmConfigForPlayerModel(model);
                modelprecache->addString(false, viewmodelArmConfig[2]);
                modelprecache->addString(false, viewmodelArmConfig[3]);
            }
        }

        const auto idx = stage == FrameStage::RENDER_END && originalIdx ? originalIdx : interfaces->modelInfo->getModelIndex(model);

        localPlayer->setModelIndex(idx);

        if (const auto ragdoll = interfaces->entityList->getEntityFromHandle(localPlayer->ragdoll()))
            ragdoll->setModelIndex(idx);
    }
}

void Visuals::nightMode() noexcept
{
    static auto drawSpecificStaticProp = interfaces->cvar->findVar("r_drawspecificstaticprop");
    if (drawSpecificStaticProp->getInt() != config->visuals.nightMode)
        drawSpecificStaticProp->setValue(config->visuals.nightMode);

    static Color null = { 0.0f, 0.0f, 0.0f, 0.0f };

    static Color 
        lastWorldColor = null,
        lastPropColor = null,
        lastSkyboxColor = null;
    static Color 
        worldColor = null,
        propColor = null,
        skyboxColor = null;
    static float r = .06f, g = .06f, b = .06f;

    if (config->visuals.nightModeOverride.enabled) {
        r = config->visuals.nightModeOverride.color[0];
        g = config->visuals.nightModeOverride.color[1];
        b = config->visuals.nightModeOverride.color[2];
    }
    else {
        r = .06f, g = .06f, b = .06f;
    }

    if (config->visuals.nightMode) {
        worldColor = { r, g, b, 1.0f };
        propColor = { -((r - 1.f) * (r - 1.f)) * 0.8f + 1.f, -((g - 1.f) * (g - 1.f)) * 0.8f + 1.f, -((b - 1.f) * (b - 1.f)) * 0.8f + 1.f, 1.0f };
        skyboxColor = { -((r - 1.f) * (r - 1.f)) * 0.8f + 1.f, -((g - 1.f) * (g - 1.f)) * 0.8f + 1.f, -((b - 1.f) * (b - 1.f)) * 0.8f + 1.f, 1.0f };
    }
    else {
        worldColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        propColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        skyboxColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    worldColor.color[3] = config->visuals.asusWalls;
    propColor.color[3] = config->visuals.asusProps;

    bool didColorChange = (worldColor.color != lastWorldColor.color) ||
        (propColor.color != lastPropColor.color) ||
        (skyboxColor.color != lastSkyboxColor.color);

    static bool wasInGame = false;
    if (!interfaces->engine->isInGame())
        wasInGame = false;
    if (!wasInGame && interfaces->engine->isInGame())
    {
        didColorChange = true;
        wasInGame = true;
    }

    if (!didColorChange)
        return;

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat)
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (textureGroup.starts_with("World")) {
            if (config->visuals.nightModeOverride.rainbow && config->visuals.nightModeOverride.enabled)
                mat->colorModulate(rainbowColor(memory->globalVars->realtime, config->visuals.nightModeOverride.rainbowSpeed));
            else
                mat->colorModulate(worldColor.color[0], worldColor.color[1], worldColor.color[2]);
            mat->alphaModulate((100.f - worldColor.color[3]) / 100.f);
            lastWorldColor = worldColor;
        }
        else if (textureGroup.starts_with("StaticProp") || textureGroup.starts_with("Prop")) {
            if (config->visuals.nightModeOverride.rainbow && config->visuals.nightModeOverride.enabled)
                mat->colorModulate(rainbowColor(memory->globalVars->realtime, config->visuals.nightModeOverride.rainbowSpeed));
            else
                mat->colorModulate(propColor.color[0], propColor.color[1], propColor.color[2]);
            mat->alphaModulate((100.f - propColor.color[3]) / 100.f);
            lastPropColor = propColor;
        }
        else if (textureGroup.starts_with("SkyBox"))
        {
            mat->colorModulate(skyboxColor.color[0], skyboxColor.color[1], skyboxColor.color[2]);
            lastSkyboxColor = skyboxColor;
        }
    }
}

void Visuals::modifySmoke(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noSmoke);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == FrameStage::RENDER_START && config->visuals.wireframeSmoke);
    }
}

void Visuals::thirdperson() noexcept
{
    static bool isInThirdperson{ true };
    static float lastTime{ 0.0f };

    if (GetAsyncKeyState(config->visuals.thirdpersonKey) && memory->globalVars->realtime - lastTime > 0.5f) {
        isInThirdperson = !isInThirdperson;
        lastTime = memory->globalVars->realtime;
    }

    if (config->visuals.thirdperson)
        if (memory->input->isCameraInThirdPerson = (!config->visuals.thirdpersonKey || isInThirdperson)
            && localPlayer && localPlayer->isAlive())
            memory->input->cameraOffset.z = static_cast<float>(config->visuals.thirdpersonDistance);
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (config->visuals.noAimPunch && !config->misc.recoilCrosshair)
            localPlayer->aimPunchAngle() = Vector{ };

        if (config->visuals.noViewPunch)
            localPlayer->viewPunchAngle() = Vector{ };

    } else if (stage == FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeBlur(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noBlur);
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(config->visuals.brightness);
}

void Visuals::removeGrass(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr auto getGrassMaterialName = []() constexpr noexcept -> const char* {
        switch (fnv::hashRuntime(interfaces->engine->getLevelName())) {
        case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
        case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
        case fnv::hash("dz_junglety"): return "detail/tropical_grass";
        default: return nullptr;
        }
    };

    if (const auto grassMaterialName = getGrassMaterialName())
        interfaces->materialSystem->findMaterial(grassMaterialName)->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noGrass);
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!config->visuals.no3dSky);
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!config->visuals.noShadows);
}

#define DRAW_SCREEN_EFFECT(material) \
{ \
    const auto drawFunction = memory->drawScreenEffectMaterial; \
    int w, h; \
    interfaces->surface->getScreenSize(w, h); \
    __asm { \
        __asm push h \
        __asm push w \
        __asm push 0 \
        __asm xor edx, edx \
        __asm mov ecx, material \
        __asm call drawFunction \
        __asm add esp, 12 \
    } \
}

void Visuals::applyScreenEffects() noexcept
{
    if (!config->visuals.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects{
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (config->visuals.screenEffect <= 2 || static_cast<std::size_t>(config->visuals.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[config->visuals.screenEffect - 2];
    }());

    if (config->visuals.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (config->visuals.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (config->visuals.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept
{
    if (config->visuals.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + config->visuals.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[]{
                "effects/dronecam",
                "effects/underwater_overlay",
                "effects/healthboost",
                "effects/dangerzone_screen"
                };

                if (config->visuals.hitEffect <= 2)
                    return effects[0];
                return effects[config->visuals.hitEffect - 2];
            };

           
            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (config->visuals.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (config->visuals.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (config->visuals.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

void Visuals::hitMarker(GameEvent* event) noexcept
{
    if (config->visuals.hitMarker == 0 || !localPlayer)
        return;

    static float alpha = 0.f;

    if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
        alpha = 255.f;
        return;
    }

    if (alpha < 1.f)
        return;

    switch (config->visuals.hitMarker) {
    case 1:
        const auto [width, height] = interfaces->surface->getScreenSize();

        const auto width_mid = width / 2;
        const auto height_mid = height / 2;

        interfaces->surface->setDrawColor(255, 255, 255, alpha);
        interfaces->surface->drawLine(width_mid + 10, height_mid + 10, width_mid + 4, height_mid + 4);
        interfaces->surface->drawLine(width_mid - 10, height_mid + 10, width_mid - 4, height_mid + 4);
        interfaces->surface->drawLine(width_mid + 10, height_mid - 10, width_mid + 4, height_mid - 4);
        interfaces->surface->drawLine(width_mid - 10, height_mid - 10, width_mid - 4, height_mid - 4);
        alpha *= .95f;
        break;
    }
}

void Visuals::disablePostProcessing(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == FrameStage::RENDER_START && config->visuals.disablePostProcessing;
}

void Visuals::reduceFlashEffect() noexcept
{
    if (localPlayer)
        localPlayer->flashMaxAlpha() = 255.0f - config->visuals.flashReduction * 2.55f;
}

bool Visuals::removeHands(const char* modelName) noexcept
{
    return config->visuals.noHands && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept
{
    return config->visuals.noSleeves && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept
{
    return config->visuals.noWeapons && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::skybox(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array skyboxes{ "cs_baggage_skybox_", "cs_tibet", "embassy", "italy", "jungle", "nukeblank", "office", "sky_cs15_daylight01_hdr", "sky_cs15_daylight02_hdr", "sky_cs15_daylight03_hdr", "sky_cs15_daylight04_hdr", "sky_csgo_cloudy01", "sky_csgo_night_flat", "sky_csgo_night02", "sky_day02_05_hdr", "sky_day02_05", "sky_dust", "sky_l4d_rural02_ldr", "sky_venice", "vertigo_hdr", "vertigo", "vertigoblue_hdr", "vietnam" };

    if (stage == FrameStage::RENDER_START && static_cast<std::size_t>(config->visuals.skybox - 1) < skyboxes.size()) {
        memory->loadSky(skyboxes[config->visuals.skybox - 1]);
    }
    else {
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}

void Visuals::bulletBeams(GameEvent* event) noexcept
{
    if (!interfaces->engine->isInGame() || !interfaces->engine->isConnected() || memory->renderBeams == nullptr)
        return;

    const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));

    if (!player || !localPlayer || player->isDormant() || !player->isAlive())
        return;

    if (player == localPlayer.get() && !config->visuals.bulletTracersLocal.enabled) 
        return;

    if (localPlayer->isOtherEnemy(player) && !config->visuals.bulletTracersEnemy.enabled)
        return;

    if (!localPlayer->isOtherEnemy(player) && player != localPlayer.get() && !config->visuals.bulletTracersAllies.enabled)
        return;

    Vector position;
    position.x = event->getFloat("x");
    position.y = event->getFloat("y");
    position.z = event->getFloat("z");

    BeamInfo_t beam_info;
    beam_info.m_nType = TE_BEAMPOINTS;
    beam_info.m_pszModelName = "sprites/physbeam.vmt";
    beam_info.m_nModelIndex = -1;
    beam_info.m_flHaloScale = 0.f;
    beam_info.m_flLife = 4.f;
    beam_info.m_flWidth = 2.f;
    beam_info.m_flEndWidth = 2.f;
    beam_info.m_flFadeLength = 0.1f;
    beam_info.m_flAmplitude = 2.f;
    beam_info.m_flBrightness = 255.f;
    beam_info.m_flSpeed = 0.2f;
    beam_info.m_nStartFrame = 0;
    beam_info.m_flFrameRate = 0.f;
    if (player == localPlayer.get()) {
        beam_info.m_flRed = config->visuals.bulletTracersLocal.color[0] * 255;
        beam_info.m_flGreen = config->visuals.bulletTracersLocal.color[1] * 255;
        beam_info.m_flBlue = config->visuals.bulletTracersLocal.color[2] * 255;
    }
    else if (localPlayer->isOtherEnemy(player)) {
        beam_info.m_flRed = config->visuals.bulletTracersEnemy.color[0] * 255;
        beam_info.m_flGreen = config->visuals.bulletTracersEnemy.color[1] * 255;
        beam_info.m_flBlue = config->visuals.bulletTracersEnemy.color[2] * 255;
    }
    else {
        beam_info.m_flRed = config->visuals.bulletTracersAllies.color[0] * 255;
        beam_info.m_flGreen = config->visuals.bulletTracersAllies.color[1] * 255;
        beam_info.m_flBlue = config->visuals.bulletTracersAllies.color[2] * 255;
    }
    beam_info.m_nSegments = 2;
    beam_info.m_bRenderable = true;
    beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

    static Vector pos{ };
    if (auto record = Resolver::getClosest(memory->globalVars->serverTime()); record.time != 1.0f)
        pos = record.position;

    beam_info.m_vecStart = pos;
    beam_info.m_vecEnd = position;

    auto beam = memory->renderBeams->CreateBeamPoints(beam_info);
    if (beam)
        memory->renderBeams->DrawBeam(beam);
}

void Visuals::aaLines() noexcept
{
    if (!config->visuals.aaLines)
        return;

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

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (!interfaces->engine->isInGame() || !interfaces->engine->isConnected())
        return;

    if (!memory->input->isCameraInThirdPerson)
        return;


    float lbyAngle = localPlayer->lby();
    float realAngle = localPlayer.get()->getAnimstate()->GoalFeetYaw;
    float fakeAngle = Animations::data.fakeAngle;
    float absAngle = localPlayer->getAbsAngle().y;

    Vector src3D, dst3D, forward, src, dst;

    //
    // LBY
    //

    AngleVectors(Vector{ 0, lbyAngle, 0 }, &forward);
    src3D = localPlayer->getAbsOrigin();
    dst3D = src3D + (forward * 40.f);

    if (!worldToScreen(src3D, src) || !worldToScreen(dst3D, dst))
        return;

    interfaces->surface->setDrawColor( 0, 255, 0 );
    interfaces->surface->drawLine(src.x, src.y, dst.x, dst.y);

    const auto [widthLby, heightLby] { interfaces->surface->getTextSize(hooks->smallFonts, L"LBY") };
    interfaces->surface->setTextColor(0, 255, 0);
    interfaces->surface->setTextFont(hooks->smallFonts);
    interfaces->surface->setTextPosition(dst.x - widthLby / 2, dst.y - heightLby / 2);
    interfaces->surface->printText(L"LBY");

    //
    // Fake
    //

    AngleVectors(Vector{ 0, fakeAngle, 0 }, &forward);
    dst3D = src3D + (forward * 40.f);


    if (!worldToScreen(src3D, src) || !worldToScreen(dst3D, dst))
        return;

    interfaces->surface->setDrawColor(0, 0, 255);
    interfaces->surface->drawLine(src.x, src.y, dst.x, dst.y);

    const auto [widthFake, heightFake] { interfaces->surface->getTextSize(hooks->smallFonts, L"FAKE") };
    interfaces->surface->setTextColor(0, 0, 255);
    interfaces->surface->setTextFont(hooks->smallFonts);
    interfaces->surface->setTextPosition(dst.x - widthFake / 2, dst.y - heightFake / 2);
    interfaces->surface->printText(L"FAKE");

    //
    // Abs
    //

    AngleVectors(Vector{ 0, absAngle, 0 }, &forward);
    dst3D = src3D + (forward * 40.f);

    if (!worldToScreen(src3D, src) || !worldToScreen(dst3D, dst))
        return;

    interfaces->surface->setDrawColor(255, 0, 0);
    interfaces->surface->drawLine(src.x, src.y, dst.x, dst.y);

    const auto [widthAbs, heightAbs] { interfaces->surface->getTextSize(hooks->smallFonts, L"ABS") };
    interfaces->surface->setTextColor(255, 0, 0);
    interfaces->surface->setTextFont(hooks->smallFonts);
    interfaces->surface->setTextPosition(dst.x - widthAbs / 2, dst.y - heightAbs / 2);
    interfaces->surface->printText(L"ABS");
}

/////////////////////////////////////
void PenetrationCrosshair() noexcept
{
    if (!config->visuals.PenetrationCrosshair)
        return;

}