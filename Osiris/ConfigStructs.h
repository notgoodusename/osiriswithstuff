#pragma once

struct ColorA {
    std::array<float, 4> color{ 1.0f, 1.0f, 1.0f, 1.0f };
    bool rainbow = false;
    float rainbowSpeed = 0.6f;

    auto operator==(const ColorA& c) const
    {
        return color == c.color
            && rainbow == c.rainbow
            && rainbowSpeed == c.rainbowSpeed;
    }
};

struct PurchaseList {
    bool enabled = false;
    bool onlyDuringFreezeTime = false;
    bool showPrices = false;
    bool noTitleBar = false;

    enum Mode {
        Details = 0,
        Summary
    };
    int mode = Details;

    auto operator==(const PurchaseList& pl) const
    {
        return enabled == pl.enabled
            && onlyDuringFreezeTime == pl.onlyDuringFreezeTime
            && showPrices == pl.showPrices
            && noTitleBar == pl.noTitleBar
            && mode == pl.mode;
    }
};

struct AutoBuy {
    bool enabled = false;

    int primaryWeapon{ 0 };
    int secondaryWeapon{ 0 };
    int armor{ 0 };
    bool utility[2]{ false };
    bool grenades[5]{ false };
};
