#include "Entity.h"

void Entity::sanitizeImpossibleValue(EntityData* e)
{
	switch (e->type)
	{
		case NINJA:
		case MINE:
		case GOLD:
		case EXIT:
		case EXIT_SWITCH:
		case LOCKED_DOOR_SWITCH:
		case TRAO_DOOR_SWITCH:
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
			e->rotation = ENTITY_DEGREE_0;
			//e->mode = ENTITY_MODE_0;
			break;
		default:
			break;
	}
}