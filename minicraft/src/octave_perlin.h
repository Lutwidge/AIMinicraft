#pragma once

#include "engine/noise/perlin.h"

class YOctavePerlin : public YPerlin
{
public:

	YOctavePerlin() : YPerlin() { };

public:

	float sample(float xBase, float yBase, float zBase, int octaves, float persistence)
	{
		float total = 0.0f;
		float frequency = 1.0f;
		float amplitude = 1.0f;
		float maxValue = 0.0f; // Utilise pour normaliser le resultat entre 0 et 1

		for (int i = 0; i < octaves; i++)
		{
			total += YPerlin::sample(xBase * frequency, yBase * frequency, zBase * frequency) * amplitude;

			maxValue += amplitude;
			amplitude *= persistence;
			frequency *= 2;
		}

		return total / maxValue;
	}
};

