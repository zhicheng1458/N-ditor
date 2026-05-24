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

bool Entity::isSame(const PairEntityData& data1, const PairEntityData& data2)
{
	if (data1.e1.type != data2.e1.type && data1.e1.type != data2.e2.type) { return false; }
	if (data1.e2.type != data2.e1.type && data1.e2.type != data2.e2.type) { return false; }

	const EntityData& e1MatchingTypeEntity = (data1.e1.type == data2.e1.type) ? data2.e1 : data2.e2;
	const EntityData& e2MatchingTypeEntity = (data1.e2.type == data2.e1.type) ? data2.e1 : data2.e2;

	return (data1.e1.type == e1MatchingTypeEntity.type &&
			data1.e1.entityCoordx == e1MatchingTypeEntity.entityCoordx &&
			data1.e1.entityCoordy == e1MatchingTypeEntity.entityCoordy &&
			data1.e1.rotation == e1MatchingTypeEntity.rotation &&
			data1.e2.type == e2MatchingTypeEntity.type &&
			data1.e2.entityCoordx == e2MatchingTypeEntity.entityCoordx &&
			data1.e2.entityCoordy == e2MatchingTypeEntity.entityCoordy &&
			data1.e2.rotation == e2MatchingTypeEntity.rotation);
}
