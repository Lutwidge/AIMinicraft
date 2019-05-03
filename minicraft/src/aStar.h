#ifndef __ASTAR__
#define __ASTAR__

#include <math.h>
#include <vector>
#include <algorithm>
#include <utility>

#include "world.h"

class AStar
{
public:
	static vector<YVec3f> findpath(YVec3f startPos, YVec3f endPos, MWorld * world, bool canFly)
	{
		if (!isTargetValid(endPos, world, canFly))
			return vector<YVec3f>();

		vector<Node *> openList;
		vector<Node *> closedList;

		Node * startNode = new Node(startPos);
		openList.push_back(startNode);

		Node * currentNode;

		while (!openList.empty())
		{
			sort(openList.begin(), openList.end(), Node::compare);
			currentNode = openList[openList.size() - 1];

			if (currentNode->position == endPos)
				return retracePath(startNode, currentNode);

			openList.pop_back();
			closedList.push_back(currentNode);

			vector<YVec3f> neighbours = getNeighbours(world, currentNode, canFly);
			for (unsigned int i = 0; i < neighbours.size(); i++)
			{
				bool alreadyInClosedList = false;
				for (unsigned int j = 0; j < closedList.size(); j++)
				{
					if (closedList[j]->position == neighbours[i])
					{
						alreadyInClosedList = true;
						break;
					}
				}

				if (alreadyInClosedList)
					continue;

				Node * neighbour = new Node(neighbours[i], currentNode);
				neighbour->calculateHeuristic(endPos);

				vector<Node *>::iterator nodeIterator = find(openList.begin(), openList.end(), neighbour);
				if (nodeIterator == openList.end())
					openList.push_back(neighbour);
				else if ((*nodeIterator)->getTotalCost() > neighbour->getTotalCost())
				{
					(*nodeIterator)->cost = neighbour->cost + 1;
					(*nodeIterator)->heuristic = neighbour->heuristic;
					(*nodeIterator)->parent = currentNode;
				}
			}
		}

		return vector<YVec3f>();
	}

private:
	class Node
	{
	public:
		YVec3f position;
		Node * parent;
		float cost;
		float heuristic;

		Node() {}

		Node(YVec3f pos, Node * parent = 0) :
			position(pos), parent(parent)
		{
			cost = (parent == 0 ? 0 : parent->cost + 1);
		}

		void calculateHeuristic(YVec3f targetPos)
		{
			heuristic = manhattanDistance(targetPos);
		}

		float getTotalCost() const
		{
			return (cost + heuristic);
		}

		static bool compare(const Node * const a, const Node * const b)
		{
			return a->getTotalCost() > b->getTotalCost();
		}

	private:
		float manhattanDistance(YVec3f targetPos)
		{
			float x = abs(position.X - targetPos.X);
			float y = abs(position.Y - targetPos.Y);
			float z = abs(position.Z - targetPos.Z);

			return (x + y + z);
		}
	};
	
	static bool isTargetValid(YVec3f targetPos, MWorld * world, bool canFly)
	{
		return (world->getCube(targetPos.X, targetPos.Y, targetPos.Z)->getType() == MCube::CUBE_AIR &&
			(canFly || (targetPos.Z == 0 || world->getCube(targetPos.X, targetPos.Y, targetPos.Z - 1)->isSolid())));
	}

	static vector<YVec3f> getNeighbours(MWorld * world, Node * node, bool canFly)
	{
		vector<YVec3f> neighbours;
		pair<YVec3f, MCube *> neighbourPair[3][3][4];
		bool neighbourExists[3][3][4];

		for (int x = -1; x < 2; x++)
		{
			for (int y = -1; y < 2; y++)
			{
				for (int z = -2; z < 2; z++)
				{
					if ((x == -1 && node->position.X == 0) || (x == 1 && node->position.X == MWorld::MAT_SIZE_CUBES - 1) ||
						(y == -1 && node->position.Y == 0) || (y == 1 && node->position.Y == MWorld::MAT_SIZE_CUBES - 1) ||
						(z <= -1 && node->position.Z == 0) || (z == 1 && node->position.Z == MWorld::MAT_HEIGHT_CUBES - 1) ||
						(x == 0 && y == 0 && z == 0))
					{
						neighbourExists[x + 1][y + 1][z + 2] = false;
					}
					else
					{
						YVec3f newPosition = YVec3f(node->position.X + x, node->position.Y + y, node->position.Z + z);
						neighbourPair[x + 1][y + 1][z + 2] = make_pair(newPosition, world->getCube(newPosition.X, newPosition.Y, newPosition.Z));
					}						
				}
			}
		}

		pair<YVec3f, MCube *> pair, pairBelow, pairLowZ, pairSameZ, pairUpZ;
		bool pairExists, pairBelowExists, existsLowZ, existsSameZ, existsUpZ;
		int z0 = -1, z1 = 0, z2 = 1;
		for (int x = -1; x < 2; x++)
		{
			for (int y = -1; y < 2; y++)
			{
				if (x == 0 && y == 0 && !canFly)
					continue;

				for (int z = -1; z < 2; z++)
				{
					pairExists = neighbourExists[x + 1][y + 1][z + 2];
					pair = neighbourPair[x + 1][y + 1][z + 2];

					pairBelowExists = neighbourExists[x + 1][y + 1][z + 1];
					pairBelow = neighbourPair[x + 1][y + 1][z + 1];

					if (pairExists && pair.second->getType() == MCube::CUBE_AIR && (canFly || (pairBelowExists && pairBelow.second->isSolid())))
						neighbours.push_back(pair.first);
				}
			}
		}

		return neighbours;
	}

	static vector<YVec3f> retracePath(Node * startNode, Node * endNode)
	{
		vector<YVec3f> path;
		Node * currentNode = endNode;

		while (currentNode != startNode)
		{
			path.push_back(currentNode->position);
			currentNode = currentNode->parent;
		}

		reverse(path.begin(), path.end());
		return path;
	}
};
 
#endif
