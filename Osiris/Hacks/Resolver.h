#pragma once

#include "../SDK/Entity.h"
#include "../SDK/Vector.h"

#include <array>
#include <deque>

namespace Resolver
{
	float CalculateFeet(Entity* entity) noexcept;
	void UpdateShots(UserCmd*) noexcept;
	void Update(GameEvent*) noexcept;
	struct Tick
	{
		matrix3x4 matrix[256];
		Vector origin;
		Vector mins;
		Vector max;
		float time;
	};
	struct Info
	{
		Info() : misses(0), hit(false){ }
		int misses;
		bool hit;
	};
	struct Ticks
	{
		Vector position;
		float time;
	};
	Resolver::Ticks getClosest(const float time) noexcept;
	extern std::array<Info, 65> player;
	extern std::deque<Ticks> bulletImpacts;
	extern std::deque<Ticks> tick;
	extern std::deque<Tick> shot[65];
}