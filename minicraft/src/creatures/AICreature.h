#pragma once
#include <unordered_map>
#include "../world.h"
#include "../AStar.h"
#include "CreatureManager.h"

class AICreature {
public:
	#pragma region States
	struct State {
		State() = delete;
		State(const State&) = delete;
		State(AICreature* creature) : creature(creature) {}

		virtual void enter() = 0;
		virtual void update(float elapsed) = 0;
		virtual void exit() = 0;

	protected:
		AICreature* creature;
	};

	struct DeadState : public State {
		DeadState(AICreature* creature) : State(creature) {}

		virtual void enter() {
			// On donne � la cr�ature sa target finale
			creature->goToFinalTarget();
		}
		virtual void update(float elapsed) {
			// On fait tomber la cr�ature jusqu'au sol
			if (creature->hasNotReachedTarget())
				creature->move(elapsed);
		}
		virtual void exit() {

		}
	};
	#pragma endregion

	State* state;
	YVec3f position;

	AICreature(string name, MWorld *world, CreatureManager* manager, YVec3f pos, bool canFly, float speed, float decay, float reproThreshold) : 
		name(name), world(world), manager(manager), position(pos), canFly(canFly), timeBetweenMoves(speed), satiationDecay(decay), satiation(1.0f), reproductionThreshold(reproThreshold) {
		//switchState(initialState);
		manager->registerCreature(this);
	}

	~AICreature() {
		manager->unregisterCreature(this);
		delete state;
	}

	//// UPDATES ////
	virtual void update(float elapsed) {
		state->update(elapsed);
	}

	virtual bool updateSatiation(float elapsed) {
		satiation -= satiationDecay * elapsed;
		if (satiation <= 0.0f) {
			die();
			return false;
		}
		return true;
	}

	//// STATES ////
	void addPossibleState(std::string id, State* state) {
		
	}

	virtual void die() {
		switchState(new DeadState(this));
	}

	void switchState(State* state) {
		if (this->state != nullptr) {
			this->state->exit();
			delete this->state;
		}
		this->state = state;
		this->state->enter();
	}

	//// MOVEMENT ////
	virtual void goTo(YVec3f targetPos) {
		this->targetPos = targetPos;
		pathToTarget = AStar::findpath(position, targetPos, world, canFly);
		currentMoveIndex = 0;
		if (pathToTarget.size() == 0)
			YLog::log(YLog::USER_ERROR, "No path to target position");
	}

	virtual bool hasNotReachedTarget() {
		return (!(position == targetPos) && pathToTarget.size() > 0);
	}

	virtual void move(float elapsed) {
		timeSinceLastMove += elapsed;
		if (timeSinceLastMove >= timeBetweenMoves) {
			timeSinceLastMove = 0;
			position = pathToTarget[currentMoveIndex];
			currentMoveIndex++;
		}
	}

	void goToFinalTarget() {
		goTo(YVec3f(position.X, position.Y, world->getHighestPoint(position.X, position.Y)));
	}

	//// EATING ////
	virtual void setEatTarget(YVec3f target) {
		eatTarget = target;
	}

	virtual void gotToEatTarget() {
		goTo(eatTarget);
	}

	virtual bool isEatTargetValid() = 0;

	virtual void eat() = 0;

	//// REPRODUCTION ////
	virtual bool canReproduce() {
		return satiation >= reproductionThreshold;
	}

	virtual void resetPartner() {
		partner = nullptr;
		if (partner != nullptr) {
			partner->partner = nullptr;
		}
	}

	virtual bool setPartner(AICreature* newPartner) = 0;

	virtual bool isPartnerValid() = 0;

	virtual void reproduce() = 0;

protected:
	MWorld *world;
	CreatureManager* manager = nullptr;

	string name;
	std::unordered_map<std::string, State*> possibleStates;
	YVec3f targetPos;
	YVec3f eatTarget;
	vector<YVec3f> pathToTarget;
	int currentMoveIndex = 0;
	float timeSinceLastMove = 0;
	float timeBetweenMoves;
	float satiation = 1.0f;
	float satiationDecay = 0.02f;
	bool canFly;
	float reproductionThreshold = 0.8f;
	AICreature* partner = nullptr;
};