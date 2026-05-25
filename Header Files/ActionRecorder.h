#pragma once

#include "Action.h"
#include <list>

class ActionRecorder
{
	public:
		bool newAction(const std::vector<TileData> & tiles, const std::vector<EntityData>& entities, Action action);
		bool newAction(const Modification& changes);
		bool checkHasChanges(const Modification& changes);
		bool undo(Modification& changes);
		bool redo(Modification& changes);
		bool reset();

	private:
		std::list<Modification> recorder;
		std::list<Modification>::iterator currentStep = recorder.end();
		const int MAX_NUM_ACTIONS_RECORDED = 30;
};