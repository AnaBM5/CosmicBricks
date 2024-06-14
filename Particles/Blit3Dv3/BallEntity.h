#pragma once

#include "Entity.h"

class BallEntity : public Entity
{
public:

	float fastBallTimer = 0.f;		//Keeps track of how much time must pass before the ball returns to the usual speed
	float fireBallTimer = 0.f;		//Keeps track of how much time must pass before the ball returns to the usual speed

	Sprite* fireSprite = NULL;
	BallEntity() 
	{
		typeID = ENTITYBALL;
	}

	void BallEntity::Draw();
};

BallEntity * MakeBall(Sprite *sprite, Sprite *fireBallSprite);