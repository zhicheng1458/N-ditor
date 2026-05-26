#pragma once

#include "Core.h"
#include "Tile.h"
#include "Entity.h"

struct Modification
{
	std::vector<TileData> oldTiles; //Always treated as changing for tiles
	std::vector<TileData> newTiles;

	std::vector<EntityData> oldSingleEntity;
	std::vector<EntityData> newSingleEntity;
	std::vector<PairEntityData> oldPairEntity;
	std::vector<PairEntityData> newPairEntity;

	/* When undoing add action: Replace all item, but treat it as a new action and push to end. Move action back 1 step.
	 * When undoing delete action: Add all item, but treat it as a new action and push to end. Move action back 1 step.
	 *
	 * When undoing move action:
	 * Treat move action as delete (old location) action follow by add (new location) action.
	 * Undo add action first then undo delete action
	 */
};
