#include <fstream>
#include <ShlObj.h>

#include "json/json.h"

#include "Config.h"

Config::Config(const char* name) noexcept
{
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        path /= name;
        CoTaskMemFree(pathToDocuments);
    }

    listConfigs();
}

void Config::load(size_t id) noexcept
{
    Json::Value json;

    if (std::ifstream in{ path / (const char8_t*)configs[id].c_str() }; in.good())
        in >> json;
    else
        return;

    for (size_t i = 0; i < legitbot.size(); i++) {
        const auto& legitbotJson = json["Legitbot"][i];
        auto& legitbotConfig = legitbot[i];

        if (legitbotJson.isMember("Enabled")) legitbotConfig.enabled = legitbotJson["Enabled"].asBool();
        if (legitbotJson.isMember("On key")) legitbotConfig.onKey = legitbotJson["On key"].asBool();
        if (legitbotJson.isMember("Key")) legitbotConfig.key = legitbotJson["Key"].asInt();
        if (legitbotJson.isMember("Key mode")) legitbotConfig.keyMode = legitbotJson["Key mode"].asInt();
        if (legitbotJson.isMember("Aimlock")) legitbotConfig.aimlock = legitbotJson["Aimlock"].asBool();
        if (legitbotJson.isMember("Silent")) legitbotConfig.silent = legitbotJson["Silent"].asBool();
        if (legitbotJson.isMember("Friendly fire")) legitbotConfig.friendlyFire = legitbotJson["Friendly fire"].asBool();
        if (legitbotJson.isMember("Scoped only")) legitbotConfig.scopedOnly = legitbotJson["Scoped only"].asBool();
        if (legitbotJson.isMember("Ignore flash")) legitbotConfig.ignoreFlash = legitbotJson["Ignore flash"].asBool();
        if (legitbotJson.isMember("Ignore smoke")) legitbotConfig.ignoreSmoke = legitbotJson["Ignore smoke"].asBool();
        if (legitbotJson.isMember("Fov")) legitbotConfig.fov = legitbotJson["Fov"].asFloat();
        if (legitbotJson.isMember("Smooth")) legitbotConfig.smooth = legitbotJson["Smooth"].asFloat();
        if (legitbotJson.isMember("Reaction Time")) legitbotConfig.reactionTime = legitbotJson["Reaction Time"].asInt();
        if (legitbotJson.isMember("Multipoint")) legitbotConfig.multipoint = legitbotJson["Multipoint"].asInt();
        if (legitbotJson.isMember("BacktrackLegitbot")) legitbotConfig.backtrack = legitbotJson["BacktrackLegitbot"].asBool();

        if (legitbotJson.isMember("Hitboxes")) {
            const auto& hitboxesJson = legitbotJson["Hitboxes"];
            auto& hitboxesConfig = legitbotConfig;

            hitboxesConfig.hitboxes[0] = hitboxesJson["Hitboxes"][0].asBool();
            hitboxesConfig.hitboxes[1] = hitboxesJson["Hitboxes"][1].asBool();
            hitboxesConfig.hitboxes[2] = hitboxesJson["Hitboxes"][2].asBool();
            hitboxesConfig.hitboxes[3] = hitboxesJson["Hitboxes"][3].asBool();
            hitboxesConfig.hitboxes[4] = hitboxesJson["Hitboxes"][4].asBool();
        }
    }
    for (size_t i = 0; i < ragebot.size(); i++)
    {
        const auto& ragebotJson = json["Ragebot"][i];
        auto& ragebotConfig = ragebot[i];

        if (ragebotJson.isMember("Enabled")) ragebotConfig.enabled = ragebotJson["Enabled"].asBool();
        if (ragebotJson.isMember("On key")) ragebotConfig.onKey = ragebotJson["On key"].asBool();
        if (ragebotJson.isMember("Key")) ragebotConfig.key = ragebotJson["Key"].asInt();
        if (ragebotJson.isMember("Key mode")) ragebotConfig.keyMode = ragebotJson["Key mode"].asInt();
        if (ragebotJson.isMember("Silent")) ragebotConfig.silent = ragebotJson["Silent"].asBool();
        if (ragebotJson.isMember("Friendly fire")) ragebotConfig.friendlyFire = ragebotJson["Friendly fire"].asBool();
        if (ragebotJson.isMember("Visible only")) ragebotConfig.visibleOnly = ragebotJson["Visible only"].asBool();
        if (ragebotJson.isMember("Scoped only")) ragebotConfig.scopedOnly = ragebotJson["Scoped only"].asBool();
        if (ragebotJson.isMember("Ignore flash")) ragebotConfig.ignoreFlash = ragebotJson["Ignore flash"].asBool();
        if (ragebotJson.isMember("Ignore smoke")) ragebotConfig.ignoreSmoke = ragebotJson["Ignore smoke"].asBool();
        if (ragebotJson.isMember("Auto shot")) ragebotConfig.autoShot = ragebotJson["Auto shot"].asBool();
        if (ragebotJson.isMember("Auto scope")) ragebotConfig.autoScope = ragebotJson["Auto scope"].asBool();
        if (ragebotJson.isMember("Auto stop")) ragebotConfig.autoStop = ragebotJson["Auto stop"].asBool();
        if (ragebotJson.isMember("Between shots")) ragebotConfig.betweenShots = ragebotJson["Between shots"].asBool();
        if (ragebotJson.isMember("Fov")) ragebotConfig.fov = ragebotJson["Fov"].asFloat();
        if (ragebotJson.isMember("Hitchance")) ragebotConfig.hitChance = ragebotJson["Hitchance"].asInt();
        if (ragebotJson.isMember("Multipoint")) ragebotConfig.multiPoint = ragebotJson["Multipoint"].asInt();
        if (ragebotJson.isMember("Priority")) ragebotConfig.priority = ragebotJson["Priority"].asInt();
        if (ragebotJson.isMember("Min damage")) ragebotConfig.minDamage = ragebotJson["Min damage"].asInt();
        if (ragebotJson.isMember("Min damage override key")) ragebotConfig.overrideDamageKey = ragebotJson["Min damage override key"].asInt();
        if (ragebotJson.isMember("Min damage override amount")) ragebotConfig.overrideDamageAmount = ragebotJson["Min damage override amount"].asInt();
        if (ragebotJson.isMember("Body aim key")) ragebotConfig.bodyAimKey = ragebotJson["Body aim key"].asInt();

        if (ragebotJson.isMember("Hitboxes")) {
            const auto& hitboxesJson = ragebotJson["Hitboxes"];
            auto& hitboxesConfig = ragebotConfig;

            hitboxesConfig.hitboxes[0] = hitboxesJson["Hitboxes"][0].asBool();
            hitboxesConfig.hitboxes[1] = hitboxesJson["Hitboxes"][1].asBool();
            hitboxesConfig.hitboxes[2] = hitboxesJson["Hitboxes"][2].asBool();
            hitboxesConfig.hitboxes[3] = hitboxesJson["Hitboxes"][3].asBool();
            hitboxesConfig.hitboxes[4] = hitboxesJson["Hitboxes"][4].asBool();
        }
    }

    for (size_t i = 0; i < triggerbot.size(); i++) {
        const auto& triggerbotJson = json["Triggerbot"][i];
        auto& triggerbotConfig = triggerbot[i];

        if (triggerbotJson.isMember("Enabled")) triggerbotConfig.enabled = triggerbotJson["Enabled"].asBool();
        if (triggerbotJson.isMember("On key")) triggerbotConfig.onKey = triggerbotJson["On key"].asBool();
        if (triggerbotJson.isMember("Key")) triggerbotConfig.key = triggerbotJson["Key"].asInt();
        if (triggerbotJson.isMember("Friendly fire")) triggerbotConfig.friendlyFire = triggerbotJson["Friendly fire"].asBool();
        if (triggerbotJson.isMember("Scoped only")) triggerbotConfig.scopedOnly = triggerbotJson["Scoped only"].asBool();
        if (triggerbotJson.isMember("Ignore flash")) triggerbotConfig.ignoreFlash = triggerbotJson["Ignore flash"].asBool();
        if (triggerbotJson.isMember("Ignore smoke")) triggerbotConfig.ignoreSmoke = triggerbotJson["Ignore smoke"].asBool();
        if (triggerbotJson.isMember("Hitboxes")) {
            const auto& hitboxesJson = triggerbotJson["Hitboxes"];
            auto& hitboxesConfig = triggerbotConfig;

            hitboxesConfig.hitboxes[0] = hitboxesJson["Hitboxes"][0].asBool();
            hitboxesConfig.hitboxes[1] = hitboxesJson["Hitboxes"][1].asBool();
            hitboxesConfig.hitboxes[2] = hitboxesJson["Hitboxes"][2].asBool();
            hitboxesConfig.hitboxes[3] = hitboxesJson["Hitboxes"][3].asBool();
            hitboxesConfig.hitboxes[4] = hitboxesJson["Hitboxes"][4].asBool();
        }
        if (triggerbotJson.isMember("Hitchance")) triggerbotConfig.hitChance = triggerbotJson["Hitchance"].asInt();
        if (triggerbotJson.isMember("Shot delay")) triggerbotConfig.shotDelay = triggerbotJson["Shot delay"].asInt();
        if (triggerbotJson.isMember("Min damage")) triggerbotConfig.minDamage = triggerbotJson["Min damage"].asInt();
        if (triggerbotJson.isMember("Killshot")) triggerbotConfig.killshot = triggerbotJson["Killshot"].asBool();
    }

    {
        const auto& backtrackJson = json["Backtrack"];
        if (backtrackJson.isMember("Enabled")) backtrack.enabled = backtrackJson["Enabled"].asBool();
        if (backtrackJson.isMember("Ignore smoke")) backtrack.ignoreSmoke = backtrackJson["Ignore smoke"].asBool();
        if (backtrackJson.isMember("Ignore flash")) backtrack.ignoreFlash = backtrackJson["Ignore flash"].asBool();
        if (backtrackJson.isMember("Time limit")) backtrack.timeLimit = backtrackJson["Time limit"].asInt();
        if (backtrackJson.isMember("Fake Latency")) backtrack.fakeLatency = backtrackJson["Fake Latency"].asBool();
        if (backtrackJson.isMember("Latency Amount")) backtrack.fakeLatencyAmount = backtrackJson["Latency Amount"].asInt();
    }

    {
        const auto& antiAimJson = json["Anti aim"];
        if (antiAimJson.isMember("Enabled")) antiAim.enabled = antiAimJson["Enabled"].asBool();
        if (antiAimJson.isMember("Mode")) antiAim.mode = antiAimJson["Mode"].asInt();
        if (antiAimJson.isMember("LBY delta")) antiAim.lbyDelta = antiAimJson["LBY delta"].asInt();
        if (antiAimJson.isMember("Pitch angle")) antiAim.pitchAngle = antiAimJson["Pitch angle"].asInt();
        if (antiAimJson.isMember("Yaw angle")) antiAim.yawAngle = antiAimJson["Yaw angle"].asInt();
        if (antiAimJson.isMember("Jitter range")) antiAim.jitterRange = antiAimJson["Jitter range"].asInt();
        if (antiAimJson.isMember("LBY sway")) antiAim.lbySway = antiAimJson["LBY sway"].asBool();
        if (antiAimJson.isMember("Invert")) antiAim.invert = antiAimJson["Invert"].asInt();
        if (antiAimJson.isMember("Auto direction")) antiAim.autodir = antiAimJson["Auto direction"].asBool();
        if (antiAimJson.isMember("Fake Lag")) antiAim.fakeLag = antiAimJson["Fake Lag"].asBool();
        if (antiAimJson.isMember("Fake Lag Mode")) antiAim.fakeLagMode = antiAimJson["Fake Lag Mode"].asInt();
        if (antiAimJson.isMember("Fake Lag Amount")) antiAim.fakeLagAmount = antiAimJson["Fake Lag Amount"].asInt();
        if (antiAimJson.isMember("Fake duck key")) antiAim.fakeDuckKey = antiAimJson["Fake duck key"].asInt();

    }

    {
        const auto& ragebotExtraJson = json["Ragebot extra"];
        if (ragebotExtraJson.isMember("Enabled")) ragebotExtra.enabled = ragebotExtraJson["Enabled"].asBool();
        if (ragebotExtraJson.isMember("Doubletap")) ragebotExtra.doubletap = ragebotExtraJson["Doubletap"].asBool();
        if (ragebotExtraJson.isMember("Doubletap speed")) ragebotExtra.doubletapSpeed = ragebotExtraJson["Doubletap speed"].asInt();
        if (ragebotExtraJson.isMember("Resolver")) ragebotExtra.resolver = ragebotExtraJson["Resolver"].asBool();
    }

    for (size_t i = 0; i < glow.size(); i++) {
        const auto& glowJson = json["glow"][i];
        auto& glowConfig = glow[i];

        if (glowJson.isMember("Enabled")) glowConfig.enabled = glowJson["Enabled"].asBool();
        if (glowJson.isMember("healthBased")) glowConfig.healthBased = glowJson["healthBased"].asBool();

        // TODO: remove soon
        if (glowJson.isMember("alpha")) glowConfig.color[3] = glowJson["alpha"].asFloat();

        if (glowJson.isMember("style")) glowConfig.style = glowJson["style"].asInt();
        if (glowJson.isMember("Color")) {
            const auto& colorJson = glowJson["Color"];
            auto& colorConfig = glowConfig;

            if (colorJson.isMember("Color")) {
                colorConfig.color[0] = colorJson["Color"][0].asFloat();
                colorConfig.color[1] = colorJson["Color"][1].asFloat();
                colorConfig.color[2] = colorJson["Color"][2].asFloat();

                if (colorJson["Color"].size() == 4)
                    colorConfig.color[3] = colorJson["Color"][3].asFloat();
            }

            if (colorJson.isMember("Rainbow")) colorConfig.rainbow = colorJson["Rainbow"].asBool();
            if (colorJson.isMember("Rainbow speed")) colorConfig.rainbowSpeed = colorJson["Rainbow speed"].asFloat();
        }
    }

    for (size_t i = 0; i < chams.size(); i++) {
        const auto& chamsJson = json["Chams"][i];
        auto& chamsConfig = chams[i];

        for (size_t j = 0; j < chamsConfig.materials.size(); j++) {
            const auto& materialsJson = chamsJson[j];
            auto& materialsConfig = chams[i].materials[j];

            if (materialsJson.isMember("Enabled")) materialsConfig.enabled = materialsJson["Enabled"].asBool();
            if (materialsJson.isMember("Health based")) materialsConfig.healthBased = materialsJson["Health based"].asBool();
            if (materialsJson.isMember("Blinking")) materialsConfig.blinking = materialsJson["Blinking"].asBool();
            if (materialsJson.isMember("Material")) materialsConfig.material = materialsJson["Material"].asInt();
            if (materialsJson.isMember("Wireframe")) materialsConfig.wireframe = materialsJson["Wireframe"].asBool();
            if (materialsJson.isMember("Color")) {
                const auto& colorJson = materialsJson["Color"];
                auto& colorConfig = materialsConfig; // leftover

                if (colorJson.isMember("Color")) {
                    colorConfig.color[0] = colorJson["Color"][0].asFloat();
                    colorConfig.color[1] = colorJson["Color"][1].asFloat();
                    colorConfig.color[2] = colorJson["Color"][2].asFloat();

                    if (colorJson["Color"].size() == 4)
                        colorConfig.color[3] = colorJson["Color"][3].asFloat();
                }

                if (colorJson.isMember("Rainbow")) colorConfig.rainbow = colorJson["Rainbow"].asBool();
                if (colorJson.isMember("Rainbow speed")) colorConfig.rainbowSpeed = colorJson["Rainbow speed"].asFloat();
            }
            if (materialsJson.isMember("Alpha")) materialsConfig.color[3] = materialsJson["Alpha"].asFloat();
        }
    }

    for (size_t i = 0; i < esp.players.size(); i++) {
        const auto& espJson = json["Esp"]["Players"][i];
        auto& espConfig = esp.players[i];

        if (espJson.isMember("Enabled")) espConfig.enabled = espJson["Enabled"].asBool();
        if (espJson.isMember("HP side")) espConfig.hpside = espJson["HP side"].asInt();
        if (espJson.isMember("Armor side")) espConfig.armorside = espJson["Armor side"].asInt();

        if (espJson.isMember("Snaplines")) {
            const auto& snaplinesJson = espJson["Snaplines"];
            auto& snaplinesConfig = espConfig.snaplines;

            if (snaplinesJson.isMember("Enabled")) snaplinesConfig.enabled = snaplinesJson["Enabled"].asBool();

            if (snaplinesJson.isMember("Color")) {
                snaplinesConfig.color[0] = snaplinesJson["Color"][0].asFloat();
                snaplinesConfig.color[1] = snaplinesJson["Color"][1].asFloat();
                snaplinesConfig.color[2] = snaplinesJson["Color"][2].asFloat();
            }

            if (snaplinesJson.isMember("Rainbow")) snaplinesConfig.rainbow = snaplinesJson["Rainbow"].asBool();
            if (snaplinesJson.isMember("Rainbow speed")) snaplinesConfig.rainbowSpeed = snaplinesJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Eye traces")) {
            const auto& eyeTracesJson = espJson["Eye traces"];
            auto& eyeTracesConfig = espConfig.eyeTraces;

            if (eyeTracesJson.isMember("Enabled")) eyeTracesConfig.enabled = eyeTracesJson["Enabled"].asBool();

            if (eyeTracesJson.isMember("Color")) {
                eyeTracesConfig.color[0] = eyeTracesJson["Color"][0].asFloat();
                eyeTracesConfig.color[1] = eyeTracesJson["Color"][1].asFloat();
                eyeTracesConfig.color[2] = eyeTracesJson["Color"][2].asFloat();
            }

            if (eyeTracesJson.isMember("Rainbow")) eyeTracesConfig.rainbow = eyeTracesJson["Rainbow"].asBool();
            if (eyeTracesJson.isMember("Rainbow speed")) eyeTracesConfig.rainbowSpeed = eyeTracesJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box")) {
            const auto& boxJson = espJson["Box"];
            auto& boxConfig = espConfig.box;

            if (boxJson.isMember("Enabled")) boxConfig.enabled = boxJson["Enabled"].asBool();

            if (boxJson.isMember("Color")) {
                boxConfig.color[0] = boxJson["Color"][0].asFloat();
                boxConfig.color[1] = boxJson["Color"][1].asFloat();
                boxConfig.color[2] = boxJson["Color"][2].asFloat();
            }

            if (boxJson.isMember("Rainbow")) boxConfig.rainbow = boxJson["Rainbow"].asBool();
            if (boxJson.isMember("Rainbow speed")) boxConfig.rainbowSpeed = boxJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box type")) espConfig.boxType = espJson["Box type"].asInt();

        if (espJson.isMember("Name")) {
            const auto& nameJson = espJson["Name"];
            auto& nameConfig = espConfig.name;

            if (nameJson.isMember("Enabled")) nameConfig.enabled = nameJson["Enabled"].asBool();

            if (nameJson.isMember("Color")) {
                nameConfig.color[0] = nameJson["Color"][0].asFloat();
                nameConfig.color[1] = nameJson["Color"][1].asFloat();
                nameConfig.color[2] = nameJson["Color"][2].asFloat();
            }

            if (nameJson.isMember("Rainbow")) nameConfig.rainbow = nameJson["Rainbow"].asBool();
            if (nameJson.isMember("Rainbow speed")) nameConfig.rainbowSpeed = nameJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Health")) {
            const auto& healthJson = espJson["Health"];
            auto& healthConfig = espConfig.health;

            if (healthJson.isMember("Enabled")) healthConfig.enabled = healthJson["Enabled"].asBool();

            if (healthJson.isMember("Color")) {
                healthConfig.color[0] = healthJson["Color"][0].asFloat();
                healthConfig.color[1] = healthJson["Color"][1].asFloat();
                healthConfig.color[2] = healthJson["Color"][2].asFloat();
            }

            if (healthJson.isMember("Rainbow")) healthConfig.rainbow = healthJson["Rainbow"].asBool();
            if (healthJson.isMember("Rainbow speed")) healthConfig.rainbowSpeed = healthJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Health bar")) espConfig.healthBar = espJson["Health bar"].asBool();

        if (espJson.isMember("Health bar color")) {
            const auto& healthBarJson = espJson["Health bar color"];
            auto& healthBarConfig = espConfig.healthBarColor;

            if (healthBarJson.isMember("Enabled")) healthBarConfig.enabled = healthBarJson["Enabled"].asBool();

            if (healthBarJson.isMember("Color")) {
                healthBarConfig.color[0] = healthBarJson["Color"][0].asFloat();
                healthBarConfig.color[1] = healthBarJson["Color"][1].asFloat();
                healthBarConfig.color[2] = healthBarJson["Color"][2].asFloat();
            }

            if (healthBarJson.isMember("Rainbow")) healthBarConfig.rainbow = healthBarJson["Rainbow"].asBool();
            if (healthBarJson.isMember("Rainbow speed")) healthBarConfig.rainbowSpeed = healthBarJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Armor")) {
            const auto& armorJson = espJson["Armor"];
            auto& armorConfig = espConfig.armor;

            if (armorJson.isMember("Enabled")) armorConfig.enabled = armorJson["Enabled"].asBool();

            if (armorJson.isMember("Color")) {
                armorConfig.color[0] = armorJson["Color"][0].asFloat();
                armorConfig.color[1] = armorJson["Color"][1].asFloat();
                armorConfig.color[2] = armorJson["Color"][2].asFloat();
            }

            if (armorJson.isMember("Rainbow")) armorConfig.rainbow = armorJson["Rainbow"].asBool();
            if (armorJson.isMember("Rainbow speed")) armorConfig.rainbowSpeed = armorJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Armor bar")) {
            const auto& armorBarJson = espJson["Armor bar"];
            auto& armorBarConfig = espConfig.armorBar;

            if (armorBarJson.isMember("Enabled")) armorBarConfig.enabled = armorBarJson["Enabled"].asBool();

            if (armorBarJson.isMember("Color")) {
                armorBarConfig.color[0] = armorBarJson["Color"][0].asFloat();
                armorBarConfig.color[1] = armorBarJson["Color"][1].asFloat();
                armorBarConfig.color[2] = armorBarJson["Color"][2].asFloat();
            }

            if (armorBarJson.isMember("Rainbow")) armorBarConfig.rainbow = armorBarJson["Rainbow"].asBool();
            if (armorBarJson.isMember("Rainbow speed")) armorBarConfig.rainbowSpeed = armorBarJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Money")) {
            const auto& moneyJson = espJson["Money"];
            auto& moneyConfig = espConfig.money;

            if (moneyJson.isMember("Enabled")) moneyConfig.enabled = moneyJson["Enabled"].asBool();

            if (moneyJson.isMember("Color")) {
                moneyConfig.color[0] = moneyJson["Color"][0].asFloat();
                moneyConfig.color[1] = moneyJson["Color"][1].asFloat();
                moneyConfig.color[2] = moneyJson["Color"][2].asFloat();
            }

            if (moneyJson.isMember("Rainbow")) moneyConfig.rainbow = moneyJson["Rainbow"].asBool();
            if (moneyJson.isMember("Rainbow speed")) moneyConfig.rainbowSpeed = moneyJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Head dot")) {
            const auto& headDotJson = espJson["Head dot"];
            auto& headDotConfig = espConfig.headDot;

            if (headDotJson.isMember("Enabled")) headDotConfig.enabled = headDotJson["Enabled"].asBool();

            if (headDotJson.isMember("Color")) {
                headDotConfig.color[0] = headDotJson["Color"][0].asFloat();
                headDotConfig.color[1] = headDotJson["Color"][1].asFloat();
                headDotConfig.color[2] = headDotJson["Color"][2].asFloat();
            }

            if (headDotJson.isMember("Rainbow")) headDotConfig.rainbow = headDotJson["Rainbow"].asBool();
            if (headDotJson.isMember("Rainbow speed")) headDotConfig.rainbowSpeed = headDotJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Active weapon")) {
            const auto& activeWeaponJson = espJson["Active weapon"];
            auto& activeWeaponConfig = espConfig.activeWeapon;

            if (activeWeaponJson.isMember("Enabled")) activeWeaponConfig.enabled = activeWeaponJson["Enabled"].asBool();

            if (activeWeaponJson.isMember("Color")) {
                activeWeaponConfig.color[0] = activeWeaponJson["Color"][0].asFloat();
                activeWeaponConfig.color[1] = activeWeaponJson["Color"][1].asFloat();
                activeWeaponConfig.color[2] = activeWeaponJson["Color"][2].asFloat();
            }

            if (activeWeaponJson.isMember("Rainbow")) activeWeaponConfig.rainbow = activeWeaponJson["Rainbow"].asBool();
            if (activeWeaponJson.isMember("Rainbow speed")) activeWeaponConfig.rainbowSpeed = activeWeaponJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Outline")) {
            const auto& outlineJson = espJson["Outline"];
            auto& outlineConfig = espConfig.outline;

            if (outlineJson.isMember("Enabled")) outlineConfig.enabled = outlineJson["Enabled"].asBool();

            if (outlineJson.isMember("Color")) {
                outlineConfig.color[0] = outlineJson["Color"][0].asFloat();
                outlineConfig.color[1] = outlineJson["Color"][1].asFloat();
                outlineConfig.color[2] = outlineJson["Color"][2].asFloat();
            }

            if (outlineJson.isMember("Rainbow")) outlineConfig.rainbow = outlineJson["Rainbow"].asBool();
            if (outlineJson.isMember("Rainbow speed")) outlineConfig.rainbowSpeed = outlineJson["Rainbow speed"].asFloat();
        }
        if (espJson.isMember("Ammo")) {
            const auto& ammoJson = espJson["Ammo"];
            auto& ammoConfig = espConfig.ammo;

            if (ammoJson.isMember("Enabled")) ammoConfig.enabled = ammoJson["Enabled"].asBool();

            if (ammoJson.isMember("Color")) {
                ammoConfig.color[0] = ammoJson["Color"][0].asFloat();
                ammoConfig.color[1] = ammoJson["Color"][1].asFloat();
                ammoConfig.color[2] = ammoJson["Color"][2].asFloat();
            }

            if (ammoJson.isMember("Rainbow")) ammoConfig.rainbow = ammoJson["Rainbow"].asBool();
            if (ammoJson.isMember("Rainbow speed")) ammoConfig.rainbowSpeed = ammoJson["Rainbow speed"].asFloat();
        }
        if (espJson.isMember("Distance")) {
            const auto& distanceJson = espJson["Distance"];
            auto& distanceConfig = espConfig.distance;

            if (distanceJson.isMember("Enabled")) distanceConfig.enabled = distanceJson["Enabled"].asBool();

            if (distanceJson.isMember("Color")) {
                distanceConfig.color[0] = distanceJson["Color"][0].asFloat();
                distanceConfig.color[1] = distanceJson["Color"][1].asFloat();
                distanceConfig.color[2] = distanceJson["Color"][2].asFloat();
            }

            if (distanceJson.isMember("Rainbow")) distanceConfig.rainbow = distanceJson["Rainbow"].asBool();
            if (distanceJson.isMember("Rainbow speed")) distanceConfig.rainbowSpeed = distanceJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Dead ESP")) espConfig.deadesp = espJson["Dead ESP"].asBool();
        if (espJson.isMember("Max distance")) espConfig.maxDistance = espJson["Max distance"].asFloat();
    }

    {
        const auto& espJson = json["Esp"]["Weapons"];
        auto& espConfig = esp.weapon;

        if (espJson.isMember("Enabled")) espConfig.enabled = espJson["Enabled"].asBool();
        if (espJson.isMember("Snaplines")) {
            const auto& snaplinesJson = espJson["Snaplines"];
            auto& snaplinesConfig = espConfig.snaplines;

            if (snaplinesJson.isMember("Enabled")) snaplinesConfig.enabled = snaplinesJson["Enabled"].asBool();

            if (snaplinesJson.isMember("Color")) {
                snaplinesConfig.color[0] = snaplinesJson["Color"][0].asFloat();
                snaplinesConfig.color[1] = snaplinesJson["Color"][1].asFloat();
                snaplinesConfig.color[2] = snaplinesJson["Color"][2].asFloat();
            }

            if (snaplinesJson.isMember("Rainbow")) snaplinesConfig.rainbow = snaplinesJson["Rainbow"].asBool();
            if (snaplinesJson.isMember("Rainbow speed")) snaplinesConfig.rainbowSpeed = snaplinesJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box")) {
            const auto& boxJson = espJson["Box"];
            auto& boxConfig = espConfig.box;

            if (boxJson.isMember("Enabled")) boxConfig.enabled = boxJson["Enabled"].asBool();

            if (boxJson.isMember("Color")) {
                boxConfig.color[0] = boxJson["Color"][0].asFloat();
                boxConfig.color[1] = boxJson["Color"][1].asFloat();
                boxConfig.color[2] = boxJson["Color"][2].asFloat();
            }

            if (boxJson.isMember("Rainbow")) boxConfig.rainbow = boxJson["Rainbow"].asBool();
            if (boxJson.isMember("Rainbow speed")) boxConfig.rainbowSpeed = boxJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box type")) espConfig.boxType = espJson["Box type"].asInt();

        if (espJson.isMember("Outline")) {
            const auto& outlineJson = espJson["Outline"];
            auto& outlineConfig = espConfig.outline;

            if (outlineJson.isMember("Enabled")) outlineConfig.enabled = outlineJson["Enabled"].asBool();

            if (outlineJson.isMember("Color")) {
                outlineConfig.color[0] = outlineJson["Color"][0].asFloat();
                outlineConfig.color[1] = outlineJson["Color"][1].asFloat();
                outlineConfig.color[2] = outlineJson["Color"][2].asFloat();
            }

            if (outlineJson.isMember("Rainbow")) outlineConfig.rainbow = outlineJson["Rainbow"].asBool();
            if (outlineJson.isMember("Rainbow speed")) outlineConfig.rainbowSpeed = outlineJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Name")) {
            const auto& nameJson = espJson["Name"];
            auto& nameConfig = espConfig.name;

            if (nameJson.isMember("Enabled")) nameConfig.enabled = nameJson["Enabled"].asBool();

            if (nameJson.isMember("Color")) {
                nameConfig.color[0] = nameJson["Color"][0].asFloat();
                nameConfig.color[1] = nameJson["Color"][1].asFloat();
                nameConfig.color[2] = nameJson["Color"][2].asFloat();
            }

            if (nameJson.isMember("Rainbow")) nameConfig.rainbow = nameJson["Rainbow"].asBool();
            if (nameJson.isMember("Rainbow speed")) nameConfig.rainbowSpeed = nameJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Distance")) {
            const auto& distanceJson = espJson["Distance"];
            auto& distanceConfig = espConfig.distance;

            if (distanceJson.isMember("Enabled")) distanceConfig.enabled = distanceJson["Enabled"].asBool();

            if (distanceJson.isMember("Color")) {
                distanceConfig.color[0] = distanceJson["Color"][0].asFloat();
                distanceConfig.color[1] = distanceJson["Color"][1].asFloat();
                distanceConfig.color[2] = distanceJson["Color"][2].asFloat();
            }

            if (distanceJson.isMember("Rainbow")) distanceConfig.rainbow = distanceJson["Rainbow"].asBool();
            if (distanceJson.isMember("Rainbow speed")) distanceConfig.rainbowSpeed = distanceJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Max distance")) espConfig.maxDistance = espJson["Max distance"].asFloat();
    }

    for (size_t i = 0; i < esp.dangerZone.size(); i++) {
        const auto& espJson = json["Esp"]["Danger Zone"][i];
        auto& espConfig = esp.dangerZone[i];

        if (espJson.isMember("Enabled")) espConfig.enabled = espJson["Enabled"].asBool();
        if (espJson.isMember("Snaplines")) {
            const auto& snaplinesJson = espJson["Snaplines"];
            auto& snaplinesConfig = espConfig.snaplines;

            if (snaplinesJson.isMember("Enabled")) snaplinesConfig.enabled = snaplinesJson["Enabled"].asBool();

            if (snaplinesJson.isMember("Color")) {
                snaplinesConfig.color[0] = snaplinesJson["Color"][0].asFloat();
                snaplinesConfig.color[1] = snaplinesJson["Color"][1].asFloat();
                snaplinesConfig.color[2] = snaplinesJson["Color"][2].asFloat();
            }

            if (snaplinesJson.isMember("Rainbow")) snaplinesConfig.rainbow = snaplinesJson["Rainbow"].asBool();
            if (snaplinesJson.isMember("Rainbow speed")) snaplinesConfig.rainbowSpeed = snaplinesJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box")) {
            const auto& boxJson = espJson["Box"];
            auto& boxConfig = espConfig.box;

            if (boxJson.isMember("Enabled")) boxConfig.enabled = boxJson["Enabled"].asBool();

            if (boxJson.isMember("Color")) {
                boxConfig.color[0] = boxJson["Color"][0].asFloat();
                boxConfig.color[1] = boxJson["Color"][1].asFloat();
                boxConfig.color[2] = boxJson["Color"][2].asFloat();
            }

            if (boxJson.isMember("Rainbow")) boxConfig.rainbow = boxJson["Rainbow"].asBool();
            if (boxJson.isMember("Rainbow speed")) boxConfig.rainbowSpeed = boxJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box type")) espConfig.boxType = espJson["Box type"].asInt();

        if (espJson.isMember("Outline")) {
            const auto& outlineJson = espJson["Outline"];
            auto& outlineConfig = espConfig.outline;

            if (outlineJson.isMember("Enabled")) outlineConfig.enabled = outlineJson["Enabled"].asBool();

            if (outlineJson.isMember("Color")) {
                outlineConfig.color[0] = outlineJson["Color"][0].asFloat();
                outlineConfig.color[1] = outlineJson["Color"][1].asFloat();
                outlineConfig.color[2] = outlineJson["Color"][2].asFloat();
            }

            if (outlineJson.isMember("Rainbow")) outlineConfig.rainbow = outlineJson["Rainbow"].asBool();
            if (outlineJson.isMember("Rainbow speed")) outlineConfig.rainbowSpeed = outlineJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Name")) {
            const auto& nameJson = espJson["Name"];
            auto& nameConfig = espConfig.name;

            if (nameJson.isMember("Enabled")) nameConfig.enabled = nameJson["Enabled"].asBool();

            if (nameJson.isMember("Color")) {
                nameConfig.color[0] = nameJson["Color"][0].asFloat();
                nameConfig.color[1] = nameJson["Color"][1].asFloat();
                nameConfig.color[2] = nameJson["Color"][2].asFloat();
            }

            if (nameJson.isMember("Rainbow")) nameConfig.rainbow = nameJson["Rainbow"].asBool();
            if (nameJson.isMember("Rainbow speed")) nameConfig.rainbowSpeed = nameJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Distance")) {
            const auto& distanceJson = espJson["Distance"];
            auto& distanceConfig = espConfig.distance;

            if (distanceJson.isMember("Enabled")) distanceConfig.enabled = distanceJson["Enabled"].asBool();

            if (distanceJson.isMember("Color")) {
                distanceConfig.color[0] = distanceJson["Color"][0].asFloat();
                distanceConfig.color[1] = distanceJson["Color"][1].asFloat();
                distanceConfig.color[2] = distanceJson["Color"][2].asFloat();
            }

            if (distanceJson.isMember("Rainbow")) distanceConfig.rainbow = distanceJson["Rainbow"].asBool();
            if (distanceJson.isMember("Rainbow speed")) distanceConfig.rainbowSpeed = distanceJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Max distance")) espConfig.maxDistance = espJson["Max distance"].asFloat();
    }

    for (size_t i = 0; i < esp.projectiles.size(); i++) {
        const auto& espJson = json["Esp"]["Projectiles"][i];
        auto& espConfig = esp.projectiles[i];

        if (espJson.isMember("Enabled")) espConfig.enabled = espJson["Enabled"].asBool();
        if (espJson.isMember("Snaplines")) {
            const auto& snaplinesJson = espJson["Snaplines"];
            auto& snaplinesConfig = espConfig.snaplines;

            if (snaplinesJson.isMember("Enabled")) snaplinesConfig.enabled = snaplinesJson["Enabled"].asBool();

            if (snaplinesJson.isMember("Color")) {
                snaplinesConfig.color[0] = snaplinesJson["Color"][0].asFloat();
                snaplinesConfig.color[1] = snaplinesJson["Color"][1].asFloat();
                snaplinesConfig.color[2] = snaplinesJson["Color"][2].asFloat();
            }

            if (snaplinesJson.isMember("Rainbow")) snaplinesConfig.rainbow = snaplinesJson["Rainbow"].asBool();
            if (snaplinesJson.isMember("Rainbow speed")) snaplinesConfig.rainbowSpeed = snaplinesJson["Rainbow speed"].asFloat();
        }
        if (espJson.isMember("Box")) {
            const auto& boxJson = espJson["Box"];
            auto& boxConfig = espConfig.box;

            if (boxJson.isMember("Enabled")) boxConfig.enabled = boxJson["Enabled"].asBool();

            if (boxJson.isMember("Color")) {
                boxConfig.color[0] = boxJson["Color"][0].asFloat();
                boxConfig.color[1] = boxJson["Color"][1].asFloat();
                boxConfig.color[2] = boxJson["Color"][2].asFloat();
            }

            if (boxJson.isMember("Rainbow")) boxConfig.rainbow = boxJson["Rainbow"].asBool();
            if (boxJson.isMember("Rainbow speed")) boxConfig.rainbowSpeed = boxJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Box type")) espConfig.boxType = espJson["Box type"].asInt();

        if (espJson.isMember("Outline")) {
            const auto& outlineJson = espJson["Outline"];
            auto& outlineConfig = espConfig.outline;

            if (outlineJson.isMember("Enabled")) outlineConfig.enabled = outlineJson["Enabled"].asBool();

            if (outlineJson.isMember("Color")) {
                outlineConfig.color[0] = outlineJson["Color"][0].asFloat();
                outlineConfig.color[1] = outlineJson["Color"][1].asFloat();
                outlineConfig.color[2] = outlineJson["Color"][2].asFloat();
            }

            if (outlineJson.isMember("Rainbow")) outlineConfig.rainbow = outlineJson["Rainbow"].asBool();
            if (outlineJson.isMember("Rainbow speed")) outlineConfig.rainbowSpeed = outlineJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Name")) {
            const auto& nameJson = espJson["Name"];
            auto& nameConfig = espConfig.name;

            if (nameJson.isMember("Enabled")) nameConfig.enabled = nameJson["Enabled"].asBool();

            if (nameJson.isMember("Color")) {
                nameConfig.color[0] = nameJson["Color"][0].asFloat();
                nameConfig.color[1] = nameJson["Color"][1].asFloat();
                nameConfig.color[2] = nameJson["Color"][2].asFloat();
            }

            if (nameJson.isMember("Rainbow")) nameConfig.rainbow = nameJson["Rainbow"].asBool();
            if (nameJson.isMember("Rainbow speed")) nameConfig.rainbowSpeed = nameJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Distance")) {
            const auto& distanceJson = espJson["Distance"];
            auto& distanceConfig = espConfig.distance;

            if (distanceJson.isMember("Enabled")) distanceConfig.enabled = distanceJson["Enabled"].asBool();

            if (distanceJson.isMember("Color")) {
                distanceConfig.color[0] = distanceJson["Color"][0].asFloat();
                distanceConfig.color[1] = distanceJson["Color"][1].asFloat();
                distanceConfig.color[2] = distanceJson["Color"][2].asFloat();
            }

            if (distanceJson.isMember("Rainbow")) distanceConfig.rainbow = distanceJson["Rainbow"].asBool();
            if (distanceJson.isMember("Rainbow speed")) distanceConfig.rainbowSpeed = distanceJson["Rainbow speed"].asFloat();
        }

        if (espJson.isMember("Max distance")) espConfig.maxDistance = espJson["Max distance"].asFloat();
    }

    {
        const auto& visualsJson = json["visuals"];
        if (visualsJson.isMember("disablePostProcessing")) visuals.disablePostProcessing = visualsJson["disablePostProcessing"].asBool();
        if (visualsJson.isMember("inverseRagdollGravity")) visuals.inverseRagdollGravity = visualsJson["inverseRagdollGravity"].asBool();
        if (visualsJson.isMember("noFog")) visuals.noFog = visualsJson["noFog"].asBool();
        if (visualsJson.isMember("no3dSky")) visuals.no3dSky = visualsJson["no3dSky"].asBool();
        if (visualsJson.isMember("No aim punch")) visuals.noAimPunch = visualsJson["No aim punch"].asBool();
        if (visualsJson.isMember("No view punch")) visuals.noViewPunch = visualsJson["No view punch"].asBool();
        if (visualsJson.isMember("noHands")) visuals.noHands = visualsJson["noHands"].asBool();
        if (visualsJson.isMember("noSleeves")) visuals.noSleeves = visualsJson["noSleeves"].asBool();
        if (visualsJson.isMember("noWeapons")) visuals.noWeapons = visualsJson["noWeapons"].asBool();
        if (visualsJson.isMember("noSmoke")) visuals.noSmoke = visualsJson["noSmoke"].asBool();
        if (visualsJson.isMember("noBlur")) visuals.noBlur = visualsJson["noBlur"].asBool();
        if (visualsJson.isMember("noScopeOverlay")) visuals.noScopeOverlay = visualsJson["noScopeOverlay"].asBool();
        if (visualsJson.isMember("noGrass")) visuals.noGrass = visualsJson["noGrass"].asBool();
        if (visualsJson.isMember("noShadows")) visuals.noShadows = visualsJson["noShadows"].asBool();
        if (visualsJson.isMember("wireframeSmoke")) visuals.wireframeSmoke = visualsJson["wireframeSmoke"].asBool();
        if (visualsJson.isMember("thirdperson")) visuals.thirdperson = visualsJson["thirdperson"].asBool();
        if (visualsJson.isMember("thirdpersonKey")) visuals.thirdpersonKey = visualsJson["thirdpersonKey"].asInt();
        if (visualsJson.isMember("thirdpersonDistance")) visuals.thirdpersonDistance = visualsJson["thirdpersonDistance"].asInt();
        if (visualsJson.isMember("viewmodelFov")) visuals.viewmodelFov = visualsJson["viewmodelFov"].asInt();
        if (visualsJson.isMember("Fov")) visuals.fov = visualsJson["Fov"].asInt();
        if (visualsJson.isMember("Scoped Fov")) visuals.fovScoped = visualsJson["Scoped Fov"].asInt();
        if (visualsJson.isMember("farZ")) visuals.farZ = visualsJson["farZ"].asInt();
        if (visualsJson.isMember("flashReduction")) visuals.flashReduction = visualsJson["flashReduction"].asInt();
        if (visualsJson.isMember("brightness")) visuals.brightness = visualsJson["brightness"].asFloat();
        if (visualsJson.isMember("skybox")) visuals.skybox = visualsJson["skybox"].asInt();
        if (visualsJson.isMember("Nightmode")) visuals.nightMode = visualsJson["Nightmode"].asBool();
        if (visualsJson.isMember("Nightmode override")) {
            auto& visualsConfig = visuals;
            const auto& nightmodeJson = visualsJson["Nightmode override"];
            auto& nightmodeConfig = visualsConfig.nightModeOverride;

            if (nightmodeJson.isMember("Enabled")) nightmodeConfig.enabled = nightmodeJson["Enabled"].asBool();

            if (nightmodeJson.isMember("Color")) {
                nightmodeConfig.color[0] = nightmodeJson["Color"][0].asFloat();
                nightmodeConfig.color[1] = nightmodeJson["Color"][1].asFloat();
                nightmodeConfig.color[2] = nightmodeJson["Color"][2].asFloat();
            }

            if (nightmodeJson.isMember("Rainbow")) nightmodeConfig.rainbow = nightmodeJson["Rainbow"].asBool();
            if (nightmodeJson.isMember("Rainbow speed")) nightmodeConfig.rainbowSpeed = nightmodeJson["Rainbow speed"].asFloat();
        }
        if (visualsJson.isMember("Deagle spinner")) visuals.deagleSpinner = visualsJson["Deagle spinner"].asBool();
        if (visualsJson.isMember("Screen effect")) visuals.screenEffect = visualsJson["Screen effect"].asInt();
        if (visualsJson.isMember("Hit effect")) visuals.hitEffect = visualsJson["Hit effect"].asInt();
        if (visualsJson.isMember("Hit effect time")) visuals.hitEffectTime = visualsJson["Hit effect time"].asFloat();
        if (visualsJson.isMember("Hitmarker")) visuals.hitMarker = visualsJson["Hitmarker"].asBool();
        if (visualsJson.isMember("Playermodel T")) visuals.playerModelT = visualsJson["Playermodel T"].asInt();
        if (visualsJson.isMember("Playermodel CT")) visuals.playerModelCT = visualsJson["Playermodel CT"].asInt();

        if (visualsJson.isMember("Color correction")) {
            const auto& cc = visualsJson["Color correction"];

            if (cc.isMember("Enabled")) visuals.colorCorrection.enabled = cc["Enabled"].asBool();
            if (cc.isMember("Blue")) visuals.colorCorrection.blue = cc["Blue"].asFloat();
            if (cc.isMember("Red")) visuals.colorCorrection.red = cc["Red"].asFloat();
            if (cc.isMember("Mono")) visuals.colorCorrection.mono = cc["Mono"].asFloat();
            if (cc.isMember("Saturation")) visuals.colorCorrection.saturation = cc["Saturation"].asFloat();
            if (cc.isMember("Ghost")) visuals.colorCorrection.ghost = cc["Ghost"].asFloat();
            if (cc.isMember("Green")) visuals.colorCorrection.green = cc["Green"].asFloat();
            if (cc.isMember("Yellow")) visuals.colorCorrection.yellow = cc["Yellow"].asFloat();
        }

        if (visualsJson.isMember("Asus walls")) visuals.asusWalls = visualsJson["Asus walls"].asInt();
        if (visualsJson.isMember("Asus props")) visuals.asusProps = visualsJson["Asus props"].asInt();
        if (visualsJson.isMember("Thirdperson transparency")) visuals.thirdpersonTransparency = visualsJson["Thirdperson transparency"].asInt();

        if (visualsJson.isMember("Local bullet beams")) {
            const auto& bulletbeamsJson = visualsJson["Local bullet beams"];

            if (bulletbeamsJson.isMember("Enabled")) visuals.bulletTracersLocal.enabled = bulletbeamsJson["Enabled"].asBool();

            if (bulletbeamsJson.isMember("Color")) {
                visuals.bulletTracersLocal.color[0] = bulletbeamsJson["Color"][0].asFloat();
                visuals.bulletTracersLocal.color[1] = bulletbeamsJson["Color"][1].asFloat();
                visuals.bulletTracersLocal.color[2] = bulletbeamsJson["Color"][2].asFloat();
            }
            if (bulletbeamsJson.isMember("Rainbow")) visuals.bulletTracersLocal.rainbow = bulletbeamsJson["Rainbow"].asBool();
            if (bulletbeamsJson.isMember("Rainbow speed")) visuals.bulletTracersLocal.rainbowSpeed = bulletbeamsJson["Rainbow speed"].asFloat();
        }

        if (visualsJson.isMember("Teammate bullet beams")) {
            const auto& bulletbeamsJson = visualsJson["Teammate bullet beams"];

            if (bulletbeamsJson.isMember("Enabled")) visuals.bulletTracersAllies.enabled = bulletbeamsJson["Enabled"].asBool();

            if (bulletbeamsJson.isMember("Color")) {
                visuals.bulletTracersAllies.color[0] = bulletbeamsJson["Color"][0].asFloat();
                visuals.bulletTracersAllies.color[1] = bulletbeamsJson["Color"][1].asFloat();
                visuals.bulletTracersAllies.color[2] = bulletbeamsJson["Color"][2].asFloat();
            }
            if (bulletbeamsJson.isMember("Rainbow")) visuals.bulletTracersAllies.rainbow = bulletbeamsJson["Rainbow"].asBool();
            if (bulletbeamsJson.isMember("Rainbow speed")) visuals.bulletTracersAllies.rainbowSpeed = bulletbeamsJson["Rainbow speed"].asFloat();
        }

        if (visualsJson.isMember("Enemy bullet beams")) {
            const auto& bulletbeamsJson = visualsJson["Enemy bullet beams"];

            if (bulletbeamsJson.isMember("Enabled")) visuals.bulletTracersEnemy.enabled = bulletbeamsJson["Enabled"].asBool();

            if (bulletbeamsJson.isMember("Color")) {
                visuals.bulletTracersEnemy.color[0] = bulletbeamsJson["Color"][0].asFloat();
                visuals.bulletTracersEnemy.color[1] = bulletbeamsJson["Color"][1].asFloat();
                visuals.bulletTracersEnemy.color[2] = bulletbeamsJson["Color"][2].asFloat();
            }
            if (bulletbeamsJson.isMember("Rainbow")) visuals.bulletTracersEnemy.rainbow = bulletbeamsJson["Rainbow"].asBool();
            if (bulletbeamsJson.isMember("Rainbow speed")) visuals.bulletTracersEnemy.rainbowSpeed = bulletbeamsJson["Rainbow speed"].asFloat();
        }

        if (visualsJson.isMember("AA lines")) visuals.aaLines = visualsJson["AA lines"].asBool();
        if (visualsJson.isMember("Penetration Crosshair")) visuals.PenetrationCrosshair = visualsJson["Penetration Crosshair"].asBool();
    }

    for (size_t i = 0; i < skinChanger.size(); i++) {
        const auto& skinChangerJson = json["skinChanger"][i];
        auto& skinChangerConfig = skinChanger[i];

        if (skinChangerJson.isMember("Enabled")) skinChangerConfig.enabled = skinChangerJson["Enabled"].asBool();
        if (skinChangerJson.isMember("definition_vector_index")) skinChangerConfig.itemIdIndex = skinChangerJson["definition_vector_index"].asInt();
        if (skinChangerJson.isMember("definition_index")) skinChangerConfig.itemId = skinChangerJson["definition_index"].asInt();
        if (skinChangerJson.isMember("entity_quality_vector_index")) skinChangerConfig.entity_quality_vector_index = skinChangerJson["entity_quality_vector_index"].asInt();
        if (skinChangerJson.isMember("entity_quality_index")) skinChangerConfig.quality = skinChangerJson["entity_quality_index"].asInt();
        if (skinChangerJson.isMember("paint_kit_vector_index")) skinChangerConfig.paint_kit_vector_index = skinChangerJson["paint_kit_vector_index"].asInt();
        if (skinChangerJson.isMember("paint_kit_index")) skinChangerConfig.paintKit = skinChangerJson["paint_kit_index"].asInt();
        if (skinChangerJson.isMember("definition_override_vector_index")) skinChangerConfig.definition_override_vector_index = skinChangerJson["definition_override_vector_index"].asInt();
        if (skinChangerJson.isMember("definition_override_index")) skinChangerConfig.definition_override_index = skinChangerJson["definition_override_index"].asInt();
        if (skinChangerJson.isMember("seed")) skinChangerConfig.seed = skinChangerJson["seed"].asInt();
        if (skinChangerJson.isMember("stat_trak")) skinChangerConfig.stat_trak = skinChangerJson["stat_trak"].asInt();
        if (skinChangerJson.isMember("wear")) skinChangerConfig.wear = skinChangerJson["wear"].asFloat();
        if (skinChangerJson.isMember("custom_name")) strcpy_s(skinChangerConfig.custom_name, sizeof(skinChangerConfig.custom_name), skinChangerJson["custom_name"].asCString());

        if (skinChangerJson.isMember("stickers")) {
            for (size_t j = 0; j < skinChangerConfig.stickers.size(); j++) {
                const auto& stickerJson = skinChangerJson["stickers"][j];
                auto& stickerConfig = skinChangerConfig.stickers[j];

                if (stickerJson.isMember("kit")) stickerConfig.kit = stickerJson["kit"].asInt();
                if (stickerJson.isMember("kit_vector_index")) stickerConfig.kit_vector_index = stickerJson["kit_vector_index"].asInt();
                if (stickerJson.isMember("wear")) stickerConfig.wear = stickerJson["wear"].asFloat();
                if (stickerJson.isMember("scale")) stickerConfig.scale = stickerJson["scale"].asFloat();
                if (stickerJson.isMember("rotation")) stickerConfig.rotation = stickerJson["rotation"].asFloat();
            }
        }
    }

    {
        const auto& soundJson = json["Sound"];

        if (soundJson.isMember("Chicken volume")) sound.chickenVolume = soundJson["Chicken volume"].asInt();

        if (soundJson.isMember("Players")) {
            for (size_t i = 0; i < sound.players.size(); i++) {
                const auto& playerJson = soundJson["Players"][i];
                auto& playerConfig = sound.players[i];

                if (playerJson.isMember("Master volume")) playerConfig.masterVolume = playerJson["Master volume"].asInt();
                if (playerJson.isMember("Headshot volume")) playerConfig.headshotVolume = playerJson["Headshot volume"].asInt();
                if (playerJson.isMember("Weapon volume")) playerConfig.weaponVolume = playerJson["Weapon volume"].asInt();
                if (playerJson.isMember("Footstep volume")) playerConfig.footstepVolume = playerJson["Footstep volume"].asInt();
            }
        }
    }

    {
        const auto& miscJson = json["Misc"];

        if (miscJson.isMember("Menu key")) misc.menuKey = miscJson["Menu key"].asInt();
        if (miscJson.isMember("Anti AFK kick")) misc.antiAfkKick = miscJson["Anti AFK kick"].asBool();
        if (miscJson.isMember("Auto strafe")) misc.autoStrafe = miscJson["Auto strafe"].asBool();
        if (miscJson.isMember("Bunny hop")) misc.bunnyHop = miscJson["Bunny hop"].asBool();
        if (miscJson.isMember("Clantag")) misc.clanTag = miscJson["Clantag"].asBool();
        if (miscJson.isMember("Fast duck")) misc.fastDuck = miscJson["Fast duck"].asBool();
        if (miscJson.isMember("Moonwalk")) misc.moonwalk = miscJson["Moonwalk"].asBool();
        if (miscJson.isMember("Edge Jump")) misc.edgejump = miscJson["Edge Jump"].asBool();
        if (miscJson.isMember("Edge Jump Key")) misc.edgejumpkey = miscJson["Edge Jump Key"].asInt();
        if (miscJson.isMember("Quick peek")) misc.quickpeek = miscJson["Quick peek"].asBool();
        if (miscJson.isMember("Quick peek key")) misc.quickpeekkey = miscJson["Quick peek key"].asInt();
        if (miscJson.isMember("Slowwalk")) misc.slowwalk = miscJson["Slowwalk"].asBool();
        if (miscJson.isMember("Slowwalk key")) misc.slowwalkKey = miscJson["Slowwalk key"].asInt();
        if (miscJson.isMember("Fast Stop")) misc.fastStop = miscJson["Fast Stop"].asBool();
        if (miscJson.isMember("Sniper crosshair")) misc.sniperCrosshair = miscJson["Sniper crosshair"].asBool();
        if (miscJson.isMember("Recoil crosshair")) misc.recoilCrosshair = miscJson["Recoil crosshair"].asBool();
        if (miscJson.isMember("Auto pistol")) misc.autoPistol = miscJson["Auto pistol"].asBool();
        if (miscJson.isMember("Auto accept")) misc.autoAccept = miscJson["Auto accept"].asBool();
        if (miscJson.isMember("Radar hack")) misc.radarHack = miscJson["Radar hack"].asBool();
        if (miscJson.isMember("Reveal ranks")) misc.revealRanks = miscJson["Reveal ranks"].asBool();
        if (miscJson.isMember("Reveal money")) misc.revealMoney = miscJson["Reveal money"].asBool();
        if (miscJson.isMember("Reveal suspect")) misc.revealSuspect = miscJson["Reveal suspect"].asBool();
        if (miscJson.isMember("Spec list")) misc.spectatorList = miscJson["Spec list"].asBool();
        if (miscJson.isMember("Debug window")) misc.debugwindow = miscJson["Debug window"].asBool();
        if (miscJson.isMember("Water mark")) misc.watermark = miscJson["Water mark"].asBool();
        if (miscJson.isMember("Disable model occlusion")) misc.disableModelOcclusion = miscJson["Disable model occlusion"].asBool();
        if (miscJson.isMember("Aspect Ratio")) misc.aspectratio = miscJson["Aspect Ratio"].asFloat();
        if (miscJson.isMember("Killsay")) misc.killSay = miscJson["Killsay"].asBool();
        if (miscJson.isMember("Chicken deathsay")) misc.chickenDeathSay = miscJson["Chicken deathsay"].asBool();
        if (miscJson.isMember("Name stealer"))  misc.nameStealer = miscJson["Name stealer"].asBool();
        if (miscJson.isMember("Disable HUD blur"))  misc.disablePanoramablur = miscJson["Disable HUD blur"].asBool();
        if (miscJson.isMember("Ban color")) misc.banColor = miscJson["Ban color"].asInt();
        if (miscJson.isMember("Ban text")) misc.banText = miscJson["Ban text"].asString();
        if (miscJson.isMember("Fast plant")) misc.fastPlant = miscJson["Fast plant"].asBool();

        if (const auto& bombTimer{ miscJson["Bomb timer"] }; bombTimer.isObject()) {
            if (const auto& enabled{ bombTimer["Enabled"] }; enabled.isBool())
                misc.bombTimer.enabled = enabled.asBool();

            if (const auto& color{ bombTimer["Color"] }; color.isArray()) {
                misc.bombTimer.color[0] = color[0].asFloat();
                misc.bombTimer.color[1] = color[1].asFloat();
                misc.bombTimer.color[2] = color[2].asFloat();
            }
            if (const auto& rainbow{ bombTimer["Rainbow"] }; rainbow.isBool())
                misc.bombTimer.rainbow = rainbow.asBool();

            if (const auto& rainbowSpeed{ bombTimer["Rainbow speed"] }; rainbowSpeed.isDouble())
                misc.bombTimer.rainbowSpeed = rainbowSpeed.asFloat();
        }

        if (miscJson.isMember("Prepare revolver")) misc.prepareRevolver = miscJson["Prepare revolver"].asBool();
        if (miscJson.isMember("Prepare revolver key")) misc.prepareRevolverKey = miscJson["Prepare revolver key"].asInt();
        if (miscJson.isMember("Hit sound")) misc.hitSound = miscJson["Hit sound"].asInt();
        if (miscJson.isMember("Grenade predict")) misc.nadePredict = miscJson["Grenade predict"].asBool();
        if (miscJson.isMember("Grenade trajectory")) misc.nadeTrajectory = miscJson["Grenade trajectory"].asBool();
        if (miscJson.isMember("Show impacts")) misc.showImpacts = miscJson["Show impacts"].asBool();
        if (miscJson.isMember("Max angle delta")) misc.maxAngleDelta = miscJson["Max angle delta"].asFloat();
        if (miscJson.isMember("Fake prime")) misc.fakePrime = miscJson["Fake prime"].asBool();
        if (miscJson.isMember("Custom Hit Sound")) misc.customHitSound = miscJson["Custom Hit Sound"].asString();
        if (miscJson.isMember("Kill sound")) misc.killSound = miscJson["Kill sound"].asInt();
        if (miscJson.isMember("Custom Kill Sound")) misc.customKillSound = miscJson["Custom Kill Sound"].asString();
        

        if (const auto& purchaseList = miscJson["Purchase List"]; purchaseList.isObject()) {
            if (const auto& enabled{ purchaseList["Enabled"] }; enabled.isBool())
                misc.purchaseList.enabled = enabled.asBool();
            if (const auto& onlyDuringFreezeTime{ purchaseList["Only During Freeze Time"] }; onlyDuringFreezeTime.isBool())
                misc.purchaseList.onlyDuringFreezeTime = onlyDuringFreezeTime.asBool();
            if (const auto& showPrices{ purchaseList["Show Prices"] }; showPrices.isBool())
                misc.purchaseList.showPrices = showPrices.asBool();
            if (const auto& noTitleBar{ purchaseList["No Title Bar"] }; noTitleBar.isBool())
                misc.purchaseList.noTitleBar = noTitleBar.asBool();
            if (const auto& mode{ purchaseList["Mode"] }; mode.isInt())
                misc.purchaseList.mode = mode.asInt();
        }

        if (const auto& autoBuy{ miscJson["Autobuy"] }; autoBuy.isObject()) {
            if (const auto& enabled{ autoBuy["Enabled"] }; enabled.isBool())
                misc.autoBuy.enabled = enabled.asBool();

            if (const auto& primaryWeapon{ autoBuy["Primary weapon"] }; primaryWeapon.isInt())
                misc.autoBuy.primaryWeapon = primaryWeapon.asInt();

            if (const auto& secondaryWeapon{ autoBuy["Secondary weapon"] }; secondaryWeapon.isInt())
                misc.autoBuy.secondaryWeapon = secondaryWeapon.asInt();

            if (const auto& armor{ autoBuy["Armor"] }; armor.isInt())
                misc.autoBuy.armor = armor.asInt();

            if (const auto& utility{ autoBuy["Utility"] }; utility.isArray()) {
                misc.autoBuy.utility[0] = utility[0].asBool();
                misc.autoBuy.utility[1] = utility[1].asBool();
            }

            if (const auto& nades{ autoBuy["Grenades"] }; nades.isArray()) {
                misc.autoBuy.grenades[0] = nades[0].asBool();
                misc.autoBuy.grenades[1] = nades[1].asBool();
                misc.autoBuy.grenades[2] = nades[2].asBool();
                misc.autoBuy.grenades[3] = nades[3].asBool();
                misc.autoBuy.grenades[4] = nades[4].asBool();
            }
        }

        if (miscJson.isMember("Sv pure bypass")) misc.svpurebypass = miscJson["Sv pure bypass"].asBool();
        if (miscJson.isMember("Unlock Inventory")) misc.unlockInventory = miscJson["Unlock Inventory"].asBool();
        if (miscJson.isMember("Preserve killfeed")) misc.preserveDeathNotices = miscJson["Preserve killfeed"].asBool();
        if (miscJson.isMember("Auto disconnect")) misc.autoDisconnect = miscJson["Auto disconnect"].asBool();
    }
}

void Config::save(size_t id) const noexcept
{
    Json::Value json;

    for (size_t i = 0; i < legitbot.size(); i++) {
        auto& legitbotJson = json["Legitbot"][i];
        const auto& legitbotConfig = legitbot[i];

        legitbotJson["Enabled"] = legitbotConfig.enabled;
        legitbotJson["On key"] = legitbotConfig.onKey;
        legitbotJson["Key"] = legitbotConfig.key;
        legitbotJson["Key mode"] = legitbotConfig.keyMode;
        legitbotJson["Aimlock"] = legitbotConfig.aimlock;
        legitbotJson["Silent"] = legitbotConfig.silent;
        legitbotJson["Friendly fire"] = legitbotConfig.friendlyFire;
        legitbotJson["Scoped only"] = legitbotConfig.scopedOnly;
        legitbotJson["Ignore flash"] = legitbotConfig.ignoreFlash;;
        legitbotJson["Ignore smoke"] = legitbotConfig.ignoreSmoke;
        legitbotJson["Fov"] = legitbotConfig.fov;
        legitbotJson["Smooth"] = legitbotConfig.smooth;
        legitbotJson["Reaction Time"] = legitbotConfig.reactionTime;
        legitbotJson["Multipoint"] = legitbotConfig.multipoint;
        legitbotJson["BacktrackLegitbot"] = legitbotConfig.backtrack;

        {
            auto& hitboxesJson = legitbotJson["Hitboxes"];
            const auto& hitboxesConfig = legitbotConfig;
            hitboxesJson["Hitboxes"][0] = hitboxesConfig.hitboxes[0];
            hitboxesJson["Hitboxes"][1] = hitboxesConfig.hitboxes[1];
            hitboxesJson["Hitboxes"][2] = hitboxesConfig.hitboxes[2];
            hitboxesJson["Hitboxes"][3] = hitboxesConfig.hitboxes[3];
            hitboxesJson["Hitboxes"][4] = hitboxesConfig.hitboxes[4];
        }
    }

    for (size_t i = 0; i < ragebot.size(); i++) {
        auto& ragebotJson = json["Ragebot"][i];
        const auto& ragebotConfig = ragebot[i];

        ragebotJson["Enabled"] = ragebotConfig.enabled;
        ragebotJson["On key"] = ragebotConfig.onKey;
        ragebotJson["Key"] = ragebotConfig.key;
        ragebotJson["Key mode"] = ragebotConfig.keyMode;
        ragebotJson["Silent"] = ragebotConfig.silent;
        ragebotJson["Friendly fire"] = ragebotConfig.friendlyFire;
        ragebotJson["Visible only"] = ragebotConfig.visibleOnly;
        ragebotJson["Scoped only"] = ragebotConfig.scopedOnly;
        ragebotJson["Ignore flash"] = ragebotConfig.ignoreFlash;;
        ragebotJson["Ignore smoke"] = ragebotConfig.ignoreSmoke;
        ragebotJson["Auto shot"] = ragebotConfig.autoShot;
        ragebotJson["Auto scope"] = ragebotConfig.autoScope;
        ragebotJson["Auto stop"] = ragebotConfig.autoStop;
        ragebotJson["Between shots"] = ragebotConfig.betweenShots;
        ragebotJson["Fov"] = ragebotConfig.fov;
        ragebotJson["Hitchance"] = ragebotConfig.hitChance;
        ragebotJson["Multipoint"] = ragebotConfig.multiPoint;
        ragebotJson["Priority"] = ragebotConfig.priority;
        ragebotJson["Min damage"] = ragebotConfig.minDamage;
        ragebotJson["Min damage override key"] = ragebotConfig.overrideDamageKey;
        ragebotJson["Min damage override amount"] = ragebotConfig.overrideDamageAmount;
        ragebotJson["Body aim key"] = ragebotConfig.bodyAimKey;

        {
            auto& hitboxesJson = ragebotJson["Hitboxes"];
            const auto& hitboxesConfig = ragebotConfig;
            hitboxesJson["Hitboxes"][0] = hitboxesConfig.hitboxes[0];
            hitboxesJson["Hitboxes"][1] = hitboxesConfig.hitboxes[1];
            hitboxesJson["Hitboxes"][2] = hitboxesConfig.hitboxes[2];
            hitboxesJson["Hitboxes"][3] = hitboxesConfig.hitboxes[3];
            hitboxesJson["Hitboxes"][4] = hitboxesConfig.hitboxes[4];
        }
    }

    for (size_t i = 0; i < triggerbot.size(); i++) {
        auto& triggerbotJson = json["Triggerbot"][i];
        const auto& triggerbotConfig = triggerbot[i];

        triggerbotJson["Enabled"] = triggerbotConfig.enabled;
        triggerbotJson["On key"] = triggerbotConfig.onKey;
        triggerbotJson["Key"] = triggerbotConfig.key;
        triggerbotJson["Friendly fire"] = triggerbotConfig.friendlyFire;
        triggerbotJson["Scoped only"] = triggerbotConfig.scopedOnly;
        triggerbotJson["Ignore flash"] = triggerbotConfig.ignoreFlash;
        triggerbotJson["Ignore smoke"] = triggerbotConfig.ignoreSmoke;
        {
            auto& hitboxesJson = triggerbotJson["Hitboxes"];
            const auto& hitboxesConfig = triggerbotConfig;
            hitboxesJson["Hitboxes"][0] = hitboxesConfig.hitboxes[0];
            hitboxesJson["Hitboxes"][1] = hitboxesConfig.hitboxes[1];
            hitboxesJson["Hitboxes"][2] = hitboxesConfig.hitboxes[2];
            hitboxesJson["Hitboxes"][3] = hitboxesConfig.hitboxes[3];
            hitboxesJson["Hitboxes"][4] = hitboxesConfig.hitboxes[4];
        }
        triggerbotJson["Hitchance"] = triggerbotConfig.hitChance;
        triggerbotJson["Shot delay"] = triggerbotConfig.shotDelay;
        triggerbotJson["Min damage"] = triggerbotConfig.minDamage;
        triggerbotJson["Killshot"] = triggerbotConfig.killshot;
    }

    {
        auto& backtrackJson = json["Backtrack"];
        backtrackJson["Enabled"] = backtrack.enabled;
        backtrackJson["Ignore smoke"] = backtrack.ignoreSmoke;
        backtrackJson["Ignore flash"] = backtrack.ignoreFlash;
        backtrackJson["Time limit"] = backtrack.timeLimit;
        backtrackJson["Fake Latency"] = backtrack.fakeLatency;
        backtrackJson["Latency Amount"] = backtrack.fakeLatencyAmount;
    }

    {
        auto& antiAimJson = json["Anti aim"];
        antiAimJson["Enabled"] = antiAim.enabled;
        antiAimJson["Mode"] = antiAim.mode;
        antiAimJson["LBY delta"] = antiAim.lbyDelta;
        antiAimJson["Pitch angle"] = antiAim.pitchAngle;
        antiAimJson["Yaw angle"] = antiAim.yawAngle;
        antiAimJson["Jitter range"] = antiAim.jitterRange;
        antiAimJson["LBY sway"] = antiAim.lbySway;
        antiAimJson["Invert"] = antiAim.invert;
        antiAimJson["Auto direction"] = antiAim.autodir;
        antiAimJson["Fake Lag"] = antiAim.fakeLag;
        antiAimJson["Fake Lag Mode"] = antiAim.fakeLagMode;
        antiAimJson["Fake Lag Amount"] = antiAim.fakeLagAmount;
        antiAimJson["Fake duck key"] = antiAim.fakeDuckKey;
    }

    {
        auto& ragebotExtraJson = json["Ragebot extra"];
        ragebotExtraJson["Enabled"] = ragebotExtra.enabled;
        ragebotExtraJson["Doubletap"] = ragebotExtra.doubletap;
        ragebotExtraJson["Doubletap speed"] = ragebotExtra.doubletapSpeed;
        ragebotExtraJson["Resolver"] = ragebotExtra.resolver;
    }

    for (size_t i = 0; i < glow.size(); i++) {
        auto& glowJson = json["glow"][i];
        const auto& glowConfig = glow[i];

        glowJson["Enabled"] = glowConfig.enabled;
        glowJson["healthBased"] = glowConfig.healthBased;
        glowJson["style"] = glowConfig.style;

        {
            auto& colorJson = glowJson["Color"];
            const auto& colorConfig = glowConfig;

            colorJson["Color"][0] = colorConfig.color[0];
            colorJson["Color"][1] = colorConfig.color[1];
            colorJson["Color"][2] = colorConfig.color[2];
            colorJson["Color"][3] = colorConfig.color[3];

            colorJson["Rainbow"] = colorConfig.rainbow;
            colorJson["Rainbow speed"] = colorConfig.rainbowSpeed;
        }
    }

    for (size_t i = 0; i < chams.size(); i++) {
        auto& chamsJson = json["Chams"][i];
        const auto& chamsConfig = chams[i];

        for (size_t j = 0; j < chamsConfig.materials.size(); j++) {
            auto& materialsJson = chamsJson[j];
            const auto& materialsConfig = chams[i].materials[j];

            materialsJson["Enabled"] = materialsConfig.enabled;
            materialsJson["Health based"] = materialsConfig.healthBased;
            materialsJson["Blinking"] = materialsConfig.blinking;
            materialsJson["Material"] = materialsConfig.material;
            materialsJson["Wireframe"] = materialsConfig.wireframe;

            {
                auto& colorJson = materialsJson["Color"];
                const auto& colorConfig = materialsConfig; // leftover

                colorJson["Color"][0] = colorConfig.color[0];
                colorJson["Color"][1] = colorConfig.color[1];
                colorJson["Color"][2] = colorConfig.color[2];
                colorJson["Color"][3] = colorConfig.color[3];

                colorJson["Rainbow"] = colorConfig.rainbow;
                colorJson["Rainbow speed"] = colorConfig.rainbowSpeed;
            }
        }
    }

    for (size_t i = 0; i < esp.players.size(); i++) {
        auto& espJson = json["Esp"]["Players"][i];
        const auto& espConfig = esp.players[i];

        espJson["Enabled"] = espConfig.enabled;
        espJson["HP side"] = espConfig.hpside;
        espJson["Armor side"] = espConfig.armorside;

        {
            auto& snaplinesJson = espJson["Snaplines"];
            const auto& snaplinesConfig = espConfig.snaplines;

            snaplinesJson["Enabled"] = snaplinesConfig.enabled;
            snaplinesJson["Color"][0] = snaplinesConfig.color[0];
            snaplinesJson["Color"][1] = snaplinesConfig.color[1];
            snaplinesJson["Color"][2] = snaplinesConfig.color[2];
            snaplinesJson["Rainbow"] = snaplinesConfig.rainbow;
            snaplinesJson["Rainbow speed"] = snaplinesConfig.rainbowSpeed;
        }

        {
            auto& eyeTracesJson = espJson["Eye traces"];
            const auto& eyeTracesConfig = espConfig.eyeTraces;

            eyeTracesJson["Enabled"] = eyeTracesConfig.enabled;
            eyeTracesJson["Color"][0] = eyeTracesConfig.color[0];
            eyeTracesJson["Color"][1] = eyeTracesConfig.color[1];
            eyeTracesJson["Color"][2] = eyeTracesConfig.color[2];
            eyeTracesJson["Rainbow"] = eyeTracesConfig.rainbow;
            eyeTracesJson["Rainbow speed"] = eyeTracesConfig.rainbowSpeed;
        }

        {
            auto& boxJson = espJson["Box"];
            const auto& boxConfig = espConfig.box;

            boxJson["Enabled"] = boxConfig.enabled;
            boxJson["Color"][0] = boxConfig.color[0];
            boxJson["Color"][1] = boxConfig.color[1];
            boxJson["Color"][2] = boxConfig.color[2];
            boxJson["Rainbow"] = boxConfig.rainbow;
            boxJson["Rainbow speed"] = boxConfig.rainbowSpeed;
        }

        espJson["Box type"] = espConfig.boxType;

        {
            auto& nameJson = espJson["Name"];
            const auto& nameConfig = espConfig.name;

            nameJson["Enabled"] = nameConfig.enabled;
            nameJson["Color"][0] = nameConfig.color[0];
            nameJson["Color"][1] = nameConfig.color[1];
            nameJson["Color"][2] = nameConfig.color[2];
            nameJson["Rainbow"] = nameConfig.rainbow;
            nameJson["Rainbow speed"] = nameConfig.rainbowSpeed;
        }

        {
            auto& healthJson = espJson["Health"];
            const auto& healthConfig = espConfig.health;

            healthJson["Enabled"] = healthConfig.enabled;
            healthJson["Color"][0] = healthConfig.color[0];
            healthJson["Color"][1] = healthConfig.color[1];
            healthJson["Color"][2] = healthConfig.color[2];
            healthJson["Rainbow"] = healthConfig.rainbow;
            healthJson["Rainbow speed"] = healthConfig.rainbowSpeed;
        }

        espJson["Health bar"] = espConfig.healthBar;

        {
            auto& healthBarJson = espJson["Health bar color"];
            const auto& healthBarConfig = espConfig.healthBarColor;

            healthBarJson["Enabled"] = healthBarConfig.enabled;
            healthBarJson["Color"][0] = healthBarConfig.color[0];
            healthBarJson["Color"][1] = healthBarConfig.color[1];
            healthBarJson["Color"][2] = healthBarConfig.color[2];
            healthBarJson["Rainbow"] = healthBarConfig.rainbow;
            healthBarJson["Rainbow speed"] = healthBarConfig.rainbowSpeed;
        }

        {
            auto& armorJson = espJson["Armor"];
            const auto& armorConfig = espConfig.armor;

            armorJson["Enabled"] = armorConfig.enabled;
            armorJson["Color"][0] = armorConfig.color[0];
            armorJson["Color"][1] = armorConfig.color[1];
            armorJson["Color"][2] = armorConfig.color[2];
            armorJson["Rainbow"] = armorConfig.rainbow;
            armorJson["Rainbow speed"] = armorConfig.rainbowSpeed;
        }

        {
            auto& armorBarJson = espJson["Armor bar"];
            const auto& armorBarConfig = espConfig.armorBar;

            armorBarJson["Enabled"] = armorBarConfig.enabled;
            armorBarJson["Color"][0] = armorBarConfig.color[0];
            armorBarJson["Color"][1] = armorBarConfig.color[1];
            armorBarJson["Color"][2] = armorBarConfig.color[2];
            armorBarJson["Rainbow"] = armorBarConfig.rainbow;
            armorBarJson["Rainbow speed"] = armorBarConfig.rainbowSpeed;
        }

        {
            auto& moneyJson = espJson["Money"];
            const auto& moneyConfig = espConfig.money;

            moneyJson["Enabled"] = moneyConfig.enabled;
            moneyJson["Color"][0] = moneyConfig.color[0];
            moneyJson["Color"][1] = moneyConfig.color[1];
            moneyJson["Color"][2] = moneyConfig.color[2];
            moneyJson["Rainbow"] = moneyConfig.rainbow;
            moneyJson["Rainbow speed"] = moneyConfig.rainbowSpeed;
        }

        {
            auto& headDotJson = espJson["Head dot"];
            const auto& headDotConfig = espConfig.headDot;

            headDotJson["Enabled"] = headDotConfig.enabled;
            headDotJson["Color"][0] = headDotConfig.color[0];
            headDotJson["Color"][1] = headDotConfig.color[1];
            headDotJson["Color"][2] = headDotConfig.color[2];
            headDotJson["Rainbow"] = headDotConfig.rainbow;
            headDotJson["Rainbow speed"] = headDotConfig.rainbowSpeed;
        }

        {
            auto& activeWeaponJson = espJson["Active weapon"];
            const auto& activeWeaponConfig = espConfig.activeWeapon;

            activeWeaponJson["Enabled"] = activeWeaponConfig.enabled;
            activeWeaponJson["Color"][0] = activeWeaponConfig.color[0];
            activeWeaponJson["Color"][1] = activeWeaponConfig.color[1];
            activeWeaponJson["Color"][2] = activeWeaponConfig.color[2];
            activeWeaponJson["Rainbow"] = activeWeaponConfig.rainbow;
            activeWeaponJson["Rainbow speed"] = activeWeaponConfig.rainbowSpeed;
        }

        {
            auto& outlineJson = espJson["Outline"];
            const auto& outlineConfig = espConfig.outline;

            outlineJson["Enabled"] = outlineConfig.enabled;
            outlineJson["Color"][0] = outlineConfig.color[0];
            outlineJson["Color"][1] = outlineConfig.color[1];
            outlineJson["Color"][2] = outlineConfig.color[2];
            outlineJson["Rainbow"] = outlineConfig.rainbow;
            outlineJson["Rainbow speed"] = outlineConfig.rainbowSpeed;
        }
        {
            auto& ammoJson = espJson["Ammo"];
            const auto& ammoConfig = espConfig.ammo;

            ammoJson["Enabled"] = ammoConfig.enabled;
            ammoJson["Color"][0] = ammoConfig.color[0];
            ammoJson["Color"][1] = ammoConfig.color[1];
            ammoJson["Color"][2] = ammoConfig.color[2];
            ammoJson["Rainbow"] = ammoConfig.rainbow;
            ammoJson["Rainbow speed"] = ammoConfig.rainbowSpeed;
        }
        {
            auto& distanceJson = espJson["Distance"];
            const auto& distanceConfig = espConfig.distance;

            distanceJson["Enabled"] = distanceConfig.enabled;
            distanceJson["Color"][0] = distanceConfig.color[0];
            distanceJson["Color"][1] = distanceConfig.color[1];
            distanceJson["Color"][2] = distanceConfig.color[2];
            distanceJson["Rainbow"] = distanceConfig.rainbow;
            distanceJson["Rainbow speed"] = distanceConfig.rainbowSpeed;
        }

        espJson["Dead ESP"] = espConfig.deadesp;
        espJson["Max distance"] = espConfig.maxDistance;
    }

    {
        auto& espJson = json["Esp"]["Weapons"];
        const auto& espConfig = esp.weapon;

        espJson["Enabled"] = espConfig.enabled;
        {
            auto& snaplinesJson = espJson["Snaplines"];
            const auto& snaplinesConfig = espConfig.snaplines;

            snaplinesJson["Enabled"] = snaplinesConfig.enabled;
            snaplinesJson["Color"][0] = snaplinesConfig.color[0];
            snaplinesJson["Color"][1] = snaplinesConfig.color[1];
            snaplinesJson["Color"][2] = snaplinesConfig.color[2];
            snaplinesJson["Rainbow"] = snaplinesConfig.rainbow;
            snaplinesJson["Rainbow speed"] = snaplinesConfig.rainbowSpeed;
        }

        {
            auto& boxJson = espJson["Box"];
            const auto& boxConfig = espConfig.box;

            boxJson["Enabled"] = boxConfig.enabled;
            boxJson["Color"][0] = boxConfig.color[0];
            boxJson["Color"][1] = boxConfig.color[1];
            boxJson["Color"][2] = boxConfig.color[2];
            boxJson["Rainbow"] = boxConfig.rainbow;
            boxJson["Rainbow speed"] = boxConfig.rainbowSpeed;
        }

        espJson["Box type"] = espConfig.boxType;

        {
            auto& outlineJson = espJson["Outline"];
            const auto& outlineConfig = espConfig.outline;

            outlineJson["Enabled"] = outlineConfig.enabled;
            outlineJson["Color"][0] = outlineConfig.color[0];
            outlineJson["Color"][1] = outlineConfig.color[1];
            outlineJson["Color"][2] = outlineConfig.color[2];
            outlineJson["Rainbow"] = outlineConfig.rainbow;
            outlineJson["Rainbow speed"] = outlineConfig.rainbowSpeed;
        }

        {
            auto& nameJson = espJson["Name"];
            const auto& nameConfig = espConfig.name;

            nameJson["Enabled"] = nameConfig.enabled;
            nameJson["Color"][0] = nameConfig.color[0];
            nameJson["Color"][1] = nameConfig.color[1];
            nameJson["Color"][2] = nameConfig.color[2];
            nameJson["Rainbow"] = nameConfig.rainbow;
            nameJson["Rainbow speed"] = nameConfig.rainbowSpeed;
        }

        {
            auto& distanceJson = espJson["Distance"];
            const auto& distanceConfig = espConfig.distance;

            distanceJson["Enabled"] = distanceConfig.enabled;
            distanceJson["Color"][0] = distanceConfig.color[0];
            distanceJson["Color"][1] = distanceConfig.color[1];
            distanceJson["Color"][2] = distanceConfig.color[2];
            distanceJson["Rainbow"] = distanceConfig.rainbow;
            distanceJson["Rainbow speed"] = distanceConfig.rainbowSpeed;
        }

        espJson["Max distance"] = espConfig.maxDistance;
    }

    for (size_t i = 0; i < esp.dangerZone.size(); i++) {
        auto& espJson = json["Esp"]["Danger Zone"][i];
        const auto& espConfig = esp.dangerZone[i];

        espJson["Enabled"] = espConfig.enabled;
        {
            auto& snaplinesJson = espJson["Snaplines"];
            const auto& snaplinesConfig = espConfig.snaplines;

            snaplinesJson["Enabled"] = snaplinesConfig.enabled;
            snaplinesJson["Color"][0] = snaplinesConfig.color[0];
            snaplinesJson["Color"][1] = snaplinesConfig.color[1];
            snaplinesJson["Color"][2] = snaplinesConfig.color[2];
            snaplinesJson["Rainbow"] = snaplinesConfig.rainbow;
            snaplinesJson["Rainbow speed"] = snaplinesConfig.rainbowSpeed;
        }

        {
            auto& boxJson = espJson["Box"];
            const auto& boxConfig = espConfig.box;

            boxJson["Enabled"] = boxConfig.enabled;
            boxJson["Color"][0] = boxConfig.color[0];
            boxJson["Color"][1] = boxConfig.color[1];
            boxJson["Color"][2] = boxConfig.color[2];
            boxJson["Rainbow"] = boxConfig.rainbow;
            boxJson["Rainbow speed"] = boxConfig.rainbowSpeed;
        }

        espJson["Box type"] = espConfig.boxType;

        {
            auto& outlineJson = espJson["Outline"];
            const auto& outlineConfig = espConfig.outline;

            outlineJson["Enabled"] = outlineConfig.enabled;
            outlineJson["Color"][0] = outlineConfig.color[0];
            outlineJson["Color"][1] = outlineConfig.color[1];
            outlineJson["Color"][2] = outlineConfig.color[2];
            outlineJson["Rainbow"] = outlineConfig.rainbow;
            outlineJson["Rainbow speed"] = outlineConfig.rainbowSpeed;
        }

        {
            auto& nameJson = espJson["Name"];
            const auto& nameConfig = espConfig.name;

            nameJson["Enabled"] = nameConfig.enabled;
            nameJson["Color"][0] = nameConfig.color[0];
            nameJson["Color"][1] = nameConfig.color[1];
            nameJson["Color"][2] = nameConfig.color[2];
            nameJson["Rainbow"] = nameConfig.rainbow;
            nameJson["Rainbow speed"] = nameConfig.rainbowSpeed;
        }

        {
            auto& distanceJson = espJson["Distance"];
            const auto& distanceConfig = espConfig.distance;

            distanceJson["Enabled"] = distanceConfig.enabled;
            distanceJson["Color"][0] = distanceConfig.color[0];
            distanceJson["Color"][1] = distanceConfig.color[1];
            distanceJson["Color"][2] = distanceConfig.color[2];
            distanceJson["Rainbow"] = distanceConfig.rainbow;
            distanceJson["Rainbow speed"] = distanceConfig.rainbowSpeed;
        }

        espJson["Max distance"] = espConfig.maxDistance;
    }

    for (size_t i = 0; i < esp.projectiles.size(); i++) {
        auto& espJson = json["Esp"]["Projectiles"][i];
        const auto& espConfig = esp.projectiles[i];

        espJson["Enabled"] = espConfig.enabled;
        {
            auto& snaplinesJson = espJson["Snaplines"];
            const auto& snaplinesConfig = espConfig.snaplines;

            snaplinesJson["Enabled"] = snaplinesConfig.enabled;
            snaplinesJson["Color"][0] = snaplinesConfig.color[0];
            snaplinesJson["Color"][1] = snaplinesConfig.color[1];
            snaplinesJson["Color"][2] = snaplinesConfig.color[2];
            snaplinesJson["Rainbow"] = snaplinesConfig.rainbow;
            snaplinesJson["Rainbow speed"] = snaplinesConfig.rainbowSpeed;
        }

        {
            auto& boxJson = espJson["Box"];
            const auto& boxConfig = espConfig.box;

            boxJson["Enabled"] = boxConfig.enabled;
            boxJson["Color"][0] = boxConfig.color[0];
            boxJson["Color"][1] = boxConfig.color[1];
            boxJson["Color"][2] = boxConfig.color[2];
            boxJson["Rainbow"] = boxConfig.rainbow;
            boxJson["Rainbow speed"] = boxConfig.rainbowSpeed;
        }

        espJson["Box type"] = espConfig.boxType;

        {
            auto& outlineJson = espJson["Outline"];
            const auto& outlineConfig = espConfig.outline;

            outlineJson["Enabled"] = outlineConfig.enabled;
            outlineJson["Color"][0] = outlineConfig.color[0];
            outlineJson["Color"][1] = outlineConfig.color[1];
            outlineJson["Color"][2] = outlineConfig.color[2];
            outlineJson["Rainbow"] = outlineConfig.rainbow;
            outlineJson["Rainbow speed"] = outlineConfig.rainbowSpeed;
        }

        {
            auto& nameJson = espJson["Name"];
            const auto& nameConfig = espConfig.name;

            nameJson["Enabled"] = nameConfig.enabled;
            nameJson["Color"][0] = nameConfig.color[0];
            nameJson["Color"][1] = nameConfig.color[1];
            nameJson["Color"][2] = nameConfig.color[2];
            nameJson["Rainbow"] = nameConfig.rainbow;
            nameJson["Rainbow speed"] = nameConfig.rainbowSpeed;
        }

        {
            auto& distanceJson = espJson["Distance"];
            const auto& distanceConfig = espConfig.distance;

            distanceJson["Enabled"] = distanceConfig.enabled;
            distanceJson["Color"][0] = distanceConfig.color[0];
            distanceJson["Color"][1] = distanceConfig.color[1];
            distanceJson["Color"][2] = distanceConfig.color[2];
            distanceJson["Rainbow"] = distanceConfig.rainbow;
            distanceJson["Rainbow speed"] = distanceConfig.rainbowSpeed;
        }

        espJson["Max distance"] = espConfig.maxDistance;
    }

    {
        auto& visualsJson = json["visuals"];
        visualsJson["disablePostProcessing"] = visuals.disablePostProcessing;
        visualsJson["inverseRagdollGravity"] = visuals.inverseRagdollGravity;
        visualsJson["noFog"] = visuals.noFog;
        visualsJson["no3dSky"] = visuals.no3dSky;
        visualsJson["No aim punch"] = visuals.noAimPunch;
        visualsJson["No view punch"] = visuals.noViewPunch;
        visualsJson["noHands"] = visuals.noHands;
        visualsJson["noSleeves"] = visuals.noSleeves;
        visualsJson["noWeapons"] = visuals.noWeapons;
        visualsJson["noSmoke"] = visuals.noSmoke;
        visualsJson["noBlur"] = visuals.noBlur;
        visualsJson["noScopeOverlay"] = visuals.noScopeOverlay;
        visualsJson["noGrass"] = visuals.noGrass;
        visualsJson["noShadows"] = visuals.noShadows;
        visualsJson["wireframeSmoke"] = visuals.wireframeSmoke;
        visualsJson["thirdperson"] = visuals.thirdperson;
        visualsJson["thirdpersonKey"] = visuals.thirdpersonKey;
        visualsJson["thirdpersonDistance"] = visuals.thirdpersonDistance;
        visualsJson["viewmodelFov"] = visuals.viewmodelFov;
        visualsJson["Fov"] = visuals.fov;
        visualsJson["Scoped Fov"] = visuals.fovScoped;
        visualsJson["farZ"] = visuals.farZ;
        visualsJson["flashReduction"] = visuals.flashReduction;
        visualsJson["brightness"] = visuals.brightness;
        visualsJson["skybox"] = visuals.skybox;
        visualsJson["Nightmode"] = visuals.nightMode;
        {
            const auto& visualsConfig = visuals;
            auto& nightmodeJson = visualsJson["Nightmode override"];
            const auto& nightmodeConfig = visualsConfig.nightModeOverride;

            nightmodeJson["Enabled"] = nightmodeConfig.enabled;
            nightmodeJson["Color"][0] = nightmodeConfig.color[0];
            nightmodeJson["Color"][1] = nightmodeConfig.color[1];
            nightmodeJson["Color"][2] = nightmodeConfig.color[2];
            nightmodeJson["Rainbow"] = nightmodeConfig.rainbow;
            nightmodeJson["Rainbow speed"] = nightmodeConfig.rainbowSpeed;
        }
        visualsJson["Deagle spinner"] = visuals.deagleSpinner;
        visualsJson["Screen effect"] = visuals.screenEffect;
        visualsJson["Hit effect"] = visuals.hitEffect;
        visualsJson["Hit effect time"] = visuals.hitEffectTime;
        visualsJson["Hitmarker"] = visuals.hitMarker;
        visualsJson["Playermodel T"] = visuals.playerModelT;
        visualsJson["Playermodel CT"] = visuals.playerModelCT;

        {
            auto& cc = visualsJson["Color correction"];
            cc["Enabled"] = visuals.colorCorrection.enabled;
            cc["Blue"] = visuals.colorCorrection.blue;
            cc["Red"] = visuals.colorCorrection.red;
            cc["Mono"] = visuals.colorCorrection.mono;
            cc["Saturation"] = visuals.colorCorrection.saturation;
            cc["Ghost"] = visuals.colorCorrection.ghost;
            cc["Green"] = visuals.colorCorrection.green;
            cc["Yellow"] = visuals.colorCorrection.yellow;
        }

        visualsJson["Asus walls"] = visuals.asusWalls;
        visualsJson["Asus props"] = visuals.asusProps;
        visualsJson["Thirdperson transparency"] = visuals.thirdpersonTransparency;

        {
            auto& bulletbeamJson = visualsJson["Local bullet beams"];
            bulletbeamJson["Enabled"] = visuals.bulletTracersLocal.enabled;
            bulletbeamJson["Color"][0] = visuals.bulletTracersLocal.color[0];
            bulletbeamJson["Color"][1] = visuals.bulletTracersLocal.color[1];
            bulletbeamJson["Color"][2] = visuals.bulletTracersLocal.color[2];
            bulletbeamJson["Rainbow"] = visuals.bulletTracersLocal.rainbow;
            bulletbeamJson["Rainbow speed"] = visuals.bulletTracersLocal.rainbowSpeed;
        }

        {
            auto& bulletbeamJson = visualsJson["Teammate bullet beams"];
            bulletbeamJson["Enabled"] = visuals.bulletTracersAllies.enabled;
            bulletbeamJson["Color"][0] = visuals.bulletTracersAllies.color[0];
            bulletbeamJson["Color"][1] = visuals.bulletTracersAllies.color[1];
            bulletbeamJson["Color"][2] = visuals.bulletTracersAllies.color[2];
            bulletbeamJson["Rainbow"] = visuals.bulletTracersAllies.rainbow;
            bulletbeamJson["Rainbow speed"] = visuals.bulletTracersAllies.rainbowSpeed;
        }

        {
            auto& bulletbeamJson = visualsJson["Enemy bullet beams"];
            bulletbeamJson["Enabled"] = visuals.bulletTracersEnemy.enabled;
            bulletbeamJson["Color"][0] = visuals.bulletTracersEnemy.color[0];
            bulletbeamJson["Color"][1] = visuals.bulletTracersEnemy.color[1];
            bulletbeamJson["Color"][2] = visuals.bulletTracersEnemy.color[2];
            bulletbeamJson["Rainbow"] = visuals.bulletTracersEnemy.rainbow;
            bulletbeamJson["Rainbow speed"] = visuals.bulletTracersEnemy.rainbowSpeed;
        }

        visualsJson["AA lines"] = visuals.aaLines;
        visualsJson["Penetration Crosshair"] = visuals.PenetrationCrosshair;

    }

    for (size_t i = 0; i < skinChanger.size(); i++) {
        auto& skinChangerJson = json["skinChanger"][i];
        const auto& skinChangerConfig = skinChanger[i];

        skinChangerJson["Enabled"] = skinChangerConfig.enabled;
        skinChangerJson["definition_vector_index"] = skinChangerConfig.itemIdIndex;
        skinChangerJson["definition_index"] = skinChangerConfig.itemId;
        skinChangerJson["entity_quality_vector_index"] = skinChangerConfig.entity_quality_vector_index;
        skinChangerJson["entity_quality_index"] = skinChangerConfig.quality;
        skinChangerJson["paint_kit_vector_index"] = skinChangerConfig.paint_kit_vector_index;
        skinChangerJson["paint_kit_index"] = skinChangerConfig.paintKit;
        skinChangerJson["definition_override_vector_index"] = skinChangerConfig.definition_override_vector_index;
        skinChangerJson["definition_override_index"] = skinChangerConfig.definition_override_index;
        skinChangerJson["seed"] = skinChangerConfig.seed;
        skinChangerJson["stat_trak"] = skinChangerConfig.stat_trak;
        skinChangerJson["wear"] = skinChangerConfig.wear;
        skinChangerJson["custom_name"] = skinChangerConfig.custom_name;

        for (size_t j = 0; j < skinChangerConfig.stickers.size(); j++) {
            auto& stickerJson = skinChangerJson["stickers"][j];
            const auto& stickerConfig = skinChangerConfig.stickers[j];

            stickerJson["kit"] = stickerConfig.kit;
            stickerJson["kit_vector_index"] = stickerConfig.kit_vector_index;
            stickerJson["wear"] = stickerConfig.wear;
            stickerJson["scale"] = stickerConfig.scale;
            stickerJson["rotation"] = stickerConfig.rotation;
        }
    }

    {
        auto& soundJson = json["Sound"];

        soundJson["Chicken volume"] = sound.chickenVolume;

        for (size_t i = 0; i < sound.players.size(); i++) {
            auto& playerJson = soundJson["Players"][i];
            const auto& playerConfig = sound.players[i];

            playerJson["Master volume"] = playerConfig.masterVolume;
            playerJson["Headshot volume"] = playerConfig.headshotVolume;
            playerJson["Weapon volume"] = playerConfig.weaponVolume;
            playerJson["Footstep volume"] = playerConfig.footstepVolume;
        }
    }

    {
        auto& miscJson = json["Misc"];

        miscJson["Menu key"] = misc.menuKey;
        miscJson["Anti AFK kick"] = misc.antiAfkKick;
        miscJson["Auto strafe"] = misc.autoStrafe;
        miscJson["Bunny hop"] = misc.bunnyHop;
        miscJson["Clantag"] = misc.clanTag;
        miscJson["Fast duck"] = misc.fastDuck;
        miscJson["Moonwalk"] = misc.moonwalk;
        miscJson["Edge Jump"] = misc.edgejump;
        miscJson["Edge Jump Key"] = misc.edgejumpkey;
        miscJson["Quick peek"] = misc.quickpeek;
        miscJson["Quick peek key"] = misc.quickpeekkey;
        miscJson["Slowwalk"] = misc.slowwalk;
        miscJson["Slowwalk key"] = misc.slowwalkKey;
        miscJson["Fast Stop"] = misc.fastStop;
        miscJson["Sniper crosshair"] = misc.sniperCrosshair;
        miscJson["Recoil crosshair"] = misc.recoilCrosshair;
        miscJson["Auto pistol"] = misc.autoPistol;
        miscJson["Auto accept"] = misc.autoAccept;
        miscJson["Radar hack"] = misc.radarHack;
        miscJson["Reveal ranks"] = misc.revealRanks;
        miscJson["Reveal money"] = misc.revealMoney;
        miscJson["Reveal suspect"] = misc.revealSuspect;
        miscJson["Debug Window"] = misc.debugwindow;
        miscJson["Spec list"] = misc.spectatorList;
        miscJson["Water mark"] = misc.watermark;
        miscJson["Disable model occlusion"] = misc.disableModelOcclusion;
        miscJson["Aspect Ratio"] = misc.aspectratio;
        miscJson["Killsay"] = misc.killSay;
        miscJson["Chicken deathsay"] = misc.chickenDeathSay;
        miscJson["Name stealer"] = misc.nameStealer;
        miscJson["Disable HUD blur"] = misc.disablePanoramablur;
        miscJson["Ban color"] = misc.banColor;
        miscJson["Ban text"] = misc.banText;
        miscJson["Fast plant"] = misc.fastPlant;

        {
            auto& bombTimerJson = miscJson["Bomb timer"];
            bombTimerJson["Enabled"] = misc.bombTimer.enabled;
            bombTimerJson["Color"][0] = misc.bombTimer.color[0];
            bombTimerJson["Color"][1] = misc.bombTimer.color[1];
            bombTimerJson["Color"][2] = misc.bombTimer.color[2];
            bombTimerJson["Rainbow"] = misc.bombTimer.rainbow;
            bombTimerJson["Rainbow speed"] = misc.bombTimer.rainbowSpeed;
        }

        miscJson["Prepare revolver"] = misc.prepareRevolver;
        miscJson["Prepare revolver key"] = misc.prepareRevolverKey;
        miscJson["Hit sound"] = misc.hitSound;
        miscJson["Grenade predict"] = misc.nadePredict;
        miscJson["Grenade trajectory"] = misc.nadeTrajectory;
        miscJson["Show impacts"] = misc.showImpacts;
        miscJson["Max angle delta"] = misc.maxAngleDelta;
        miscJson["Fake prime"] = misc.fakePrime;
        miscJson["Custom Hit Sound"] = misc.customHitSound;
        miscJson["Kill sound"] = misc.killSound;
        miscJson["Custom Kill Sound"] = misc.customKillSound;
        miscJson["Opposite Hand Knife"] = misc.oppositeHandKnife;

        {
            auto& purchaseListJson = miscJson["Purchase List"];
            purchaseListJson["Enabled"] = misc.purchaseList.enabled;
            purchaseListJson["Only During Freeze Time"] = misc.purchaseList.onlyDuringFreezeTime;
            purchaseListJson["Show Prices"] = misc.purchaseList.showPrices;
            purchaseListJson["No Title Bar"] = misc.purchaseList.noTitleBar;
            purchaseListJson["Mode"] = misc.purchaseList.mode;
        }

        {
            auto& autoBuyJson = miscJson["Autobuy"];
            autoBuyJson["Enabled"] = misc.autoBuy.enabled;
            autoBuyJson["Primary weapon"] = misc.autoBuy.primaryWeapon;
            autoBuyJson["Secondary weapon"] = misc.autoBuy.secondaryWeapon;
            autoBuyJson["Armor"] = misc.autoBuy.armor;
            autoBuyJson["Utility"][0] = misc.autoBuy.utility[0];
            autoBuyJson["Utility"][1] = misc.autoBuy.utility[1];
            autoBuyJson["Grenades"][0] = misc.autoBuy.grenades[0];
            autoBuyJson["Grenades"][1] = misc.autoBuy.grenades[1];
            autoBuyJson["Grenades"][2] = misc.autoBuy.grenades[2];
            autoBuyJson["Grenades"][3] = misc.autoBuy.grenades[3];
            autoBuyJson["Grenades"][4] = misc.autoBuy.grenades[4];
        }

        miscJson["Sv pure bypass"] = misc.svpurebypass;
        miscJson["Unlock Inventory"] = misc.unlockInventory;
        miscJson["Preserve killfeed"] = misc.preserveDeathNotices;
        miscJson["Auto disconnect"] = misc.autoDisconnect;
    }

    std::error_code ec;
    std::filesystem::create_directory(path, ec);

    if (std::ofstream out{ path / (const char8_t*)configs[id].c_str() }; out.good())
        out << json;
}

void Config::add(const char* name) noexcept
{
    if (*name && std::find(configs.cbegin(), configs.cend(), name) == configs.cend()) {
        configs.emplace_back(name);
        save(configs.size() - 1);
    }
}

void Config::remove(size_t id) noexcept
{
    std::error_code ec;
    std::filesystem::remove(path / (const char8_t*)configs[id].c_str(), ec);
    configs.erase(configs.cbegin() + id);
}

void Config::rename(size_t item, const char* newName) noexcept
{
    std::error_code ec;
    std::filesystem::rename(path / (const char8_t*)configs[item].c_str(), path / (const char8_t*)newName, ec);
    configs[item] = newName;
}

void Config::reset() noexcept
{
    legitbot = { };
    ragebot = { };
    triggerbot = { };
    backtrack = { };
    glow = { };
    chams = { };
    esp = { };
    visuals = { };
    skinChanger = { };
    sound = { };
    misc = { };
}

void Config::listConfigs() noexcept
{
    configs.clear();

    std::error_code ec;
    std::transform(std::filesystem::directory_iterator{ path, ec },
                   std::filesystem::directory_iterator{ },
                   std::back_inserter(configs),
                   [](const auto& entry) { return std::string{ (const char*)entry.path().filename().u8string().c_str() }; });
}
