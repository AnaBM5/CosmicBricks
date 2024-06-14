#include "BrickEntity.h"
#include "BallEntity.h"
#include "CollisionMask.h"
#include <random>

//Decide whether to change colour or die off:
//return true if this pbject should be removed
bool BrickEntity::HandleCollision(BallEntity* ball)
{
	bool retval = false;
	
	if (ball->fireBallTimer > 0)		//If fire ball is active, the brick is destroyed regardless of colour
		return true;
	
	
	switch(colour)
	{
	case BrickColour::GREEN:
		colour = BrickColour::GREY;
		sprite = greyBrickSprite;
		break;

	case BrickColour::GREY:
		colour = BrickColour::YELLOW;
		sprite = yellowBrickSprite;
		break;

	case BrickColour::YELLOW:
	default: //because PURPLE wasn't handled yet, it will be handled by the default case
		retval = true;
		break;
	}

	
	

	return retval;
}

extern b2World *world;

void LoadMap(std::string fileName, std::vector<BrickEntity*>& brickList)
{
	std::uniform_int_distribution<int> brickRandom(0, 7);

	//open file
	std::ifstream myfile;
	myfile.open(fileName);

	if (myfile.is_open())
	{
		//read in # of bricks
		int brickNum = 0;
		myfile >> brickNum;

		//read in each brick
		for (; brickNum > 0; --brickNum)
		{
			//make a brick
			BrickEntity* B = new BrickEntity();
			int t = 0;
			myfile >> t;
			B->colour = (BrickColour)t;

			//1 in 8 chance to change a normal brick into a purple brick
			if (brickRandom(rng) == 0)			
				B->colour = BrickColour::PURPLE;

			myfile >> B->x;
			myfile >> B->y;

			switch (B->colour)
			{
			case BrickColour::GREY:
				B->sprite = greyBrickSprite;
				break;

			case BrickColour::GREEN:
				B->sprite = greenBrickSprite;
				break;

			case BrickColour::YELLOW:
				B->sprite = yellowBrickSprite;
				break;

			case BrickColour::PURPLE:
				B->sprite = purpleBrickSprite;
				break;
			}

			//make the physics body
			b2BodyDef brickBodyDef;

			//set the position of the center of the body, 
			//converting from pxel coords to physics measurements
			brickBodyDef.position.Set(B->x / PTM_RATIO, B->y / PTM_RATIO);
			brickBodyDef.type = b2_kinematicBody; //make it a kinematic body i.e. one moved by us

			//make the userdata point back to this entity
			brickBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(B);

			B->body = world->CreateBody(&brickBodyDef); //create the body and add it to the world

			// Define a box shape for our dynamic body.
			b2PolygonShape boxShape;
			//SetAsBox() takes as arguments the half-width and half-height of the box
			boxShape.SetAsBox(64.0f / (2.f * PTM_RATIO), 32.0f / (2.f * PTM_RATIO));

			b2FixtureDef brickFixtureDef;
			brickFixtureDef.shape = &boxShape;
			brickFixtureDef.density = 1.0f; //won't matter, as we made this kinematic
			brickFixtureDef.restitution = 0;
			brickFixtureDef.friction = 0.1f;

			//collison masking
			brickFixtureDef.filter.categoryBits = CMASK_BRICK;  //this is a brick
			brickFixtureDef.filter.maskBits = CMASK_BALL;		//it collides wth balls

			B->body->CreateFixture(&brickFixtureDef);

			brickList.push_back(B);
		}

		myfile.close();
	}
}