#ifndef __CREATURE__
#define __CREATURE__

#include "world.h"
#include "AStar.h"

class Creature
{
public:
	YVec3f position;

	Creature(string name, MWorld * world, YVec3f pos, bool canFly, float speed) :
		name(name), world(world), position(pos), canFly(canFly), timeBetweenMoves(speed)
	{

	}

	void update(float elapsed)
	{
		if (!(position == targetPos) && pathToTarget.size() > 0)
		{
			timeSinceLastMove += elapsed;
			if (timeSinceLastMove >= timeBetweenMoves)
			{
				timeSinceLastMove = 0;
				position = pathToTarget[currentMoveIndex];
				currentMoveIndex++;
			}
		}
		else
			goTo(getRandomTarget());
	}

	void goTo(YVec3f targetPos)
	{
		this->targetPos = targetPos;
		pathToTarget = AStar::findpath(position, targetPos, world, canFly);
		currentMoveIndex = 0;
		if (pathToTarget.size() == 0)
			YLog::log(YLog::USER_ERROR, "No path to target position");
	}

	void startWandering()
	{
		goTo(getRandomTarget());
	}

private:
	string name;
	MWorld * world;
	YVec3f targetPos;
	vector<YVec3f> pathToTarget;
	int currentMoveIndex = 0;
	float timeSinceLastMove = 0;
	float timeBetweenMoves;

	bool canFly;

	YVec3f getRandomTarget()
	{
		int x = rand() % MWorld::MAT_SIZE_CUBES;
		int y = rand() % MWorld::MAT_SIZE_CUBES;
		int height = world->getSurface(x, y);
		int z = height;
		if (canFly)
		{
			z = rand() % (MWorld::MAT_HEIGHT_CUBES - height);
			z += height;
		}

		YVec3f target = YVec3f(x, y, z);
		YLog::log(YLog::USER_INFO, ("[" + name + "]: Going to (" + toString(x) + ", " + toString(y) + ", " + toString(z) + ")").c_str());

		return target;
	}
};

#endif
