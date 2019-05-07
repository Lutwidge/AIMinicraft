#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT 4
#define OCELOT_SPEED 0.2f
#define OCELOT_SATIATION_DECAY 0.02f
#define OCELOT_REPRODUCTION_THRESHOLD 0.8f

class Ocelot : public AICreature {
protected:
#pragma region States
	struct OcelotState : public State
	{
		Ocelot * ocelot;
		OcelotState(AICreature* creature) : State(creature), ocelot((Ocelot*)creature) {}
	};

	struct IdleState : public OcelotState {
		IdleState(Ocelot * ocelot) : OcelotState(ocelot) {

		}

		virtual void enter() {
			ocelot->ground();
			ocelot->initializeRandomPath();
		}

		virtual void update(float elapsed) {
			ocelot->ground();
			if (ocelot->hasNotReachedTarget()) {
				ocelot->move(elapsed);

				// Manger

			}
			else {
				ocelot->initializeRandomPath();
			}
		}

		virtual void exit() {

		}
	};

	struct EatState : public OcelotState
	{
		EatState(Ocelot * ocelot) : OcelotState(ocelot) {

		}

		virtual void enter() 
		{
			ocelot->gotToEatTarget();
		}

		virtual void update(float elapsed) {
		
		}

		virtual void exit() {

		}
	};

	struct ReproductionState : public OcelotState {
		ReproductionState(Ocelot* ocelot) : OcelotState(ocelot) {

		}

		virtual void enter() {
			// MeetingPoint pour les ocelots au sol 
			YVec3f meetingPoint = (ocelot->position + ((Ocelot*)ocelot->partner)->position) / 2;
			ocelot->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, ocelot->world->getSurface(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) {
			if (ocelot->updateSatiation(elapsed)) {

				if (ocelot->isPartnerValid()) {
					if (ocelot->hasNotReachedTarget()) {
						ocelot->move(elapsed);
					}
					//Si les ocelots sont arrivés au MeetingPoint
					else if (!ocelot->hasNotReachedTarget() && !ocelot->partner->hasNotReachedTarget()) {
						ocelot->reproduce();
						ocelot->resetPartner();
						ocelot->switchState(new IdleState(ocelot));
					}
				}
				else {
					ocelot->resetPartner();
					ocelot->switchState(new IdleState(ocelot));
				}
			}
		}

		virtual void exit() {

		}
	};

#pragma endregion

	virtual void setRandomPath() {
		int x = rand() % world->MAT_SIZE_METERS;
		int y = rand() % world->MAT_SIZE_METERS;
		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

public:

	Ocelot(MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature("Ocelot", world, cm, pos, false, OCELOT_SPEED, OCELOT_SATIATION_DECAY, OCELOT_REPRODUCTION_THRESHOLD) {
		switchState(new IdleState(this));
	}

	/* MOVEMENT */
	virtual void ground() {
		position.Z = world->getSurface(position.X, position.Y);
	}

	/* EATING */
	virtual bool isEatTargetValid() {
		return false;
	}

	virtual void eat() {}

	virtual bool setPartner(AICreature* newPartner) {
		Ocelot * ocelotPartner = nullptr;
		if (typeid(newPartner) == typeid(Ocelot*)) {
			ocelotPartner = (Ocelot*)newPartner; // Casting to Ocelot* to access protected members
		}
		// If both ocelots don't already have a partner
		if (partner == nullptr && ocelotPartner->partner == nullptr) {
			partner = ocelotPartner;
			ocelotPartner->partner = this;
			ocelotPartner->switchState(new ReproductionState(ocelotPartner));
			return true;
		}
		return false;
	}

	virtual void setEatTarget(AICreature * creature) {

	}

	virtual void initializeRandomPath() {
		setRandomPath();
	}

	virtual void incrementRandomPath() {

	}

	virtual bool isPartnerValid() {
		return false;
	}

	virtual void reproduce() {
		// Création d'un ocelot qui s'enregistre lui-même auprès du CreatureManager
		new Ocelot(world, manager, position);

		partner->resetPartner();
		partner->switchState(new IdleState((Ocelot *)partner)); // --> new IdleState(this) ?
	}
};