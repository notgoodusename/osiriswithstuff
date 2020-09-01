#pragma once

#include <memory>
#include <string>

struct ImFont;

class GUI {
public:
    GUI() noexcept;
    void render() noexcept;
    bool open = true;
private:
    static void hotkey(int&) noexcept;
    void renderLegitbotWindow() noexcept;
	void renderRagebotWindow() noexcept;
    void renderAntiAimWindow() noexcept;
    void renderTriggerbotWindow() noexcept;
    void renderBacktrackWindow() noexcept;
    void renderGlowWindow() noexcept;
    void renderChamsWindow() noexcept;
    void renderEspWindow() noexcept;
    void renderVisualsWindow() noexcept;
    void renderSkinChangerWindow() noexcept;
    void renderSoundWindow() noexcept;
    void renderMiscWindow() noexcept;
    void renderConfigWindow() noexcept;
    void renderGui() noexcept;
    void renderNewGui() noexcept;

    struct {
        bool legitbot = false;
        bool ragebot = false;
        bool antiAim = false;
        bool triggerbot = false;
        bool backtrack = false;
        bool glow = false;
        bool chams = false;
        bool esp = false;
        bool visuals = false;
        bool skinChanger = false;
        bool sound = false;
        bool style = false;
        bool misc = false;
        bool config = false;
    } window;

    struct {
        ImFont* tahoma = nullptr;
        ImFont* tahoma34 = nullptr;
        ImFont* segoeui = nullptr;
    } fonts;
};

inline std::unique_ptr<GUI> gui;
