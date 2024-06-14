#pragma once

#include "Entity.h"
#include "BallEntity.h"
#include <random>

enum class BrickColour {YELLOW, GREY, GREEN, BRONZE, PURPLE};

//externed sprites
extern Sprite* yellowBrickSprite;
extern Sprite *greyBrickSprite;
extern Sprite *greenBrickSprite;
extern Sprite* bronzeBrickSprite;
extern Sprite *purpleBrickSprite;

extern std::mt19937 rng;

class BrickEntity : public Entity
{
public:
	BrickColour colour;
	float x, y;
	BrickEntity()
	{
		typeID = ENTITYBRICK;
		colour = BrickColour::YELLOW;
		x = 0;
		y = 0;
		
	}

	bool HandleCollision(BallEntity* ball);
};

BrickEntity * MakeBrick(BrickColour type, float xpos, float ypos);

void LoadMap(std::string fileName, std::vector<BrickEntity*>& brickList);

