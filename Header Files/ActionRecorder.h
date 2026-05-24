#pragma once

#include "Action.h"
#include <list>

class ActionRecorder
{
	public:
		bool newAction(const std::vector<TileData> & tiles, const std::vector<EntityData>& entities, Action action);
		bool newAction(const Modification& changes);
		bool getPreviousAction(std::vector<TileData> & tiles, std::vector<EntityData>& entities);
		bool getNextAction(std::vector<TileData>& tiles, std::vector<EntityData>& entities);
		bool reset();

	private:
		std::list<Modification> recorder;
		std::list<Modification>::iterator currentStep = recorder.end();
		const int MAX_NUM_ACTIONS_RECORDED = 10;
};