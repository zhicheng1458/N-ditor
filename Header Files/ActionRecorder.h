#pragma once

#include "Action.h"
#include <list>

class ActionRecorder
{
	public:
		bool newAction(const Modification& changes);
		bool checkHasChanges(const Modification& changes);
		bool undo(Modification& changes);
		bool redo(Modification& changes);
		bool hasUnsavedWork();
		//void flagCurrentStep(); //For override confirmation.
		void flagCurrentStepSaved();
		//bool changedSinceLastChecked();
		bool reset();

	private:
		std::list<Modification> recorder;
		std::list<Modification>::iterator currentStep = recorder.end();
		std::list<Modification>::iterator savedStep = recorder.begin();
		//std::list<Modification>::iterator flaggedStep = currentStep;
		const int MAX_NUM_ACTIONS_RECORDED = 30;
};