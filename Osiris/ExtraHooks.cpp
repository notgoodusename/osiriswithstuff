#include "ExtraHooks.h"
#include "SDK/ModelInfo.h"

typedef float quaternion[4];

extraHooks extraHook;

static void __fastcall DoExtraBoneProcessing(Entity* entity, uint32_t edx, StudioHdr* hdr, Vector* pos, quaternion* q, matrix3x4* matrix, void* bone_list, void* context) noexcept
{
	auto state = entity->getAnimstate();

	auto backup_onground = false;
	if (state)
	{
		backup_onground = state->OnGround;
		state->OnGround = false;
	}

	extraHook.player[entity->index()].vmt.getOriginal<void>(197, entity, hdr, pos, q, matrix, bone_list, context);

	if (state)
	{
		state->OnGround = backup_onground;
	}
}

static void __fastcall StandardBlendingRules(Entity* entity, uint32_t edx, StudioHdr* hdr, Vector* pos, quaternion* q, float curTime, int boneMask) noexcept
{
	auto original = extraHook.player[entity->index()].vmt.getOriginal<void, StudioHdr*, Vector*, quaternion*, float, int>(205, hdr, pos, q, curTime, boneMask);
	*entity->getEffects() |= 8;
	original(entity, hdr, pos, q, curTime, boneMask);
	*entity->getEffects() &= ~8;
}

void extraHooks::hookEntity(Entity* entity) noexcept
{
	int i = entity->index();
	player[i].vmt.init(entity);
	player[i].vmt.hookAt(197, DoExtraBoneProcessing);
	player[i].vmt.hookAt(205, StandardBlendingRules);
	player[i].isHooked = true;
}

void extraHooks::init()
{
	if (!interfaces->engine->isInGame())
	{
		for (size_t i = 0; i < player.size(); i++)
		{
			if (player[i].isHooked)
			{
				player[i].isHooked = false;
				player[i].entity = nullptr;
			}
		}
	}
	if (interfaces->engine->isInGame())
	{
		if (!localPlayer || !localPlayer->isAlive())
			return;

		for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
		{
			auto entity = interfaces->entityList->getEntity(i);
			if (!entity || !entity->isAlive() || !entity->isPlayer())
				continue;

			if (!player[i].isHooked || player[i].entity != entity)
			{
				player[i].entity = entity;
				hookEntity(entity);
			}
		}
	}
}

void extraHooks::restore()
{
	for (size_t i = 0; i < player.size(); i++)
	{
		if (player[i].isHooked)
		{
			player[i].vmt.restore();
			player[i].isHooked = false;
		}
	}
}