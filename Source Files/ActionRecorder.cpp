#include "ActionRecorder.h"

bool ActionRecorder::newAction(const Modification& changes)
{
	//Remove all stored action beyond the point of the current step (aka no more redoing)
	if (currentStep != recorder.end())
	{
		recorder.erase(currentStep, recorder.end());
	}

	recorder.push_back(changes);
	if (recorder.size() > MAX_NUM_ACTIONS_RECORDED)
	{
		recorder.pop_front();
	}
	currentStep = recorder.end();
	return true;
}

bool ActionRecorder::checkHasChanges(const Modification& changes)
{
	return(!changes.oldTiles.empty() || !changes.newTiles.empty() ||
		   !changes.oldSingleEntity.empty() || !changes.newSingleEntity.empty() ||
		   !changes.oldPairEntity.empty() || !changes.newPairEntity.empty());
}

bool ActionRecorder::undo(Modification& changes)
{
	if (currentStep == recorder.begin() || recorder.empty())
	{
		return false;
	}

	currentStep--;
	changes = *currentStep;

	return true;
}

bool ActionRecorder::redo(Modification& changes)
{
	if (currentStep == recorder.end() || recorder.empty())
	{
		return false;
	}

	changes = *currentStep;
	currentStep++;

	return true;
}

bool ActionRecorder::hasUnsavedWork()
{
	return(currentStep != recorder.begin());
}

void ActionRecorder::flagCurrentStep()
{
	flaggedStep = currentStep;
}

bool ActionRecorder::changedSinceLastChecked()
{
	return (flaggedStep != currentStep);
}

bool ActionRecorder::reset()
{
	recorder.clear();
	currentStep = recorder.end();
	flaggedStep = currentStep;
	return true;
}