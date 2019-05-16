#pragma once

#include "../AICreature.h"
#include <typeinfo>
#include <type_traits>

#define SPEED 1
#define DECAY_SATIETY 0.02
#define REPRO_THRESHOLD 0.8
#define OWL_SIGHT_RANGE 30

class Snake;

class Owl : AICreature
{
public:
#pragma region States
	struct OwlState : State
	{
		Owl* owl;
		OwlState(Owl* owl) : State(creature), owl(owl) {}

		void enter()
		{

		}

		void update(float elapsed)
		{

		}

		void exit()
		{

		}
	};

	//Quitte l'idle quand il a reper� un serpent ou quand il meurt de fin
	struct IdleState : public OwlState
	{
		IdleState(Owl* owl, bool onBranch) : OwlState(owl) 
		{
			owl->isOnBranch = onBranch;
		}
		float timeBeforeSearch;
		float time;

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans idle");
			timeBeforeSearch = 2;
			time = timeBeforeSearch;
		}

		void update(float elapsed)
		{
			time -= elapsed;
			if (owl->updateSatiation(elapsed))
			{
				//Fuite de predateur prioritaire
				owl->predator = owl->manager->perceptor->creatureSight(owl, CreatureType::Griffin, OWL_SIGHT_RANGE);
				if (owl->predator != nullptr)
				{
					owl->switchState(new FleeState(owl, owl->predator->position));
					return;
				}

				//Priorit� recherche de bouffe
				if (owl->getSatiation() < 0.5f)
				{
					owl->switchState(new LookingForFoodState(owl));
					return;
				}
				else
				{
					//Puis va chercher a se reproduire
					if (owl->getSatiation() > 0.8f)
					{
						AICreature* crea = owl->manager->perceptor->creatureSight(owl, CreatureType::Owl, OWL_SIGHT_RANGE);
						if (crea != nullptr && crea->canReproduce())
						{
							owl->switchState(new ChaseReproState(owl, (Owl*)crea));
							return;
						}
					}
				}

				AICreature* crea = owl->manager->perceptor->creatureSight(owl, CreatureType::Snake, OWL_SIGHT_RANGE);
				if (crea != nullptr)
				{
					owl->target = crea;
					owl->switchState(new ChaseState(owl));
					return;
				}					

				//Puis va chercher a etre sur une branche
				if (!owl->isOnBranch)
				{
					owl->switchState(new LookingForTreeState(owl));
					return;
				}
			}
		}

		void exit()
		{
			owl->isOnBranch = false;
		}
	};

	struct LookingForFoodState : public OwlState
	{

		LookingForFoodState(Owl* owl) : OwlState(owl) {}
		YVec3f positionTarget;
		YVec3f originForward;

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre LFF");
			//conter anti boucle infinie
			int counter = 0;
			bool found = false;
			YVec3f hitPosition;
			originForward = owl->forward;
			while (!found && counter < 10)
			{
				YVec3f directionRandom(rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50);
				directionRandom = directionRandom.normalize();

				if (owl->position.X <= 2)
				{
					directionRandom.X = abs(directionRandom.X);
				}

				if (owl->position.X >= MWorld::MAT_SIZE - 2)
				{
					directionRandom.X = -abs(directionRandom.X);
				}

				if (owl->position.Y >= MWorld::MAT_SIZE - 2)
				{
					directionRandom.Y = -abs(directionRandom.Y);
				}

				if (owl->position.Y < 2)
				{
					directionRandom.Y = abs(directionRandom.Y);
				}

				//Aucune collision , on peut voler par l�
				if (!owl->manager->perceptor->raycast(owl->position, directionRandom,owl->sightRange, hitPosition))
				{
					found = true;
					positionTarget = owl->position + (directionRandom * owl->sightRange);
					owl->forward = directionRandom;
				}
				counter++;
			}

			if (!found)
			{
				owl->switchState(new IdleState(owl, false));
			}
		}

		void update(float elapsed)
		{
			AICreature* targetSnake = owl->manager->perceptor->creatureSight(owl, CreatureType::Snake, OWL_SIGHT_RANGE);
			owl->forward = owl->forward.rotate(YVec3f(0, 0, 1), 1);

			AICreature* creature = owl->manager->perceptor->creatureSight(owl, CreatureType::Griffin, owl->sightRange);
			if (creature != nullptr)
			{
				owl->switchState(new FleeState(owl, creature->position));
			}

			if (targetSnake != nullptr)
			{
				owl->setEatTarget(targetSnake->position);
				owl->target = targetSnake;
				owl->switchState(new ChaseState(owl));				
			}
			else
			{
				owl->position += originForward * owl->timeBetweenMoves * elapsed;

				//Point depass�
				if ((positionTarget - owl->position).getSize() < 0.5)
				{
					owl->switchState(new LookingForFoodState(owl));
				}
			}
		}
	};

	struct ChaseState : OwlState
	{
		ChaseState(Owl* owl) : OwlState(owl) {}

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans Chase");
		}

		void update(float elapsed)
		{
			if (owl->updateSatiation(elapsed))
			{
				YVec3f toSnake(owl->target->position - owl->position);
				YVec3f pos;

				AICreature* creature = owl->manager->perceptor->creatureSight(owl, CreatureType::Griffin, owl->sightRange);
				if (creature != nullptr)
				{
					owl->switchState(new FleeState(owl, creature->position));
				}

				//Rien ne gene la vue
				if (!owl->manager->perceptor->raycast(owl->position, toSnake, toSnake.getSize(), pos))
				{
					float mag = toSnake.getSize();
					owl->position += toSnake.normalize() * owl->timeBetweenMoves * elapsed;

					//Depass� (donc choppp�)
					if (mag < 0.5)
					{
						owl->eat();

						//Rassasi� , va chercher un arbre
						owl->switchState(new LookingForTreeState(owl));
					}
				}
				else
				{
					//Perdu de vue
					owl->switchState(new LookingForFoodState(owl));
				}
			}
		}

		void exit()
		{
			owl->target = nullptr;
		}
	};

	struct LookingForTreeState : OwlState
	{
		LookingForTreeState(Owl* owl) : OwlState(owl) {}
		YVec3f treePos;
		float timeBeforeRetry = 1.0;

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans tree research");
			int counter = 0;
			bool found = false;
			float range = 10;
			while (!found && counter < 20)
			{
				YVec3f directionRandom(rand() % 2 - 1, rand() % 2 - 1, rand() % 2 - 1);
				YVec3f pos;

				//Arbre vue 
				if (owl->manager->perceptor->blockSight(owl,MCube::MCubeType::CUBE_BRANCHES,owl->sightRange,pos))
				{
					treePos = pos;
					owl->switchState(new GoToTreeState(owl, owl->GetTreeAirBlock(treePos)));
					return;
				}
				counter++;
			}
		}
	};

	struct GoToTreeState : OwlState
	{
		GoToTreeState(Owl* owl, YVec3f target) : OwlState(owl) , treePos(target){}
		YVec3f treePos;

		void enter()
		{
			owl->forward = (treePos - owl->position).normalize();
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans gotree");
		}

		void update(float elapsed)
		{
			YVec3f toTarget = treePos - owl->position;
			owl->position += toTarget.normalize() * owl->timeBetweenMoves * elapsed;

			AICreature* crea = owl->manager->perceptor->creatureSight(owl, CreatureType::Snake, OWL_SIGHT_RANGE);
			if (crea != nullptr)
			{
				owl->target = crea;
				owl->switchState(new ChaseState(owl));
				return;
			}

			owl->predator = owl->manager->perceptor->creatureSight(owl, CreatureType::Griffin, OWL_SIGHT_RANGE);
			if (owl->predator != nullptr)
			{
				owl->switchState(new FleeState(owl,owl->predator->position));
				return;
			}

			//Depass� , on se repose sur l'arbre
			if ((treePos - owl->position).getSize() < 0.5)
			{
				owl->switchState(new IdleState(owl, true));
			}
		}
	};

	struct FleeState : OwlState
	{
		YVec3f predatorPos;
		YVec3f fleePosition;
		FleeState(Owl* owl, YVec3f predator) : OwlState(owl), predatorPos(predator){}

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans flee");
			YVec3f directionFlee = (owl->position - predatorPos).normalize();
			directionFlee.rotate(YVec3f(0, 0, 1), rand() % 20 - 10);

			int counter = 20;
			YVec3f pos;

			//Champ libre , on y va
			while (!owl->manager->perceptor->raycast(owl->position, directionFlee, 10, pos) && --counter > 0)
			{				
				directionFlee.rotate(YVec3f(0, 0, 1), rand() % 20 - 10);
				directionFlee.rotate(YVec3f(0, 1, 0), rand() % 20 - 10);
				directionFlee.rotate(YVec3f(1, 0, 0), rand() % 20 - 10);
			}

			if (counter)
			{
				fleePosition = owl->position + (directionFlee * 10);
				owl->forward = directionFlee;
			}
			else
			{
				owl->switchState(new IdleState(owl, true));
			}
		}	

		void update(float elapsed)
		{
			owl->position += owl->forward * owl->timeBetweenMoves * elapsed;
			if ((fleePosition - owl->position).getSize() < 0.5)
			{
				owl->switchState(new IdleState(owl, false));
			}
		}
	};

	struct ChaseReproState : OwlState
	{
		ChaseReproState(Owl * owl, Owl * target) : OwlState(owl)
		{
			potentialPartner = target;
		}
		Owl* potentialPartner;

		void enter()
		{
			//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans reroduction");
			owl->forward = (potentialPartner->position - owl->position).normalize();
		}

		void update(float elapsed)
		{
			YVec3f toPartner = potentialPartner->position - owl->position;

			if (owl->updateSatiation(elapsed))
			{
				AICreature * creature = owl->manager->perceptor->creatureSight(owl, CreatureType::Griffin, owl->sightRange);
				if (creature != nullptr)
				{
					owl->switchState(new FleeState(owl, creature->position));
					return;
				}
				YVec3f pos;

				//Rien ne gene la vue
				if (!owl->manager->perceptor->raycast(owl->position, toPartner, toPartner.getSize(), pos))
				{
					if (toPartner.getSize() < 1)
					{
						owl->switchState(new ReproState(owl, potentialPartner));
						return;
					}
					else
					{
						owl->position += toPartner.normalize() * owl->timeBetweenMoves * elapsed;
					}
				}
				else
				{
					//Perdu de vue
					owl->switchState(new LookingForTreeState(owl));
				}
			}
		}

		void exit()
		{
			owl->target = nullptr;
		}
	};

	struct ReproState : OwlState
	{
		float timerBeforeReproduce;
		Owl* part;
		ReproState(Owl* owl, Owl * partner) : OwlState(owl), part(partner)
		{
			//owl->setPartner(partner);
			owl->partner = part;
			//Delai aleatoire pour eviter la double reproduction
			timerBeforeReproduce = rand() % 3;
		}

		void update(float elapsed)
		{
			timerBeforeReproduce -= elapsed;
			if (timerBeforeReproduce < 0)
			{
				//Reproduction en soi
				owl->reproduce();
				part->switchState(new IdleState(part, true));
				owl->switchState(new IdleState(owl, true));
			}
		}

		void exit()
		{
			owl->resetPartner();
		}
	};

#pragma endregion

	Owl(string name, MWorld* world, CreatureManager* manager, YVec3f pos) :
		AICreature(name, world, manager, pos, true, SPEED, DECAY_SATIETY, REPRO_THRESHOLD)
	{
		manager->registerCreature(this);
		satiation = 1;
		sightRange = OWL_SIGHT_RANGE;
		forward = YVec3f(1, 0, 0);
		switchState(new IdleState(this, false));
	}

	~Owl()
	{
		manager->unregisterCreature(this);
		delete state;
	}

	bool isPartnerValid() override
	{
		return (partner != NULL && partner->canReproduce());
	}

	bool setPartner(AICreature * newPartner) override
	{
		//Already have a partner
		if (partner != nullptr)
			return false;

		Owl* owl = dynamic_cast<Owl*>(newPartner);
		if (owl != nullptr)
		{
			partner = owl;
			return partner->isPartnerValid();
		}
		//Not an owl
		return false;
	}

	bool canReproduce() override
	{
		return AICreature::canReproduce() && isOnBranch && partner == nullptr;
	}

	void eat() override
	{
		target->die();
		satiation += 0.5;
		target = nullptr;
	}

	void die()
	{
		//YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl est mort");
		AICreature::die();
	}

	void reproduce() override
	{
		YVec3f spawnPosition = (partner->position + position) / 2;
		new Owl("Owl enfant ", world, manager, spawnPosition);
		satiation -= 0.2;
	}

	bool isEatTargetValid() override
	{
		return true;
	}

	float getSatiation()
	{
		return satiation;
	}

	CreatureType* getType()
	{
		return CreatureType::Owl;
	}

	YVec3f GetTreeAirBlock(YVec3f input)
	{
		YVec3f cursor = input;
		while (world->getCube(cursor.X, cursor.Y, cursor.Z)->getType() != MCube::CUBE_AIR)
		{
			cursor.Z++;
		}
		return cursor;
	}

	protected :
		AICreature* target;
		float sightRange;
		bool isOnBranch;
};
