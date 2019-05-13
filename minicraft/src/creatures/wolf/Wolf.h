#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT = 4;
#define WOLF_SPEED = 0.1f;
#define WOLF_SATIATION_DELAY = 0.01f;
#define WOLF_REPRODUCTION_THRESHOLD = 0.8f;
#define WOLF_SIGHT_RANGE = 15;
#define WOLF_EAT_GAIN = 0.3f;

class Wolf : public AICreature {
	#pragma region States
	struct WolfState : public State {
		Wolf* wolf;
		WolfState(AICreature* creature): State(creature), wolf((Wolf*) creature) {}
	};

	struct IdleState:public WolfState
	{
		IdleState(Wolf* wolf): WolfState(wolf) {}

		virtual void enter() {
			//initializePath
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed)) 
			{	//TO-DO: A CHANGER POUR POINTER VERS TRAP
				if (wolf->manager->perceptor->creatureSight(wolf, CreatureType::Bear, WOLF_SIGHT_RANGE) != nullptr) 
				{
					//TO-DO : FUIR
					return;
				}
			}
		}
	};
	#pragma endregion
};
