#pragma once
#include "engine/utils/types_3d.h"

class MMy_Physics
{
public :
	// a b c sont des points du plan, on doit les fournir dans l'ordre correct pour que la normale soit dans la bonne direction
	static bool GetIntersectionWithPlane(const YVec3f &origin, const YVec3f &target, const YVec3f &a, const YVec3f &b, const YVec3f &c, float &t)
	{
		// Determine la normale du plan
		YVec3f V1 = b - a;
		YVec3f V2 = c - a;
		YVec3f normal = V1.cross(V2);

		// Resolution de l'intersection
		float divider = normal.dot(target - origin);
		t = 0.0f;
		if (divider != 0.0f)
			t = -(origin.dot(normal) - normal.dot(a)) / divider;
		else
			return false;

		if (t > 0.0f) // Garantit que l'intersection est devant
		{
			YVec3f inter = (target - origin) * t + origin;
			return true;
		}
		else
			return false;
	}

	// a b c d sont les vertices de la face, dans l'ordre CCW
	static bool GetIntersectionWithFace(const YVec3f &origin, const YVec3f &target, const YVec3f &a, const YVec3f &b, const YVec3f &c, const YVec3f &d, float &t)
	{
		if (!GetIntersectionWithPlane(origin, target, a, b, c, t))
			return false;
		
		YVec3f inter = (target - origin) * t + origin;
		// Definition des vecteurs
		YVec3f cross1 = (b - a).cross(inter - a);
		YVec3f cross2 = (c - b).cross(inter - b);
		YVec3f cross3 = (d - c).cross(inter - c);
		YVec3f cross4 = (a - d).cross(inter - d);

		// Produits scalaires pour determiner l'orientation de la face
		return (cross1.dot(cross2) >= 0 && cross1.dot(cross3) >= 0 && cross1.dot(cross4) >= 0);
	}

	// Recupere l'intersection la plus proche
	static bool GetIntersectionWithCube(const YVec3f &origin, const YVec3f &target, int x, int y, int z, float &t)
	{
		// Definition des positions des vertices
		YVec3f center(x * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f, y * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f, z * MCube::CUBE_SIZE + MCube::CUBE_SIZE / 2.0f);
		YVec3f a(center.X + MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
		YVec3f b(center.X + MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
		YVec3f c(center.X + MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
		YVec3f d(center.X + MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
		YVec3f e(center.X - MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
		YVec3f f(center.X - MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z - MCube::CUBE_SIZE / 2.0f);
		YVec3f g(center.X - MCube::CUBE_SIZE / 2.0f, center.Y + MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);
		YVec3f h(center.X - MCube::CUBE_SIZE / 2.0f, center.Y - MCube::CUBE_SIZE / 2.0f, center.Z + MCube::CUBE_SIZE / 2.0f);

		// Verification de chaque face du cube
		float tmax = (target - origin).getSize(); // Initialisation de t à sa valeur max
		float tcurrent = tmax;
		if (GetIntersectionWithFace(origin, target, a, b, c, d, t)) // x+
			if (t < tcurrent)
				tcurrent = t;
		if (GetIntersectionWithFace(origin, target, f, e, h, g, t)) // x -
			if (t < tcurrent)
				tcurrent = t;
		if (GetIntersectionWithFace(origin, target, b, f, g, c, t)) // y+
			if (t < tcurrent)
				tcurrent = t;
		if (GetIntersectionWithFace(origin, target, e, a, d, h, t)) // y -
			if (t < tcurrent)
				tcurrent = t;
		if (GetIntersectionWithFace(origin, target, c, g, h, d, t)) // z +
			if (t < tcurrent)
				tcurrent = t;
		if (GetIntersectionWithFace(origin, target, e, f, b, a, t)) // z -
			if (t < tcurrent)
				tcurrent = t;

		// Verif finale
		if (tcurrent == tmax)
			return false;
		else
		{
			t = tcurrent;
			return true;
		}
	}

	// Recupere le cube à pick
	static bool GetNearestPickableCube(const YVec3f &origin, const YVec3f &target, MWorld *world, float &t, int &xp, int &yp, int &zp)
	{
		// Definition de la zone à inspecter en utilisant la position de l'origine et de la cible (matrice de cubes le long du vecteur et pas autour du joueur : plus optimisé)
		float length = (target - origin).getSize();
		YVec3<int> originCubePosition = YVec3<int>(int(floorf((origin.X) / MCube::CUBE_SIZE)), int(floorf((origin.Y) / MCube::CUBE_SIZE)), int(floorf((origin.Z) / MCube::CUBE_SIZE)));
		YVec3<int> targetCubePosition = YVec3<int>(int(floorf((target.X) / MCube::CUBE_SIZE)), int(floorf((target.Y) / MCube::CUBE_SIZE)), int(floorf((target.Z) / MCube::CUBE_SIZE)));
		YVec3<int> minBounds = YVec3<int>(min(originCubePosition.X, targetCubePosition.X), min(originCubePosition.Y, targetCubePosition.Y), min(originCubePosition.Z, targetCubePosition.Z));
		YVec3<int> maxBounds = YVec3<int>(max(originCubePosition.X, targetCubePosition.X), max(originCubePosition.Y, targetCubePosition.Y), max(originCubePosition.Z, targetCubePosition.Z));

		// Test de chaque cube de la zone, s'il peut être pick (air et eau ne le peuvent pas, par ex)
		float tcurrent = length;
		for (int x = minBounds.X; x <= maxBounds.X; x++)
			for (int y = minBounds.Y; y <= maxBounds.Y; y++)
				for (int z = minBounds.Z; z <= maxBounds.Z; z++)
					if (world->getCube(x, y, z)->isPickable())
						if (GetIntersectionWithCube(origin, target, x, y, z, t))
							if (t < tcurrent)
							{
								tcurrent = t;
								xp = x;
								yp = y;
								zp = z;
							}

		t = tcurrent;
		return (tcurrent != length);
	}
};
