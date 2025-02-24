#include "Collision.h"

bool Collision::AABB(const SDL_Rect& recA, const SDL_Rect& recB, float offsetRatio)
{
	if (
		recA.x + recA.w*offsetRatio >= recB.x &&
		recB.x + recB.w >= recA.x + recA.w*(1-offsetRatio) &&
		recA.y + recA.h*offsetRatio >= recB.y &&
		recB.y + recB.h >= recA.y + recA.h*(1-offsetRatio)
		)
	{
		return true;
	}

	return false;
}