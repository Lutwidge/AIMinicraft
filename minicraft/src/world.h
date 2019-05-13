#ifndef __WORLD_H__
#define __WORLD_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "octave_perlin.h"
#include <vector>

class MWorld
{
public:
	typedef uint8 MAxis;
	static const int AXIS_X = 0b00000001;
	static const int AXIS_Y = 0b00000010;
	static const int AXIS_Z = 0b00000100;

#ifdef _DEBUG
	static const int MAT_SIZE = 5; //en nombre de chunks
#else
	static const int MAT_SIZE = 5; //en nombre de chunks
#endif // DEBUG

	static const int MAT_HEIGHT = 1; //en nombre de chunks
	static const int MAT_SIZE_CUBES = (MAT_SIZE * MChunk::CHUNK_SIZE);
	static const int MAT_HEIGHT_CUBES = (MAT_HEIGHT * MChunk::CHUNK_SIZE);
	static const int MAT_SIZE_METERS = (MAT_SIZE * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE);
	static const int MAT_HEIGHT_METERS = (MAT_HEIGHT * MChunk::CHUNK_SIZE  * MCube::CUBE_SIZE);

	MChunk * Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];

	vector<YVec3<int>> fruitTargets;
	int heightMap[MAT_SIZE_CUBES][MAT_SIZE_CUBES];

	MWorld()
	{
		//On cr�e les chunks
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					Chunks[x][y][z] = new MChunk(x, y, z);

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					MChunk * cxPrev = NULL;
					if (x > 0)
						cxPrev = Chunks[x - 1][y][z];
					MChunk * cxNext = NULL;
					if (x < MAT_SIZE - 1)
						cxNext = Chunks[x + 1][y][z];

					MChunk * cyPrev = NULL;
					if (y > 0)
						cyPrev = Chunks[x][y - 1][z];
					MChunk * cyNext = NULL;
					if (y < MAT_SIZE - 1)
						cyNext = Chunks[x][y + 1][z];

					MChunk * czPrev = NULL;
					if (z > 0)
						czPrev = Chunks[x][y][z - 1];
					MChunk * czNext = NULL;
					if (z < MAT_HEIGHT - 1)
						czNext = Chunks[x][y][z + 1];

					Chunks[x][y][z]->setVoisins(cxPrev, cxNext, cyPrev, cyNext, czPrev, czNext);
				}
	}

	inline int getSurface(int x, int y)
	{
		return heightMap[x][y] + 1;
	}

	inline int getHighestPoint(int x, int y) {
		for (int i = 0; i < MAT_HEIGHT_CUBES; i++) {
			if (getCube(x, y, heightMap[x][y] + 1 + i)->getType() == MCube::CUBE_AIR) {
				return heightMap[x][y] + 1 + i;
			}
		}
		return MAT_HEIGHT_CUBES - 1;
	}

	inline YVec3f getNearestAirCube(int x, int y, int z) {
		MCube* cube = getCube(x - 1, y, z);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x - 1, y, z);
		cube = getCube(x + 1, y, z);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x + 1, y, z);
		cube = getCube(x, y - 1, z);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x, y - 1, z);
		cube = getCube(x, y + 1, z);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x, y + 1, z);
		cube = getCube(x, y, z - 1);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x, y, z - 1);
		cube = getCube(x, y, z + 1);
		if (cube->getType() == MCube::CUBE_AIR)
			return YVec3f(x, y, z + 1);
		return YVec3f(x, y, z);
	}

	inline MCube * getCube(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * MChunk::CHUNK_SIZE) x = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * MChunk::CHUNK_SIZE) y = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * MChunk::CHUNK_SIZE) z = (MAT_HEIGHT * MChunk::CHUNK_SIZE) - 1;

		return &(Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->_Cubes[x % MChunk::CHUNK_SIZE][y % MChunk::CHUNK_SIZE][z % MChunk::CHUNK_SIZE]);
	}

	inline MCube* getCube(YVec3<int> v) {
		return getCube(v.X, v.Y, v.Z);
	}

	void updateCube(int x, int y, int z)
	{
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (z < 0)
			z = 0;
		if (x >= MAT_SIZE * MChunk::CHUNK_SIZE)
			x = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * MChunk::CHUNK_SIZE)
			y = (MAT_SIZE * MChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * MChunk::CHUNK_SIZE)
			z = (MAT_HEIGHT * MChunk::CHUNK_SIZE) - 1;

		YLog::log(YLog::ENGINE_INFO, ("Update world VAO + VBO"));
		Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->disableHiddenCubes();
		Chunks[x / MChunk::CHUNK_SIZE][y / MChunk::CHUNK_SIZE][z / MChunk::CHUNK_SIZE]->toVbos();
	}

	void updateCube(YVec3<int> v) 
	{
		return updateCube(v.X, v.Y, v.Z);
	}

	void deleteCube(int x, int y, int z)
	{
		MCube * cube = getCube(x, y, z);
		cube->setType(MCube::CUBE_AIR);
		cube->setDraw(false);
		cube = getCube(x - 1, y, z);
		updateCube(x, y, z);
	}

	void init_world(int seed)
	{
		YLog::log(YLog::USER_INFO, (toString("Creation du monde seed ") + toString(seed)).c_str());

		srand(seed);

		// Reset du monde
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					Chunks[x][y][z]->reset();

		// Generation du monde (utilise du bruit octave & bruit combin� avec du perlin)
		// 0 : Definition des bruits qui seront n�cessaires
		YOctavePerlin * octaves[8] = { new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin(), new YOctavePerlin() };
		for (int i = 0; i < 8; i++)
		{
			octaves[i]->setFreq(0.02f * i * randf());
			octaves[i]->updateVecs();
		}

		// 1 : Definition du niveau de l'eau
		int waterLevel = MAT_HEIGHT_CUBES / 2;
		// 2 : Creation d'une height map
		printf("Generating height map \n");
		float zr = 0.8456f;
		float persistence = 0.01f;
		float mult = 1.3f;
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
			{
				float sample1 = octaves[0]->sample(x * mult, y * mult, zr, 8, persistence);
				float heightLow = (2 * octaves[1]->sample(x * mult + sample1, y, zr, 8, persistence) - 1) * 128 / 6 - 4;
				float sample2 = octaves[2]->sample(x * mult, y * mult, zr, 8, persistence);
				float heightHigh = (2 * octaves[3]->sample(x * mult + sample2, y * mult, zr, 8, persistence) - 1) * 128;

				float heightResult = 0.0f;
				if ((2 * octaves[4]->sample(x, y, zr, 6, persistence) - 1) * 32 / 8 > 0)
					heightResult = heightLow;
				else
					heightResult = max(heightLow, heightHigh);
				heightResult /= 2;

				if (heightResult < 0)
					heightResult *= 0.8f;

				heightMap[x][y] = int(heightResult + waterLevel);
			}
		// 3 : Creation des strates
		printf("Generating strata \n");
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
			{
				float dirtThickness = (2 * octaves[5]->sample(x, y, zr, 8, 0.5f) - 1) * 128 / 24 - 4;
				int dirtTransition = heightMap[x][y];
				float stoneTransition = dirtTransition + dirtThickness;

				for (int z = 0; z < MAT_HEIGHT_CUBES; z++)
				{
					int type = MCube::CUBE_AIR;
					if (z == 0)
						type = MCube::CUBE_BRIQUES;
					else if (z <= stoneTransition)
						type = MCube::CUBE_PIERRE;
					else if (z <= dirtTransition)
						type = MCube::CUBE_TERRE;

					getCube(x, y, z)->setType(MCube::MCubeType(type));
				}
			}
		// 4 : Caves
		int caveNumber = int(MAT_SIZE_CUBES * MAT_SIZE_CUBES * MAT_HEIGHT_CUBES / (8192 * 2));
		//printf("Generating caves \n");ZZ
		//for (int k = 0; k < caveNumber; k++)
		//{
		//	YVec3<int> cavePos = YVec3<int>(rand() % MAT_SIZE_CUBES, rand() % MAT_SIZE_CUBES, rand() % MAT_HEIGHT_CUBES);
		//	int caveLength = int(randf() * randf() * 200);
		//	// Definition de la direction des caves et de leur rayon
		//	float theta = randf() * 2 * M_PI;
		//	float deltaTheta = 0.0f;
		//	float phi = randf() * 2 * M_PI;
		//	float deltaPhi = 0.0f;
		//	float caveRadius = randf() * randf();
		//	for (int i = 0; i <= caveLength; i++)
		//	{
		//		cavePos.X += sin(theta) * cos(phi);
		//		cavePos.Y += cos(theta) * cos(phi);
		//		cavePos.Z += sin(phi);

		//		theta += deltaTheta * 0.2f;
		//		deltaTheta = (deltaTheta * 0.9f) + randf() - randf();
		//		phi = phi / 2.0f + deltaPhi / 4.0f;
		//		deltaPhi = (deltaPhi * 0.75f) + randf() - randf();

		//		if (randf() >= 0.1)
		//		{
		//			YVec3f centerPos = YVec3f(cavePos.X + (rand() % 4 - 2) * 0.2, cavePos.Y + (rand() % 4 - 2) * 0.2, cavePos.Z + (rand() % 4 - 2) * 0.2);

		//			float radius = (MAT_HEIGHT_CUBES - centerPos.Z) / MAT_HEIGHT_CUBES;
		//			radius = 1.2f + (radius * 3.5f + 1) * caveRadius;
		//			radius *= sin(i * M_PI / caveLength);
		//			radius *= MCube::CUBE_SIZE;

		//			fillOblateSpheroid(centerPos, radius, int(MCube::CUBE_AIR));
		//		}
		//	}
		//}
		// 5 : Eau
		MCube * cube;
		//printf("Generating water \n");
		//for (int x = 0; x < MAT_SIZE_CUBES; x++)
		//	for (int y = 0; y < MAT_SIZE_CUBES; y++)
		//		for (int z = waterLevel + 1; z > 0; z--)
		//		{
		//			cube = getCube(x, y, z);
		//			if (cube->getType() != MCube::CUBE_AIR)
		//				break;
		//			else
		//				cube->setType(MCube::CUBE_EAU);
		//		}
		//// 6 : Caves inond�es
		//caveNumber = int(MAT_SIZE_CUBES * MAT_SIZE_CUBES * MAT_HEIGHT_CUBES / (8192 * 20));
		//printf("Generating flooded caves \n");
		//for (int k = 0; k < caveNumber; k++)
		//{
		//	YVec3<int> cavePos = YVec3<int>(rand() % MAT_SIZE_CUBES, rand() % MAT_SIZE_CUBES, rand() % MAT_HEIGHT_CUBES);
		//	int caveLength = int(randf() * randf() * 200);
		//	// Definition de la direction des caves et de leur rayon
		//	float theta = randf() * 2 * M_PI;
		//	float deltaTheta = 0.0f;
		//	float phi = randf() * 2 * M_PI;
		//	float deltaPhi = 0.0f;
		//	float caveRadius = randf() * randf();
		//	for (int i = 0; i <= caveLength; i++)
		//	{
		//		cavePos.X += sin(theta) * cos(phi);
		//		cavePos.Y += cos(theta) * cos(phi);
		//		cavePos.Z += sin(phi);

		//		theta += deltaTheta * 0.2f;
		//		deltaTheta = (deltaTheta * 0.9f) + randf() - randf();
		//		phi = phi / 2.0f + deltaPhi / 4.0f;
		//		deltaPhi = (deltaPhi * 0.75f) + randf() - randf();

		//		if (randf() >= 0.1)
		//		{
		//			YVec3f centerPos = YVec3f(cavePos.X + (rand() % 4 - 2) * 0.2, cavePos.Y + (rand() % 4 - 2) * 0.2, cavePos.Z + (rand() % 4 - 2) * 0.2);

		//			float radius = (MAT_HEIGHT_CUBES - centerPos.Z) / MAT_HEIGHT_CUBES;
		//			radius = 1.2f + (radius * 3.5f + 1) * caveRadius;
		//			radius *= sin(i * M_PI / caveLength);

		//			fillOblateSpheroid(centerPos, radius, int(MCube::CUBE_EAU));
		//		}
		//	}
		//}
		// 7 : Surface
		bool sandChance;
		bool gravelChance;
		MCube * aboveCube;
		printf("Generating surface layer \n");
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
			{
				sandChance = (2 * octaves[6]->sample(x, y, zr, 8, persistence) - 1) * 128 > 8 ? true : false;
				gravelChance = (2 * octaves[7]->sample(x, y, zr, 8, persistence) - 1) * 128 > 12 ? true : false;

				int z = heightMap[x][y];
				cube = getCube(x, y, z);
				aboveCube = getCube(x, y, z + 1);

				if (aboveCube->getType() == MCube::CUBE_EAU && gravelChance)
					cube->setType(MCube::CUBE_LAINE_01);
				else if (aboveCube->getType() == MCube::CUBE_AIR)
					if (z <= waterLevel && sandChance)
						cube->setType(MCube::CUBE_SABLE_01);
					else
						cube->setType(MCube::CUBE_HERBE);
			}
		// 8 : Arbres
		int treeNumber = int(MAT_SIZE_CUBES * MAT_SIZE_CUBES * MAT_HEIGHT_CUBES / (2 * 32000));
		printf("Generating trees \n");
		for (int k = 0; k < treeNumber; k++)
		{
			YVec3<int> patchPos = YVec3<int>(rand() % MAT_SIZE_CUBES, rand() % MAT_SIZE_CUBES, 0);

			for (int i = 0; i < 20; i++)
			{
				YVec3<int> treePos = patchPos;

				for (int j = 0; j < 20; j++)
				{
					int addRand = rand() % 6 - rand() % 6;
					treePos = YVec3<int>(treePos.X + addRand, treePos.Y + addRand, 0);

					if (treePos.X < MAT_SIZE_CUBES && treePos.Y < MAT_SIZE_CUBES && randf() <= 0.15)
					{
						treePos.Z = heightMap[treePos.X][treePos.Y] + 1;
						int treeHeight = rand() % 3 + 6;

						if (isSpaceForTree(treePos, treeHeight) && treePos.Z >= waterLevel)
							growTree(treePos, treeHeight);
					}
				}
			}
		}

		// 9 : Fruit
		for (int i = 0; i < fruitTargets.size(); i++)
		{
			if (randf() > 0.95)
			{
				getCube(fruitTargets[i])->setType(MCube::CUBE_FRUIT);
			}
		}

		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
					Chunks[x][y][z]->disableHiddenCubes();

		add_world_to_vbo();
	}

	void respawnFruit()
	{
		MCube* cube;
		int randomFruit = rand() % fruitTargets.size();
		for (int i = 0; i < fruitTargets.size(); i++)
		{
			cube = getCube(fruitTargets[i]);

			if (randomFruit <= 0)
			{
				cube->setType(MCube::CUBE_FRUIT);
				updateCube(fruitTargets[i]);
				break;
			}
			else if (cube->getType() == MCube::CUBE_BRANCHES)
			{
				randomFruit--;
			}
		}
	}

	void fillOblateSpheroid(YVec3f centerPos, float radius, int type)
	{
		float dx;
		float dy;
		float dz;
		MCube * cube;
		YVec3<int> minBounds = YVec3<int>(int(floorf(centerPos.X - radius)), int(floorf(centerPos.Y - radius)), int(floorf(centerPos.Z - radius)));
		YVec3<int> maxBounds = YVec3<int>(int(floorf(centerPos.X + radius)), int(floorf(centerPos.Y + radius)), int(floorf(centerPos.Z + radius)));

		for (int x = minBounds.X; x <= maxBounds.X; x++)
			for (int y = minBounds.Y; y <= maxBounds.Y; y++)
				for (int z = minBounds.Z; z <= maxBounds.Z; z++)
				{
					cube = getCube(x, y, z);
					if (cube->getType() == MCube::CUBE_PIERRE)
					{
						dx = x - centerPos.X;
						dy = y - centerPos.Y;
						dz = z - centerPos.Z;

						if ((pow(dx, 2) + 2 * pow(dy, 2) + pow(dz, 2)) < pow(radius, 2))
						{
							cube->setType(MCube::MCubeType(type));
						}
					}
				}
	}

	bool isSpaceForTree(YVec3<int> treePos, int treeHeight)
	{
		// Verification pour le tronc
		MCube * cube;
		for (int x = treePos.X; x <= treePos.X; x++)
			for (int y = treePos.Y; y <= treePos.Y; y++)
			{
				MCube::MCubeType type = getCube(x, y, treePos.Z - 1)->getType();
				if (type == MCube::CUBE_AIR || type == MCube::CUBE_EAU)
					return false;
				else
				{
					for (int z = treePos.Z; z < treePos.Z + treeHeight - 3; z++)
					{
						if (getCube(x, y, z)->getType() != MCube::CUBE_AIR)
							return false;
					}
				}
			}

		// Verification pour les branches
		for (int x = treePos.X - 2; x <= treePos.X + 2; x++)
			for (int y = treePos.Y - 2; y <= treePos.Y + 2; y++)
				for (int z = treePos.Z + treeHeight - 3; z <= treePos.Z + treeHeight; z++)
				{
					if (getCube(x, y, z)->getType() != MCube::CUBE_AIR)
						return false;
				}


		return true;
	}

	void growTree(YVec3<int> treePos, int treeHeight)
	{
		// Construction du tronc
		for (int x = treePos.X; x <= treePos.X; x++)
			for (int y = treePos.Y; y <= treePos.Y; y++)
				for (int z = treePos.Z; z < treePos.Z + treeHeight - 3; z++)
				{
					getCube(x, y, z)->setType(MCube::CUBE_TRONC);
				}

		// Construction des branches
		for (int x = treePos.X - 2; x <= treePos.X + 2; x++)
			for (int y = treePos.Y - 2; y <= treePos.Y + 2; y++)
				for (int z = treePos.Z + treeHeight - 3; z <= treePos.Z + treeHeight; z++)
				{
					MCube* cube = getCube(x, y, z);
					cube->setType(MCube::CUBE_BRANCHES);

					if (x == treePos.X - 2 || x == treePos.X + 2 || y == treePos.Y - 2 || y == treePos.Y + 2 || z == treePos.Z + treeHeight)
						fruitTargets.push_back(YVec3<int>(x, y, z));
				}

		// Construction de la partie finale du tronc
		for (int x = treePos.X; x <= treePos.X; x++)
			for (int y = treePos.Y; y <= treePos.Y; y++)
				for (int z = treePos.Z + treeHeight - 3; z <= treePos.Z + treeHeight - 2; z++)
				{
					getCube(x, y, z)->setType(MCube::CUBE_TRONC);
				}
	}

	void add_world_to_vbo(void)
	{
		YLog::log(YLog::ENGINE_INFO, ("Initialize world VAO + VBO "));
		for (int x = 0; x<MAT_SIZE; x++)
			for (int y = 0; y<MAT_SIZE; y++)
				for (int z = 0; z<MAT_HEIGHT; z++)
				{
					Chunks[x][y][z]->toVbos();
				}
	}
	
	//Boites de collisions plus petites que deux cubes
	MAxis getMinCol(YVec3f pos, YVec3f dir, float width, float height, float & valueColMin, bool oneShot)
	{
		int x = (int)(pos.X / MCube::CUBE_SIZE);
		int y = (int)(pos.Y / MCube::CUBE_SIZE);
		int z = (int)(pos.Z / MCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / MCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / MCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / MCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / MCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / MCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / MCube::CUBE_SIZE);

		if (x < 0)	x = 0;
		if (y < 0)	y = 0;
		if (z < 0)	z = 0;

		if (xPrev < 0)	xPrev = 0;
		if (yPrev < 0)	yPrev = 0;
		if (zPrev < 0)	zPrev = 0;

		if (xNext < 0)	xNext = 0;
		if (yNext < 0)	yNext = 0;
		if (zNext < 0)	zNext = 0;

		if (x >= MAT_SIZE_CUBES)	x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)	y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)	z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)	xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)	yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)	zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)	xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)	yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)	zNext = MAT_HEIGHT_CUBES - 1;

		//On fait chaque axe
		MAxis axis = 0x00;
		valueColMin = oneShot ? 0.5f : 10000.0f;
		float seuil = 0.0000001f;
		float prodScalMin = 1.0f;
		if (dir.getSqrSize() > 1)
			dir.normalize();

		//On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				getCube(xPrev + 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((xPrev + 1) * MCube::CUBE_SIZE) - (pos.X - width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		//float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		//On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				getCube(xNext - 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (xNext * MCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_X;
					}
			}
		}

		//float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		//On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				getCube(xNext, yNext - 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (yNext * MCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		//float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		//On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				getCube(xNext, yPrev + 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((yPrev + 1) * MCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Y;
					}
			}
		}

		//On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				getCube(xNext, yNext, zNext - 1)->isSolid()) || !oneShot)
			{
				float depassement = (zNext * MCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		//On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				getCube(xNext, yNext, zPrev + 1)->isSolid()) || !oneShot)
			{
				float depassement = ((zPrev + 1) * MCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = AXIS_Z;
					}
			}
		}

		return axis;
	}
		
	void render_world_basic(GLuint shader, YVbo * vboCube) 
	{
		// Boucle for en 3 dimensions
		// Si air, rien, si terre, on le dessine
		for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
				for (int z = 0; z < MAT_HEIGHT_CUBES; z++)
				{
					MCube *cube = getCube(x, y, z);

					if (cube->getType() == MCube::CUBE_TERRE)
					{
						cube->setDraw(true);

						glPushMatrix();
						glUseProgram(shader);
						glTranslatef(x * MCube::CUBE_SIZE / 2.0f, y * MCube::CUBE_SIZE / 2.0f, z * MCube::CUBE_SIZE / 2.0f); // Translate the cube
						YRenderer::getInstance()->updateMatricesFromOgl();
						YRenderer::getInstance()->sendMatricesToShader(shader);
						GLuint var = glGetUniformLocation(shader, "cube_color");
						glUniform4f(var, 40.0f / 255.0f, 25.0f / 255.0f, 0.0f, 1.0f);
						vboCube->render();
						glPopMatrix();
					}
				}
	}

	void render_world_vbo(bool debug, bool doTransparent)
	{
		glDisable(GL_BLEND);

		//Dessiner les chunks opaques
		for (int x = 0; x < MAT_SIZE; x++)
			for (int y = 0; y < MAT_SIZE; y++)
				for (int z = 0; z < MAT_HEIGHT; z++)
				{
					glPushMatrix();
					glTranslatef(x * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE, y * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE, z * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE);
					YRenderer::getInstance()->updateMatricesFromOgl(); //Calcule toute les matrices � partir des deux matrices OGL
					YRenderer::getInstance()->sendMatricesToShader(YRenderer::getInstance()->CURRENT_SHADER); //Envoie les matrices au shader
					Chunks[x][y][z]->render(false);
					glPopMatrix();
				}
				
		glEnable(GL_BLEND);
		//Dessiner les chunks transparents
		if (doTransparent)
		{
			for (int x = 0; x < MAT_SIZE; x++)
				for (int y = 0; y < MAT_SIZE; y++)
					for (int z = 0; z < MAT_HEIGHT; z++)
					{
						glPushMatrix();
						glTranslatef(x * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE, y * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE, z * MChunk::CHUNK_SIZE * MCube::CUBE_SIZE);
						YRenderer::getInstance()->updateMatricesFromOgl(); //Calcule toute les matrices � partir des deux matrices OGL
						YRenderer::getInstance()->sendMatricesToShader(YRenderer::getInstance()->CURRENT_SHADER); //Envoie les matrices au shader
						Chunks[x][y][z]->render(true);
						glPopMatrix();
					}
		}
	}

	/**
	* Attention ce code n'est pas optimal, il est compr�hensible. Il existe de nombreuses
	* versions optimis�es de ce calcul.
	*/
	inline bool intersecDroitePlan(const YVec3f & debSegment, const  YVec3f & finSegment,
		const YVec3f & p1Plan, const YVec3f & p2Plan, const YVec3f & p3Plan,
		YVec3f & inter)
	{
		
		return true;
	}

	/**
	* Attention ce code n'est pas optimal, il est compr�hensible. Il existe de nombreuses
	* versions optimis�es de ce calcul. Il faut donner les points dans l'ordre (CW ou CCW)
	*/
	inline bool intersecDroiteCubeFace(const YVec3f & debSegment, const YVec3f & finSegment,
		const YVec3f & p1, const YVec3f & p2, const YVec3f & p3, const  YVec3f & p4,
		YVec3f & inter)
	{
		
		return false;
	}

	bool getRayCollision(const YVec3f & debSegment, const YVec3f & finSegment,
		YVec3f & inter,
		int &xCube, int&yCube, int&zCube)
	{
		
		return false;
	}

	/**
	* De meme cette fonction peut �tre grandement opitimis�e, on a priviligi� la clart�
	*/
	bool getRayCollisionWithCube(const YVec3f & debSegment, const YVec3f & finSegment,
		int x, int y, int z,
		YVec3f & inter)
	{

		return true;
	}
};



#endif