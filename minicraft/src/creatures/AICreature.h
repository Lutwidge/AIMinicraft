#pragma once
#include <unordered_map>
#include "../world.h"
#include "../AStar.h"
#include "CreatureManager.h"
#include "Perceptor.h"
#include "CreatureType.h"

class MEngineMinicraft;

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
			printf("%s: Dead\n", creature->name.c_str());
			// On donne à la créature sa target finale
			creature->goToFinalTarget();
		}
		virtual void update(float elapsed) {
			// On fait tomber la créature jusqu'au sol
			if (creature->hasNotReachedTarget())
				creature->move(elapsed);
		}
		virtual void exit() {

		}
	};
	#pragma endregion

	State* state;
	YVec3f position;
	YVec3f forward;
	MWorld* world;

	AICreature(string name, MWorld *world, CreatureManager* manager, YVec3f pos, bool canFly, float speed, float decay, float reproThreshold) : 
		name(name), world(world), manager(manager), position(pos), canFly(canFly), timeBetweenMoves(speed), satiationDecay(decay), satiation(0.5f), reproductionThreshold(reproThreshold) {
		//switchState(initialState);
		//manager->registerCreature(this);
		forward = YVec3f(1, 0, 0);
	}

	~AICreature() {
		manager->unregisterCreature(this);
		delete state;
	}

	virtual CreatureType* getType() = 0;

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

	virtual void render(MEngineMinicraft* engine);

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
			forward = (pathToTarget[currentMoveIndex] - position).normalize();
			position = pathToTarget[currentMoveIndex];
			currentMoveIndex++;
		}
	}

	void goToFinalTarget() {
		YVec3f finalPos = YVec3f(position.X, position.Y, world->getHighestPoint(position.X, position.Y));
		if (!(finalPos == position))
			goTo(finalPos);
	}

	//// EATING ////
	virtual bool setEatTarget(YVec3f target) {
		eatTarget = target;
		return true;
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
		if (partner != nullptr) {
			partner->partner = nullptr;
		}
		partner = nullptr;
	}

	virtual bool setPartner(AICreature* newPartner) = 0;

	virtual bool isPartnerValid() = 0;

	virtual void reproduce() = 0;

protected:
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
