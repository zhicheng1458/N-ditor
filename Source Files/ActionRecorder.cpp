#include "ActionRecorder.h"

bool ActionRecorder::newAction(const std::vector<TileData>& tiles, const std::vector<EntityData>& entities, Action action)
{
	if (currentStep != recorder.end())
	{
		//TODO: Discard all changes beyond this new action before appending the new action
	}

	Modification modification;
	modification.newTiles = tiles;
	//modification.newEntities = entities;
	modification.action = action;
	recorder.push_back(modification);
	currentStep = recorder.end();

	return true;
}

bool ActionRecorder::newAction(const Modification& changes)
{
	if (currentStep != recorder.end())
	{
		//TODO: Discard all changes beyond this new action before appending the new action
	}

	recorder.push_back(changes);
	if (recorder.size() > MAX_NUM_ACTIONS_RECORDED)
	{
		recorder.pop_front();
	}
	currentStep = recorder.end();
	return true;
}

bool ActionRecorder::getPreviousAction(std::vector<TileData>& tiles, std::vector<EntityData>& entities)
{
	if (currentStep == recorder.begin())
	{
		return false;
	}

	currentStep--;
	//tiles = currentStep->tiles;
	//entities = currentStep->entities;

	return true;
}

bool ActionRecorder::getNextAction(std::vector<TileData>& tiles, std::vector<EntityData>& entities)
{
	if (currentStep == recorder.end())
	{
		return false;
	}

	currentStep++;
	//tiles = currentStep->tiles;
	//entities = currentStep->entities;

	return true;
}

bool ActionRecorder::reset()
{
	recorder.clear();
	currentStep = recorder.end();
	return true;
}