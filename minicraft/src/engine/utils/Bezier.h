#pragma once
#include <utility>
#include "engine/utils/types_3d.h"

class QuadraticBezier
{
protected:
	YVec3f point1;
	YVec3f point2;
	YVec3f point3;

public : 
	QuadraticBezier(YVec3f first, YVec3f second, YVec3f third) : point1(first), point2(second), point3(third)
	{

	}

	std::pair<YVec3f, YVec3f> GetMidSegment(float alpha)
	{
		std::pair<YVec3f, YVec3f> output;
		YVec3f firstSegment = point2 - point1;
		YVec3f secondSegment = point3 - point2;
		output.first = point1 + (firstSegment * alpha);
		output.second = point2 + (secondSegment * alpha);
	}

	virtual YVec3f GetInterpolationValue(float alpha)
	{
		//Clamp [0,1]
		alpha = max(0, min(1, alpha));
		std::pair<YVec3f, YVec3f> temp;
		temp = GetMidSegment(alpha);

		return temp.first + ((temp.second - temp.first) * alpha);
	}
};