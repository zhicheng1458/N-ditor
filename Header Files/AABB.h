#pragma once
#include "Core.h"

struct AABB
{
	public:
		glm::vec2 lowerBound; //Bottom left corner
		glm::vec2 upperBound; //Top right corner
		float surfaceArea;

		AABB()
		{
			lowerBound = glm::vec2(0.0f);
			upperBound = glm::vec2(0.0f);
			surfaceArea = 0.0f;
		}

		AABB(float minX, float minY, float maxX, float maxY)
		{
			lowerBound = glm::vec2(minX, minY);
			upperBound = glm::vec2(maxX, maxY);
			surfaceArea = calculateSurfaceArea();
		}

		AABB(glm::vec2 min, glm::vec2 max)
		{
			lowerBound = min;
			upperBound = max;
			surfaceArea = calculateSurfaceArea();
		}

		~AABB()
		{
			//Nothing to do as nothing is allowcated
		}

		AABB operator+(const AABB & box)
		{
			return AABB(glm::min(lowerBound.x, box.lowerBound.x), glm::min(lowerBound.y, box.lowerBound.y),
						glm::max(upperBound.x, box.upperBound.x), glm::max(upperBound.y, box.upperBound.y));
		}

		AABB combine(const AABB & box)
		{
			return AABB(glm::min(lowerBound.x, box.lowerBound.x), glm::min(lowerBound.y, box.lowerBound.y),
						glm::max(upperBound.x, box.upperBound.x), glm::max(upperBound.y, box.upperBound.y));
		}

		bool contains(const glm::vec2 & coordinate) //Check whether this box contains the given coordinate (assumed to be in the same model space)
		{
			if (coordinate.x >= lowerBound.x && coordinate.y >= lowerBound.y &&
				coordinate.x <= upperBound.x && coordinate.y <= upperBound.y)
			{
				return true;
			}
			return false;
		}

		bool contains(const AABB & box) //Check whether this box contains the given box
		{
			if (box.lowerBound.y >= lowerBound.y && box.lowerBound.x >= lowerBound.x &&
				box.upperBound.y <= upperBound.y && box.upperBound.x <= upperBound.x)
			{
				return true;
			}
			return false;
		}

		float getXLength() const { return (upperBound.x - lowerBound.x); }
		float getYLength() const { return (upperBound.y - lowerBound.y); }

	private:
		float calculateSurfaceArea() const { return getXLength() * getYLength(); }
};