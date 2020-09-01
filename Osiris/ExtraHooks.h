#pragma once
#include "Hooks.h"
#include "Hooks/VmtSwap.h"
#include "SDK/Entity.h"

#include <array>

struct Container {
	Container() : isHooked(false), entity(nullptr) { }

	VmtSwap vmt;
	bool isHooked;
	Entity* entity;
};

class extraHooks
{
public:
	std::array <Container, 65> player;
	void hookEntity(Entity* entity) noexcept;
	void init();
	void restore();
};

extern extraHooks extraHook;
