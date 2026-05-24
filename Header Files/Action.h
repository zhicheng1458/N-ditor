#pragma once

#include "Core.h"
#include "Tile.h"
#include "Entity.h"

enum Action
{
	NO_ACTION = 0,
	ACTION_ADD = 1,
	ACTION_DELETE = 2,
	ACTION_CHANGE = 3 //Move is the same as changing
};

struct Modification
{
	std::vector<TileData> oldTiles; //Only use for changing
	std::vector<TileData> newTiles;

	/* When undoing add action: Replace all item, but treat it as a new action and push to end. Move action back 1 step.
	 * When undoing delete action: Add all item, but treat it as a new action and push to end. Move action back 1 step.
	 *
	 * When undoing move action:
	 * Treat move action as delete (old location) action follow by add (new location) action.
	 * Undo add action first then undo delete action
	 */

	Action action = NO_ACTION; //Pretty much only relevant for entities
};
