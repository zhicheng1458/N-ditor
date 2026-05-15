#include "Entity.h"

void Entity::sanitizeImpossibleValue(EntityData& e)
{
	switch (e.type)
	{
		case NINJA:
		case MINE:
		case GOLD:
		case EXIT:
		case EXIT_SWITCH:
		case LOCKED_DOOR_SWITCH:
		case TRAP_DOOR_SWITCH:
		case FLOOR_GUARD:
		case BOUNCE_BLOCK:
		case ROCKET:
		case GAUSS:
		case TOGGLE_MINE:
		case EVIL_NINJA:
		case BOOST_PAD:
		case DEATHBALL:
		case EYEBAT:
		case SHOVE_THWUMP:
		case NONE:
			e.rotation = ENTITY_DEGREE_0;
			e.mode = TRACE_WALL_CLOCKWISE;
			break;
		case LOCKED_DOOR:
		case TRAP_DOOR:
		case BOUNCE_PAD:
		case ONE_WAY:
		case THWUMP:
			e.mode = TRACE_WALL_CLOCKWISE;
		default:
			break;
	}
}

bool Entity::isSame(const EntityData& data1, const EntityData& data2)
{
	switch (data1.type)
	{
		case NINJA:
		case MINE:
		case GOLD:
		case BOUNCE_BLOCK:
		case ROCKET:
		case GAUSS:
		case TOGGLE_MINE:
		case EVIL_NINJA:
		case BOOST_PAD:
		case DEATHBALL:
		case EYEBAT:
		case SHOVE_THWUMP:
		case NONE:
		{
			return (data1.type == data2.type &&
					data1.entityCoordx == data2.entityCoordx &&
					data1.entityCoordy == data2.entityCoordy);
		}
		case REGULAR_DOOR:
		case BOUNCE_PAD:
		case ONE_WAY:
		case THWUMP:
		{
			return (data1.type == data2.type &&
					data1.entityCoordx == data2.entityCoordx &&
					data1.entityCoordy == data2.entityCoordy &&
					data1.rotation == data2.rotation);
		}
		case FLOOR_GUARD:
		{
			return (data1.type == data2.type &&
					data1.entityCoordx == data2.entityCoordx &&
					data1.entityCoordy == data2.entityCoordy &&
					data1.mode == data2.mode);
		}
		case CHAINGUN:
		case LASER_DRONE:
		case REGULAR_DRONE:
		case CHASE_DRONE:
		case LASER_TURRET:
		case MICRO_DRONE:
		{
			return (data1.type == data2.type &&
					data1.entityCoordx == data2.entityCoordx &&
					data1.entityCoordy == data2.entityCoordy &&
					data1.rotation == data2.rotation &&
					data1.mode == data2.mode);
		}
		case EXIT:
		case EXIT_SWITCH:
		case LOCKED_DOOR:
		case LOCKED_DOOR_SWITCH:
		case TRAP_DOOR:
		case TRAP_DOOR_SWITCH:
		{
			if (data1.pair == nullptr) { return false; } //Invalid entity, entity creation failed somewhere....

			if (data2.pair == nullptr) { return false; }
			if (data1.type != data2.type && data1.type != data2.pair->type) { return false; }

			const EntityData& matchingTypeEntity = (data1.type == data2.type) ? data2 : *(data2.pair);

			return (data1.type == matchingTypeEntity.type &&
					data1.entityCoordx == matchingTypeEntity.entityCoordx &&
					data1.entityCoordy == matchingTypeEntity.entityCoordy &&
					data1.rotation == matchingTypeEntity.rotation &&
					data1.pair->type == matchingTypeEntity.pair->type &&
					data1.pair->entityCoordx == matchingTypeEntity.pair->entityCoordx &&
					data1.pair->entityCoordy == matchingTypeEntity.pair->entityCoordy &&
					data1.pair->rotation == matchingTypeEntity.pair->rotation);
		}
		default:
		{
			return (data1.type == data2.type &&
					data1.entityCoordx == data2.entityCoordx &&
					data1.entityCoordy == data2.entityCoordy &&
					data1.rotation == data2.rotation &&
					data1.mode == data2.mode);
		}
	}
}