#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "imgui/imgui.h"
#include "nSkinz/config_.hpp"
#include "ConfigStructs.h"

class Config {
public:
    explicit Config(const char*) noexcept;
    void load(size_t) noexcept;
    void save(size_t) const noexcept;
    void add(const char*) noexcept;
    void remove(size_t) noexcept;
    void rename(size_t, const char*) noexcept;
    void reset() noexcept;
    void listConfigs() noexcept;

    constexpr auto& getConfigs() noexcept
    {
        return configs;
    }

    struct Color {
        float color[3]{ 1.0f, 1.0f, 1.0f };
        bool rainbow{ false };
        float rainbowSpeed{ 0.6f };
    };
    
    struct ColorToggle : public Color {
        bool enabled{ false };
    };

    struct Legitbot {
        bool enabled{ false };
        bool onKey{ false };
        int key{ 0 };
        int keyMode{ 0 };
        bool aimlock{ false };
        bool silent{ false };
        bool friendlyFire{ false };
        bool scopedOnly{ true };
        bool ignoreFlash{ false };
        bool ignoreSmoke{ false };
        float fov{ 0.0f };
        float smooth{ 1.0f };
        int reactionTime{ 100 };
        int multipoint{ 0 };
        bool hitboxes[5]{ false };
        bool backtrack{ false };
    };
    std::array<Legitbot, 43> legitbot;

    struct Ragebot {
        bool enabled{ false };
        bool onKey{ false };
        int key{ 0 };
        int keyMode{ 0 };
        bool silent{ false };
        bool friendlyFire{ false };
        bool visibleOnly{ true };
        bool scopedOnly{ true };
        bool ignoreFlash{ false };
        bool ignoreSmoke{ false };
        bool autoShot{ false };
        bool autoScope{ false };
        bool autoStop{ false };
        bool betweenShots{ false };
        bool hitboxes[5]{ false };
        float fov{ 0.0f };
        int priority{ 0 };
        int hitChance{ 0 };
        int multiPoint{ 0 };
        int minDamage{ 1 };
        int overrideDamageKey{ 0 };
        int overrideDamageAmount{ 1 };
        int bodyAimKey{ 0 };
    };
    std::array<Ragebot, 43> ragebot;

    struct {
        bool enabled{ false };
        bool doubletap{ false };
        int doubletapSpeed{ 0 };
        bool resolver{ false };
    } ragebotExtra;

    struct Triggerbot {
        bool enabled = false;
        bool friendlyFire = false;
        bool scopedOnly = true;
        bool ignoreFlash = false;
        bool ignoreSmoke = false;
        bool killshot = false;
        bool onKey = false;
        int key = 0;
        bool hitboxes[5]{ false };
        int hitChance = 50;
        int shotDelay = 0;
        int minDamage = 1;
    };
    std::array<Triggerbot, 43> triggerbot;

    struct {
        bool enabled{ false };
        bool ignoreSmoke{ false };
        bool ignoreFlash{ false };
        int timeLimit{ 200 };
        bool fakeLatency{ false };
        int fakeLatencyAmount{ 200 };
    } backtrack;

    struct {
        bool enabled{ false };
        int mode{ 0 };
        int lbyDelta{ 0 };
        int pitchAngle{ 0 };
        int yawAngle{ 0 };
        int jitterRange{ 10 };
        bool lbySway{ false };
        int invert{ 0 };
        bool autodir{ false };
        bool fakeLag{ false };
        int fakeLagMode{ 0 };
        int fakeLagAmount{ 1 };
        int fakeDuckKey{ 0 };
        bool fakeDucking{ false };
    } antiAim;

    struct Glow : ColorA {
        bool enabled{ false };
        bool healthBased{ false };
        int style{ 0 };
    };
    std::array<Glow, 21> glow;

    struct Chams {
        struct Material : ColorA {
            bool enabled = false;
            bool healthBased = false;
            bool blinking = false;
            bool wireframe = false;
            int material = 0;
        };
        std::vector<Material> materials{ {}, {} };
    };

    std::array<Chams, 19> chams;

    struct Esp {
        struct Shared {
            bool enabled{ false };
            ColorToggle snaplines;
            ColorToggle box;
            int boxType{ 0 };
            ColorToggle name;
            ColorToggle ammo;
            ColorToggle outline{ 0.0f, 0.0f, 0.0f };
            ColorToggle distance;
            float maxDistance{ 0.0f };
        };
       
        struct Player : public Shared {
            ColorToggle eyeTraces;
            ColorToggle health;
            bool healthBar{ false };
            ColorToggle healthBarColor;
            ColorToggle armor;
            ColorToggle armorBar;
            ColorToggle money;
            ColorToggle headDot;
            ColorToggle activeWeapon;
            int hpside{ 0 };
            int armorside{ 0 };
            bool deadesp{ false };
        };

        struct Weapon : public Shared { } weapon;

        struct Projectile : public Shared { };
        std::array<Projectile, 9> projectiles;

        struct DangerZone : public Shared { };
        std::array<DangerZone, 18> dangerZone;

        std::array<Player, 6> players;
    } esp;

    struct {
        bool disablePostProcessing{ false };
        bool inverseRagdollGravity{ false };
        bool noFog{ false };
        bool no3dSky{ false };
        bool noAimPunch{ false };
        bool noViewPunch{ false };
        bool noHands{ false };
        bool noSleeves{ false };
        bool noWeapons{ false };
        bool noSmoke{ false };
        bool noBlur{ false };
        bool noScopeOverlay{ false };
        bool noGrass{ false };
        bool noShadows{ false };
        bool wireframeSmoke{ false };
        bool thirdperson{ false };
        int thirdpersonKey{ 0 };
        int thirdpersonDistance{ 0 };
        int viewmodelFov{ 0 };
        int fov{ 0 };
        int fovScoped{ 0 };
        int farZ{ 0 };
        int flashReduction{ 0 };
        float brightness{ 0.0f };
        int skybox{ 0 };
        bool nightMode{ false };
        ColorToggle nightModeOverride;
        bool deagleSpinner{ false };
        int screenEffect{ 0 };
        int hitEffect{ 0 };
        float hitEffectTime{ 0.6f };
        bool hitMarker{ false };
        int playerModelT{ 0 };
        int playerModelCT{ 0 };

        struct {
            bool enabled = false;
            float blue = 0.0f;
            float red = 0.0f;
            float mono = 0.0f;
            float saturation = 0.0f;
            float ghost = 0.0f;
            float green = 0.0f;
            float yellow = 0.0f;
        } colorCorrection;

        int asusWalls{ 0 };
        int asusProps{ 0 };
        int thirdpersonTransparency{ 0 };
        ColorToggle bulletTracersEnemy;
        ColorToggle bulletTracersAllies;
        ColorToggle bulletTracersLocal;
        bool aaLines{ false };
        bool PenetrationCrosshair{ false };
    } visuals;

    std::array<item_setting, 36> skinChanger;

    struct {
        int chickenVolume{ 100 };

        struct Player {
            int masterVolume{ 100 };
            int headshotVolume{ 100 };
            int weaponVolume{ 100 };
            int footstepVolume{ 100 };
        };

        std::array<Player, 3> players;
    } sound;

    struct {
        int menuStyle{ 0 };
        int menuColors{ 0 };
    } style;

    struct {
        int menuKey{ 0x2D }; // VK_INSERT
        bool debugwindow{ false };
        bool antiAfkKick{ false };
        bool autoStrafe{ false };
        bool bunnyHop{ false };
        bool clanTag{ false };
        bool fastDuck{ false };
        bool moonwalk{ false };
        bool edgejump{ false };
        int edgejumpkey{ 0 };
        bool quickpeek{ false };
        int quickpeekkey{ 0 };
        bool slowwalk{ false };
        bool fastStop{ false };
        int slowwalkKey{ 0 };
        bool sniperCrosshair{ false };
        bool recoilCrosshair{ false };
        bool autoPistol{ false };
        bool autoAccept{ false };
        bool radarHack{ false };
        bool revealRanks{ false };
        bool revealMoney{ false };
        bool revealSuspect{ false };
        bool spectatorList{ false };
        bool watermark{ false };
        bool disableModelOcclusion{ false };
        float aspectratio{ 0 };
        bool killSay{ false };
        bool chickenDeathSay{ false };
        bool nameStealer{ false };
        bool disablePanoramablur{ false };
        int banColor{ 6 };
        std::string banText{ "Cheater has been permanently banned from official CS:GO servers." };
        bool fastPlant{ false };
        ColorToggle bombTimer{ 1.0f, 0.55f, 0.0f };
        bool prepareRevolver{ false };
        int prepareRevolverKey{ 0 };
        int hitSound{ 0 };
        bool nadePredict{ false };
        bool nadeTrajectory{ false };
        bool showImpacts{ false };
        float maxAngleDelta{ 255.0f };
        bool fakePrime{ false };
        int killSound{ 0 };
        std::string customKillSound;
        std::string customHitSound;
        PurchaseList purchaseList;
        bool svpurebypass{ false };
        bool unlockInventory{ false };
        AutoBuy autoBuy;
        bool preserveDeathNotices{ false };
        bool autoDisconnect{ false };
        bool oppositeHandKnife{ false };
    } misc;
private:
    std::filesystem::path path;
    std::vector<std::string> configs;
};

inline std::unique_ptr<Config> config;
