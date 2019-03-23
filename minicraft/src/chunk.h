#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class MChunk
{
	public :

		static const int CHUNK_SIZE = 64; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		MCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		YVbo * VboOpaque = NULL;
		YVbo * VboTransparent = NULL;

		MChunk * Voisins[6];

		int _XPos, _YPos, _ZPos; ///< Position du chunk dans le monde

		MChunk(int x, int y, int z)
		{
			memset(Voisins, 0x00, sizeof(void*)* 6);
			_XPos = x;
			_YPos = y;
			_ZPos = z;
		}

		/*
		Creation des VBO
		*/

		//On met le chunk dans son VBO
		void toVbos(void)
		{
			SAFEDELETE(VboOpaque);
			SAFEDELETE(VboTransparent);

			// Compte les vertices
			int nbVertOpaque = 0;
			int nbVertTransparent = 0;
			foreachVisibleTriangle(true, &nbVertOpaque, &nbVertTransparent, VboOpaque, VboTransparent);
			
			// Creation des VBOs avec le compte réalisé
			VboOpaque = new YVbo(4, nbVertOpaque, YVbo::PACK_BY_ELEMENT_TYPE);
			VboTransparent = new YVbo(4, nbVertTransparent, YVbo::PACK_BY_ELEMENT_TYPE);

			VboOpaque->setElementDescription(0, YVbo::Element(3)); //Vertex
			VboOpaque->setElementDescription(1, YVbo::Element(3)); //Normale
			VboOpaque->setElementDescription(2, YVbo::Element(2)); //UV
			VboOpaque->setElementDescription(3, YVbo::Element(1)); //Type

			VboTransparent->setElementDescription(0, YVbo::Element(3)); //Vertex
			VboTransparent->setElementDescription(1, YVbo::Element(3)); //Normale
			VboTransparent->setElementDescription(2, YVbo::Element(2)); //UV
			VboTransparent->setElementDescription(3, YVbo::Element(1)); //Type

			VboOpaque->createVboCpu();
			VboTransparent->createVboCpu();
																   
			// Remplissage des VBOs
			foreachVisibleTriangle(false, &nbVertOpaque, &nbVertTransparent, VboOpaque, VboTransparent);

			disableHiddenCubes();

			VboOpaque->createVboGpu();
			VboTransparent->createVboGpu();

			VboOpaque->deleteVboCpu();
			VboTransparent->deleteVboCpu();
		}

		// Ajout d'un quad au chunk, avec abcd les points dans l'ordre CCW
		int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d, float type)
		{
			YVec3f normal = (b - a).cross(d - a);
			normal.normalize();

			vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 0, 0);
			vbo->setElementValue(3, iVertice, type);

			iVertice++;

			vbo->setElementValue(0, iVertice, b.X, b.Y, b.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 1, 0);
			vbo->setElementValue(3, iVertice, type);


			iVertice++;

			vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 1, 1);
			vbo->setElementValue(3, iVertice, type);


			iVertice++;

			vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 0, 0);
			vbo->setElementValue(3, iVertice, type);


			iVertice++;

			vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 1, 1);
			vbo->setElementValue(3, iVertice, type);


			iVertice++;

			vbo->setElementValue(0, iVertice, d.X, d.Y, d.Z);
			vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
			vbo->setElementValue(2, iVertice, 0, 1);
			vbo->setElementValue(3, iVertice, type);

			iVertice++;

			return 6;
		}

		//Permet de compter les triangles ou des les ajouter aux VBO
		void foreachVisibleTriangle(bool countOnly, int * nbVertOpaque, int * nbVertTransp, YVbo * VboOpaque, YVbo * VboTransparent)
		{
			*nbVertOpaque = 0;
			*nbVertTransp = 0;
			MCube *cube = NULL;
			int type = 0;
			int iVerticeOpaque = 0;
			int iVerticeTransp = 0;
			for (int x = 0; x < CHUNK_SIZE; x++)
				for (int y = 0; y < CHUNK_SIZE; y++)
					for (int z = 0; z < CHUNK_SIZE; z++)
					{
						cube = &(_Cubes[x][y][z]);
						type = int(cube->getType());

						if (cube->getDraw() && (type != MCube::CUBE_AIR)) // Verifie que le cube est visible
						{
							// Recupere les cubes alentours
							MCube *cubeXPrev = NULL;
							MCube *cubeXNext = NULL;
							MCube *cubeYPrev = NULL;
							MCube *cubeYNext = NULL;
							MCube *cubeZPrev = NULL;
							MCube *cubeZNext = NULL;
							get_surrounding_cubes(x, y, z, &cubeXPrev, &cubeXNext, &cubeYPrev, &cubeYNext, &cubeZPrev, &cubeZNext);

							// Definition des positions
							YVec3f center(
								x * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f,
								y * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f,
								z * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f);
							YVec3f a(center.X + MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
							YVec3f b(center.X + MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
							YVec3f c(center.X + MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
							YVec3f d(center.X + MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
							YVec3f e(center.X - MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
							YVec3f f(center.X - MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
							YVec3f g(center.X - MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
							YVec3f h(center.X - MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);

							if (!cube->isTransparent()) // Opaque
							{
								if (cubeXPrev == NULL || cubeXPrev->isTransparent()) // X -
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, f, e, h, g, type);
								}

								if (cubeXNext == NULL || cubeXNext->isTransparent()) // X +
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, a, b, c, d, type);
								}

								if (cubeYPrev == NULL || cubeYPrev->isTransparent()) // Y -
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, e, a, d, h, type);
								}

								if (cubeYNext == NULL || cubeYNext->isTransparent()) // Y +
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, b, f, g, c, type);
								}

								if (cubeZPrev == NULL || cubeZPrev->isTransparent()) // Z -
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, e, f, b, a, type);
								}

								if (cubeZNext == NULL || cubeZNext->isTransparent()) // Z +
								{
									*nbVertOpaque += 6;
									if (!countOnly)
										iVerticeOpaque += addQuadToVbo(VboOpaque, iVerticeOpaque, c, g, h, d, type);
								}
							}
							else // Transparent
							{
								if (cubeXPrev == NULL || (cubeXPrev != NULL && cubeXPrev->getType() != type)) // X -
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, f, e, h, g, type);
								}

								if (cubeXNext == NULL || (cubeXNext != NULL && cubeXNext->getType() != cube->getType())) // X +
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, a, b, c, d, type);
								}

								if (cubeYPrev == NULL || (cubeYPrev != NULL && cubeYPrev->getType() != cube->getType())) // Y -
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, e, a, d, h, type);
								}

								if (cubeYNext == NULL || (cubeYNext != NULL && cubeYNext->getType() != cube->getType())) // Y +
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, b, f, g, c, type);
								}

								if (cubeZPrev == NULL || (cubeZPrev != NULL && cubeZPrev->getType() != cube->getType())) // Z -
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, e, f, b, a, type);
								}

								if (cubeZNext == NULL || (cubeZNext != NULL && cubeZNext->getType() != cube->getType())) // Z +
								{
									*nbVertTransp += 6;
									if (!countOnly)
										iVerticeTransp += addQuadToVbo(VboTransparent, iVerticeTransp, c, g, h, d, type);
								}
							}
						}

					}
		}

		/*
		Gestion du chunk
		*/

		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(false);
						_Cubes[x][y][z].setType(MCube::CUBE_AIR);
					}
		}

		void setVoisins(MChunk * xprev, MChunk * xnext, MChunk * yprev, MChunk * ynext, MChunk * zprev, MChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		void get_surrounding_cubes(int x, int y, int z, MCube ** cubeXPrev, MCube ** cubeXNext,
			MCube ** cubeYPrev, MCube ** cubeYNext,
			MCube ** cubeZPrev, MCube ** cubeZNext)
		{
			*cubeXPrev = NULL;
			*cubeXNext = NULL;
			*cubeYPrev = NULL;
			*cubeYNext = NULL;
			*cubeZPrev = NULL;
			*cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				*cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				*cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == CHUNK_SIZE - 1 && Voisins[1] != NULL)
				*cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < CHUNK_SIZE - 1)
				*cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				*cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE - 1][z]);
			else if (y > 0)
				*cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == CHUNK_SIZE - 1 && Voisins[3] != NULL)
				*cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < CHUNK_SIZE - 1)
				*cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				*cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE - 1]);
			else if (z > 0)
				*cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == CHUNK_SIZE - 1 && Voisins[5] != NULL)
				*cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < CHUNK_SIZE - 1)
				*cubeZNext = &(_Cubes[x][y][z + 1]);
		}

		void render(bool transparent)
		{
			if (transparent)
				VboTransparent->render();
			else
				VboOpaque->render();
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			MCube * cubeXPrev = NULL; 
			MCube * cubeXNext = NULL; 
			MCube * cubeYPrev = NULL; 
			MCube * cubeYNext = NULL; 
			MCube * cubeZPrev = NULL; 
			MCube * cubeZNext = NULL; 

			if(x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE-1][y][z]);
			else if(x > 0)
				cubeXPrev = &(_Cubes[x-1][y][z]);

			if(x == CHUNK_SIZE-1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if(x < CHUNK_SIZE-1)
				cubeXNext = &(_Cubes[x+1][y][z]);

			if(y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE-1][z]);
			else if(y > 0)
				cubeYPrev = &(_Cubes[x][y-1][z]);

			if(y == CHUNK_SIZE-1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if(y < CHUNK_SIZE-1)
				cubeYNext = &(_Cubes[x][y+1][z]);

			if(z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE-1]);
			else if(z > 0)
				cubeZPrev = &(_Cubes[x][y][z-1]);

			if(z == CHUNK_SIZE-1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if(z < CHUNK_SIZE-1)
				cubeZNext = &(_Cubes[x][y][z+1]);

			if( cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL )
				return false;

			if (cubeXPrev->isOpaque() == true && //droite
				cubeXNext->isOpaque() == true && //gauche
				cubeYPrev->isOpaque() == true && //haut
				cubeYNext->isOpaque() == true && //bas
				cubeZPrev->isOpaque() == true && //devant
				cubeZNext->isOpaque() == true)  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(true);
						if(test_hidden(x,y,z))
							_Cubes[x][y][z].setDraw(false);
					}
		}


};