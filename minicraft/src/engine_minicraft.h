#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"

#include "avatar.h"
#include "world.h"
//#include "my_physics.h"
#include "creatures/bird/bird.h"
#include "creatures/rat/rat.h"
#include "creatures/owl/Owl.h"
#include "creatures/wolf/Wolf.h"
#include "creatures/CreatureManager.h"

class MEngineMinicraft : public YEngine
{
private:
	YVbo *VboCube;
	GLuint ShaderCubeDebug = 0;
	GLuint ShaderCube = 0;
	GLuint SunShader = 0;
	GLuint ShaderWorld = 0;

	// Pour la rotation du soleil et de la lune
	YColor SkyColor;
	YColor SunColor;
	float sunAngle = 0.0f;
	bool increaseSunSpeed = false;
	float timeOffset = 0.0f;
	float circleRadius = 200.0f;
	YVec3f sunDirection;
	YVec3f sunPosition;
	YVec3f moonPosition;
	YVec3f unitVector1 = YVec3f(0, 1, 0);
	YVec3f unitVector2 = YVec3f(0, 0, 1);

	/* POUR LES MOUVEMENTS DE CAMERA */
	// Pour conna�tre les d�placements de souris
	int lastx = -1;
	int lasty = -1;
	// Pour camera observation
	bool hasSubjectiveView = false;
	float cameraRotationSpeed = 0.005f;
	// Modificateur ctrl
	bool isControlHeld = false;
	// Mouvement en vue fp/tp
	float speed = 50.0f;
	float xMovement = 0.0f;
	float yMovement = 0.0f;
	// Zoom
	float zoomSpeed = 500.0f;
	// Vue avec bouton molette
	bool hasWheelView = false;
	// Strafe
	float strafeSpeed = 0.05f;

	// Monde
	MWorld *World;

	// Types de camera
	enum CameraType
	{
		Observation = 0,
		FirstPerson,
		FirstPersonFly,
		ThirdPerson
	};
	int cameraTypeIndex = 0;

	// Avatar
	MAvatar *avatar;

	// Textures
	YTexFile * tex;
	YTexFile * flowTex;
	YTexFile * waterTex;

	// Creatures
	CreatureManager* cm;

public :
	//Gestion singleton
	static YEngine * getInstance()
	{
		if (Instance == NULL)
			Instance = new MEngineMinicraft();
		return Instance;
	}

	/*HANDLERS GENERAUX*/
	void loadShaders() {
		//Chargement du shader (dans loadShaders() pour etre li� � F5)
		ShaderCubeDebug = Renderer->createProgram("shaders/cube_debug");
		ShaderCube = Renderer->createProgram("shaders/cube");
		SunShader = Renderer->createProgram("shaders/sun");
		ShaderWorld = Renderer->createProgram("shaders/world");
	}

	void init() 
	{
		YLog::log(YLog::ENGINE_INFO,"Minicraft Started : initialisation");

		// Create the world
		World = new MWorld();
		World->init_world(16807);

		Renderer->setBackgroundColor(YColor(0.0f,0.0f,0.0f,1.0f));
		Renderer->Camera->setPosition(YVec3f(10, 10, 10));
		Renderer->Camera->setLookAt(YVec3f());

		//Creation du VBO cube
		// 36 points car mode implicite : chaque face a 2 triangles, donc 6 points, et 6 faces : 36 points
		VboCube = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		//D�finition du contenu du VBO
		VboCube->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboCube->setElementDescription(1, YVbo::Element(3)); //Normale
		VboCube->setElementDescription(2, YVbo::Element(2)); //UV

		//On demande d'allouer la m�moire cot� CPU
		VboCube->createVboCpu();

		// Fill the cube
		fillVBOCube(VboCube, 1.0f);

		//On envoie le contenu au GPU
		VboCube->createVboGpu();

		//On relache la m�moire CPU
		VboCube->deleteVboCpu();

		// Normaliser les vecteurs unitaires pour la rotation du soleil
		unitVector1.normalize();
		unitVector2.normalize();

		// Cr�ation de l'avatar
		avatar = new MAvatar(Renderer->Camera, World);

		// Textures
		tex = YTexManager::getInstance()->loadTexture("textures/TexCustom_0.png");

		// Initialisation des mesh par default pour tous les types de creatures
		CreatureType::initMeshes(VboCube, ShaderCubeDebug);

		// Spawn les premières créatures
		cm = new CreatureManager(World);
        new Rat( "Rat", World, cm, YVec3f( ( MWorld::MAT_SIZE_METERS ) / 2, ( MWorld::MAT_SIZE_METERS ) / 2, World->getSurface( ( MWorld::MAT_SIZE_METERS ) / 2, ( MWorld::MAT_SIZE_METERS ) / 2 ) ) );

		new Bird("Bird1", World, cm, YVec3f((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2, World->getSurface((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2) + 4));
		new Bird("Bird2", World, cm, YVec3f((MWorld::MAT_SIZE_METERS) / 2 + 5, (MWorld::MAT_SIZE_METERS) / 2, World->getSurface((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2) + 4));
		new Owl("Owl 1", World, cm, YVec3f(0, 0, World->getSurface(0, 0)));

		//new Wolf("Wolf", World, cm, YVec3f((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2, World->getSurface((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2)));
	}

	int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d)
	{
		YVec3f normal = (b - a).cross(d - a);
		normal.normalize();

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, b.X, b.Y, b.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, a.X, a.Y, a.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 0);

		iVertice++;

		vbo->setElementValue(0, iVertice, c.X, c.Y, c.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 1, 1);

		iVertice++;

		vbo->setElementValue(0, iVertice, d.X, d.Y, d.Z);
		vbo->setElementValue(1, iVertice, normal.X, normal.Y, normal.Z);
		vbo->setElementValue(2, iVertice, 0, 1);

		iVertice++;

		return 6;
	}

	void fillVBOCube(YVbo * vbo, float size = 5.0f)
	{
		int iVertice = 0;

		YVec3f a(size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f b(size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f c(size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f d(size / 2.0f, -size / 2.0f, size / 2.0f);
		YVec3f e(-size / 2.0f, -size / 2.0f, -size / 2.0f);
		YVec3f f(-size / 2.0f, size / 2.0f, -size / 2.0f);
		YVec3f g(-size / 2.0f, size / 2.0f, size / 2.0f);
		YVec3f h(-size / 2.0f, -size / 2.0f, size / 2.0f);

		iVertice += addQuadToVbo(vbo, iVertice, a, b, c, d); //x+
		iVertice += addQuadToVbo(vbo, iVertice, f, e, h, g); //x-
		iVertice += addQuadToVbo(vbo, iVertice, b, f, g, c); //y+
		iVertice += addQuadToVbo(vbo, iVertice, e, a, d, h); //y-
		iVertice += addQuadToVbo(vbo, iVertice, c, g, h, d); //z+
		iVertice += addQuadToVbo(vbo, iVertice, e, f, b, a); //z-
	}

	void updateSunAndMoon()
	{
		// Determine l'angle du soleil / de la lune en fonction de l'heure
		// Le soleil se l�ve � 6h et se couche � 19h
		SYSTEMTIME SystemTime;
		GetSystemTime(&SystemTime);
		if (increaseSunSpeed)
			timeOffset += this->DeltaTime * 20000.0f;
		if (SystemTime.wHour > 5 && SystemTime.wHour < 19)
			sunAngle = M_PI / 46800.0f * (SystemTime.wHour * 3600 + SystemTime.wMinute * 60 + SystemTime.wSecond + timeOffset) - M_PI / 13.0f * 6.0f;
		else
			sunAngle = M_PI / 39600.0f * (SystemTime.wHour * 3600 + SystemTime.wMinute * 60 + SystemTime.wSecond + timeOffset) - M_PI - M_PI / 11.0f * 19.0f;
		if (sunAngle < 0)
			sunAngle += 2 * M_PI;
		sunAngle = fmodf(sunAngle, 2 * M_PI);

		// Definition de la position du soleil / de la lune avec la formule math�matique d'un cercle en 3 dimensions
		// La position de la cam�ra est utilis�e pour donner l'illusion que les astres sont � l'infini
		sunDirection.X = circleRadius * (cos(sunAngle) * unitVector1.X + sin(sunAngle) * unitVector2.X);
		sunDirection.Y = circleRadius * (cos(sunAngle) * unitVector1.Y + sin(sunAngle) * unitVector2.Y);
		sunDirection.Z = circleRadius * (cos(sunAngle) * unitVector1.Z + sin(sunAngle) * unitVector2.Z);
		YVec3f moonDirection;
		moonDirection.X = circleRadius * (cos(sunAngle + M_PI) * unitVector1.X + sin(sunAngle + M_PI) * unitVector2.X);
		moonDirection.Y = circleRadius * (cos(sunAngle + M_PI) * unitVector1.Y + sin(sunAngle + M_PI) * unitVector2.Y);
		moonDirection.Z = circleRadius * (cos(sunAngle + M_PI) * unitVector1.Z + sin(sunAngle + M_PI) * unitVector2.Z);
		sunPosition = Renderer->Camera->Position + sunDirection;
		moonPosition = Renderer->Camera->Position + moonDirection;

		// Definition des couleurs du soleil et du ciel
		YColor targetSkyColor;
		float status = 0.0f;
		float sunStatus = 0.0f;

		if (sunAngle > 0.0f && sunAngle < M_PI / 2.0f) // Couleur jour
		{
			SkyColor = YColor(0.4f, 0.588f, 0.729f, 1.0f);
			targetSkyColor = YColor(0.905f, 0.647f, 0.325f, 1.0f);
			status = 1 - abs(cos(sunAngle));
		}
		else if (sunAngle >= M_PI / 2.0f && sunAngle < M_PI) // Couleur cr�puscule
		{
			SkyColor = YColor(0.905f, 0.647f, 0.325f, 1.0f);
			targetSkyColor = YColor(0.494f, 0.294f, 0.407f, 1.0f);
			status = abs(cos(sunAngle));
		}
		else if (sunAngle >= M_PI && sunAngle < 3.0f * M_PI / 2.0f) // Couleur nuit
		{
			SkyColor = YColor(0.494f, 0.294f, 0.407f, 1.0f);
			targetSkyColor = YColor(0.0f, 0.0f, 0.0f, 1.0f);
			status = 1 - abs(cos(sunAngle));
		}
		else // Couleur aube
		{
			SkyColor = YColor(0.0f, 0.0f, 0.0f, 1.0f);
			targetSkyColor = YColor(0.4f, 0.588f, 0.729f, 1.0f);
			status = abs(cos(sunAngle));
		}
		SkyColor = SkyColor.interpolate(targetSkyColor, status);
		Renderer->setBackgroundColor(SkyColor);
	}

	void renderSunAndMoon()
	{
		// Rendu du soleil
		glPushMatrix();
		glUseProgram(SunShader);
		SunColor = YColor(1, 1, 0.5, 1);
		GLuint var = glGetUniformLocation(SunShader, "sun_color");
		glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);
		glTranslatef(sunPosition.X, sunPosition.Y, sunPosition.Z);
		glScalef(20.0f, 20.0f, 20.0f);
		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(SunShader);
		VboCube->render();
		glPopMatrix();
		// Rendu de la lune
		glPushMatrix();
		glUseProgram(SunShader);
		SunColor = YColor(1, 1, 1, 1);
		var = glGetUniformLocation(SunShader, "sun_color");
		glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);
		glTranslatef(moonPosition.X, moonPosition.Y, moonPosition.Z);
		glScalef(20.0f, 20.0f, 20.0f);
		Renderer->updateMatricesFromOgl();
		Renderer->sendMatricesToShader(SunShader);
		VboCube->render();
		glPopMatrix();
	}

	void update(float elapsed)
	{
		// Update de l'avatar si n�cessaire
		CameraType cameraType = (CameraType)cameraTypeIndex;
		if (cameraType == CameraType::FirstPerson || cameraType == CameraType::FirstPersonFly)
		{
			glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
			avatar->update(elapsed, true);
		}	
		else if (cameraType == CameraType::ThirdPerson)
		{
			avatar->update(elapsed, false);
		}
		else
		{
			// On bouge la cam�ra en fonction des inputs
			Renderer->Camera->move(Renderer->Camera->Direction * xMovement + Renderer->Camera->RightVec * yMovement);
		}

		cm->update(elapsed);
	}

	void renderObjects();

	void resize(int width, int height)
	{
	}

	void switchCamera()
	{
		cameraTypeIndex++;
		cameraTypeIndex %= 4;

		if (cameraTypeIndex == int(CameraType::Observation))
			LblCamera->Text = "Observation Camera";
		else if (cameraTypeIndex == int(CameraType::FirstPerson))
			LblCamera->Text = "First Person Camera (Jump)";
		else if (cameraTypeIndex == int(CameraType::FirstPersonFly))
			LblCamera->Text = "First Person Camera (Fly)";
		else
			LblCamera->Text = "Third Person Camera";
	}

	/*INPUTS*/

	void keyPressed(int key, bool special, bool down, int p1, int p2) 
	{	
		if (key == 'g')
		{
			if (down)
				increaseSunSpeed = true;
			else
				increaseSunSpeed = false;
		}

		if (key == GLUT_KEY_CTRL_L)
		{
			if (down)
				isControlHeld = true;
			else
				isControlHeld = false;
		}

		if (key == 'z')
			if (down)
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					xMovement = speed * this->DeltaTime;
				else
					avatar->Forward = true;
			else
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					xMovement = 0.0f;
				else
					avatar->Forward = false;
		else if (key == 's')
			if (down)
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					xMovement = -speed * this->DeltaTime;
				else
					avatar->Backward = true;
			else
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					xMovement = 0.0f;
				else
					avatar->Backward = false;

		if (key == 'd')
			if (down)
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					yMovement = speed * this->DeltaTime;
				else
					avatar->Right = true;
			else
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					yMovement = 0.0f;
				else
					avatar->Right = false;
		else if (key == 'q')
			if (down)
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					yMovement = -speed * this->DeltaTime;
				else
					avatar->Left = true;
			else
				if ((CameraType)cameraTypeIndex == CameraType::Observation)
					yMovement = 0.0f;
				else
					avatar->Left = false;

		// Switch entre les cameras (observation, fp, fpvol and tp)
		if (key == 't' && down)
			switchCamera();

		// Saut de l'avatar
		if (key == VK_SPACE)
			if (down)
			{
				if ((CameraType)cameraTypeIndex == CameraType::FirstPersonFly)
					avatar->Fly = true;
				else if ((CameraType)cameraTypeIndex != CameraType::Observation)
					avatar->Jump = true;
			}
			else
			{
				if ((CameraType)cameraTypeIndex == CameraType::FirstPersonFly)
					avatar->Fly = false;
			}
	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		if ((CameraType)cameraTypeIndex == CameraType::Observation)
			Renderer->Camera->move(Renderer->Camera->Direction * dir * zoomSpeed * this->DeltaTime);
	}

	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		if ((CameraType)cameraTypeIndex == CameraType::Observation)
		{
			if (button == GLUT_RIGHT_BUTTON)
			{
				if (state == GLUT_DOWN)
					hasSubjectiveView = true;
				else if (state == GLUT_UP)
					hasSubjectiveView = false;
			}

			if (button == GLUT_MIDDLE_BUTTON)
			{
				if (state == GLUT_DOWN)
					hasWheelView = true;
				else if (state == GLUT_UP)
					hasWheelView = false;
			}
		}
		else
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
				avatar->Pick = true;
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		CameraType cameraType = (CameraType)cameraTypeIndex;
		if (cameraType == CameraType::FirstPerson || cameraType == CameraType::FirstPersonFly)
		{
			if (lastx == -1 && lasty == -1)
			{
				lastx = x;
				lasty = y;
			}

			int dx = x - lastx;
			int dy = y - lasty;

			if (dx == 0 && dy == 0)
				return;

			showMouse(false);

			// Move camera
			Renderer->Camera->rotate(-(float)dx * cameraRotationSpeed);
			Renderer->Camera->rotateUp(-(float)dy * cameraRotationSpeed);
			// Reset de la position souris
			glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
			lastx = Renderer->ScreenWidth / 2;
			lasty = Renderer->ScreenHeight / 2;
		}
		else if (cameraType == CameraType::ThirdPerson)
		{
			if (lastx == -1 && lasty == -1)
			{
				lastx = x;
				lasty = y;
			}

			int dx = x - lastx;
			int dy = y - lasty;

			if (dx == 0 && dy == 0)
				return;

			showMouse(false);

			// Move camera
			Renderer->Camera->rotateAround(-(float)dx * cameraRotationSpeed);
			Renderer->Camera->rotateUpAround(-(float)dy * cameraRotationSpeed);
			// Reset de la position souris
			glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
			lastx = Renderer->ScreenWidth / 2;
			lasty = Renderer->ScreenHeight / 2;
		}
		else
		{
			if (!pressed)
			{
				lastx = x;
				lasty = y;
				showMouse(true);
			}
			else
			{
				if (lastx == -1 && lasty == -1)
				{
					lastx = x;
					lasty = y;
				}

				int dx = x - lastx;
				int dy = y - lasty;

				if (dx == 0 && dy == 0)
					return;

				lastx = x;
				lasty = y;

				if (hasSubjectiveView)
				{
					showMouse(false);
					if (isControlHeld)
					{
						// Move camera
						Renderer->Camera->rotateAround(-(float)dx * cameraRotationSpeed);
						Renderer->Camera->rotateUpAround(-(float)dy * cameraRotationSpeed);
						// Reset de la position souris
						glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
						lastx = Renderer->ScreenWidth / 2;
						lasty = Renderer->ScreenHeight / 2;
					}
					else
					{
						// Move camera
						Renderer->Camera->rotate(-(float)dx * cameraRotationSpeed);
						Renderer->Camera->rotateUp(-(float)dy * cameraRotationSpeed);
						// Reset de la position souris
						glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
						lastx = Renderer->ScreenWidth / 2;
						lasty = Renderer->ScreenHeight / 2;
					}
				}

				if (hasWheelView)
				{
					showMouse(false);
					if (isControlHeld)
					{
						YVec3f strafe = Renderer->Camera->RightVec;
						strafe.Z = 0;
						strafe.normalize();
						strafe *= (float)-dx * strafeSpeed;

						YVec3f avance = Renderer->Camera->Direction;
						avance.Z = 0;
						avance.normalize();
						avance *= (float)dy * strafeSpeed;

						// Move camera
						Renderer->Camera->move(avance + strafe);
						// Reset de la position souris
						glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
						lastx = Renderer->ScreenWidth / 2;
						lasty = Renderer->ScreenHeight / 2;
					}
					else
					{
						YVec3f strafe = Renderer->Camera->RightVec;
						strafe.Z = 0;
						strafe.normalize();
						strafe *= (float)-dx * strafeSpeed;

						// Move camera
						Renderer->Camera->move(Renderer->Camera->UpRef * (float)dy * strafeSpeed);
						Renderer->Camera->move(strafe);
						// Reset de la position souris
						glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
						lastx = Renderer->ScreenWidth / 2;
						lasty = Renderer->ScreenHeight / 2;
					}
				}
			}
		}		
	}	
};


inline void MEngineMinicraft::renderObjects()
{
	/* UPDATE DES POSITIONS DU SOLEIL ET DE LA LUNE */
	updateSunAndMoon();

	#pragma region PASSE2RenduNormal
	/* PASSE 1 : RENDU NORMAL */
	glUseProgram(ShaderWorld);

	/* RENDU DU SOLEIL & DE LA LUNE */
	renderSunAndMoon();

	//glDisable(GL_CULL_FACE);

	/* RENDU DU MONDE */
	glUseProgram(ShaderWorld);
	// Envoi des donn�es n�cessaires au shader
	GLuint var = glGetUniformLocation(ShaderWorld, "world_size");
	glUniform1f(var, MWorld::MAT_SIZE_CUBES * MCube::CUBE_SIZE);

	var = glGetUniformLocation(ShaderWorld, "lightDir");
	glUniform3f(var, sunPosition.X - YRenderer::getInstance()->Camera->Position.X,
		sunPosition.Y - YRenderer::getInstance()->Camera->Position.Y,
		sunPosition.Z - YRenderer::getInstance()->Camera->Position.Z);

	var = glGetUniformLocation(ShaderWorld, "sunColor");
	glUniform3f(var, SunColor.R, SunColor.V, SunColor.B);

	var = glGetUniformLocation(ShaderWorld, "skyColor");
	glUniform3f(var, SkyColor.R, SkyColor.V, SkyColor.B);

	var = glGetUniformLocation(ShaderWorld, "camPos");
	glUniform3f(var, YRenderer::getInstance()->Camera->Position.X, YRenderer::getInstance()->Camera->Position.Y,
	            YRenderer::getInstance()->Camera->Position.Z);

	var = glGetUniformLocation(ShaderWorld, "elapsed");
	glUniform1f(var, this->DeltaTime);

	Renderer->sendTimeToShader(this->DeltaTimeCumul, ShaderWorld);
	tex->setAsShaderInput(ShaderWorld, GL_TEXTURE4, "colTex");
	World->render_world_vbo(false, true);

	/* RENDU DE L'AVATAR */
	// Ne rendre l'avatar que si on est pas en fp
	CameraType cameraType = (CameraType)cameraTypeIndex;
	if (cameraType != CameraType::FirstPerson && cameraType != CameraType::FirstPersonFly)
	{
		glUseProgram(ShaderCubeDebug);
		avatar->render(VboCube);
	}

	/* RENDU DES CREATURES */
	cm->render(this);

	#pragma endregion

	///* RENDU DES AXES */
	//glUseProgram(0);
	//glDisable(GL_LIGHTING);
	//// Axis
	//glBegin(GL_LINES);
	//glColor3d(1, 0, 0);
	//glVertex3d(0, 0, 0);
	//glVertex3d(10000, 0, 0);
	//glColor3d(0, 1, 0);
	//glVertex3d(0, 0, 0);
	//glVertex3d(0, 10000, 0);
	//glColor3d(0, 0, 1);
	//glVertex3d(0, 0, 0);
	//glVertex3d(0, 0, 10000);
	//// Rayon de picking
	//YVec3f target = avatar->updatePickingTarget();
	//glColor3d(1, 0, 0);
	//glVertex3d(avatar->Position.X, avatar->Position.Y, avatar->Position.Z);
	//glVertex3d(target.X, target.Y, target.Z);
	//glEnd();

	glEnable(GL_CULL_FACE);
}
#endif
