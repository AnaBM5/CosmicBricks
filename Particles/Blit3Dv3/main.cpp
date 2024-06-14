/*
Example program that demonstrates collision handling
*/
#include "Blit3D.h"

#include <random>

#include "Physics.h"
#include "Entity.h"
#include "PaddleEntity.h"
#include "BallEntity.h"
#include "BrickEntity.h"
#include "GroundEntity.h"
#include "PowerUpEntity.h"

#include "MyContactListener.h" //for handling collisions

#include "CollisionMask.h"

#include "Particle.h" //particles, yay!

#include "Camera.h" //Screenshake, more yay!

Blit3D *blit3D = NULL;

//this code sets up memory leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


//GLOBAL DATA
std::mt19937 rng;
std::uniform_real_distribution<float> plusMinus5Degrees(-5, +5);
std::uniform_real_distribution<float> plusMinus70Degrees(-70, +70);
std::uniform_real_distribution<float> plusMinus90Degrees(-90, +90);
std::uniform_real_distribution<float> random360Degrees(0, 360);


b2Vec2 gravity; //defines our gravity vector
b2World *world; //our physics engine

// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;
float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed

//contact listener to handle collisions between important objects
MyContactListener *contactListener;

float cursorX = 0;
PaddleEntity *paddleEntity = NULL;

enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;
bool attachedBall = true; //is the ball ready to be launched from the paddle?
int lives = 3;

std::vector<BrickEntity *> brickEntityList; //bricks go here
std::vector<BallEntity *> ballEntityList; //track the balls seperately from everything else
std::vector<Entity *> entityList; //other entities in our game go here
std::vector<Entity *> deadEntityList; //dead entities

float currentBallSpeed = 160; //defaultball speed
int level = 0; //current level of play
int score = 0;

//Sprites 
Sprite *logo = NULL;
Sprite *ballSprite = NULL;
Sprite* fireBallSprite = NULL;
Sprite* lifeSprite = NULL;

Sprite *yellowBrickSprite = NULL;
Sprite* greyBrickSprite = NULL;
Sprite *greenBrickSprite = NULL;
Sprite* bronzeBrickSprite = NULL;
Sprite* purpleBrickSprite = NULL;

Sprite *paddleSprite = NULL;


Sprite *multiBallSprites[5];
Sprite* fastBallSprites[5];

Sprite* backgroundSprite = NULL;
Sprite* gameOverBackground = NULL;

//font
AngelcodeFont *debugFont = NULL;
AngelcodeFont* gameOverFont = NULL;

//particle stuff
std::vector<Particle*> particleList;

Sprite* cloudSprite1 = NULL;
Sprite* cloudSprite2 = NULL;
Sprite* cloudSprite3 = NULL;

Sprite* explosionSprite1 = NULL;
Sprite* explosionSprite2 = NULL;
Sprite* explosionSprite3 = NULL;
Sprite* explosionSprite4 = NULL;
Sprite* explosionSprite5 = NULL;
Sprite* explosionSprite6 = NULL;
Sprite* explosionSprite7 = NULL;

Sprite* boinkSprite1 = NULL;
Sprite* boinkSprite2 = NULL;
Sprite* boinkSprite3 = NULL;

Camera2D* camera = NULL; //for screenshake

//Build the levels
void MakeLevel()
{
	//delete any extra balls and attach ball
	for (int i = ballEntityList.size() - 1; i > 0; --i)
	{
		world->DestroyBody(ballEntityList[i]->body);
		delete ballEntityList[i];
		ballEntityList.erase(ballEntityList.begin() + i);
	}
	attachedBall = true;

	//Removes porwerUps when the level changes
	ballEntityList[0]->fastBallTimer = 0;
	ballEntityList[0]->fireBallTimer = 0;

	//delete all extra entities, like powerups
	for (int i = entityList.size() - 1; i >= 0; --i)
	{
		if (entityList[i]->typeID != ENTITYGROUND && entityList[i]->typeID != ENTITYPADDLE)
		{
			world->DestroyBody(entityList[i]->body);
			delete entityList[i];
			entityList.erase(entityList.begin() + i);
		}
	}

	
	currentBallSpeed = 50 + 7.5f * level; //balls move faster by default at higher levels

	//remove any old bricks, just in case
	for (auto &b : brickEntityList)
	{
		world->DestroyBody(b->body);
		delete b;
	}

	brickEntityList.clear();

	//Advances the level
	if(level<3) 
		level++;

	std::string levelFile = "level" + std::to_string(level) +".txt";
	
	// And loads the corresponding file
	LoadMap(levelFile, brickEntityList);

}

void GroundCollisionParticle(Entity* B){
	
	Particle* p = new Particle();
	
	p->spriteList.push_back(explosionSprite4);
	p->spriteList.push_back(explosionSprite1);
	p->spriteList.push_back(explosionSprite1);
	p->spriteList.push_back(explosionSprite2);
	p->spriteList.push_back(explosionSprite3);
	p->spriteList.push_back(explosionSprite4);
	p->spriteList.push_back(explosionSprite5);
	p->spriteList.push_back(explosionSprite6);
	p->spriteList.push_back(explosionSprite7);
	
	p->spriteList.push_back(cloudSprite3);
	p->spriteList.push_back(cloudSprite2);
	p->spriteList.push_back(cloudSprite1);
	p->spriteList.push_back(cloudSprite1);
	p->spriteList.push_back(cloudSprite2);
	p->spriteList.push_back(cloudSprite3);
	
	p->rotationSpeed = plusMinus90Degrees(rng);
	p->angle = random360Degrees(rng);
	p->direction = b2Vec2(0.f, 0.f);
	p->startingSpeed = 0;
	p->targetSpeed = 0;
	p->totalTimeToLive = 0.5f;
	//get coords of ball
	p->coords = Physics2Pixels(B->body->GetPosition());

	particleList.push_back(p);
}

void BrickCollisionParticle(Entity* B, MyContact contact) {
	Particle* p = new Particle();
	p->spriteList.push_back(cloudSprite1);
	p->spriteList.push_back(cloudSprite2);
	p->spriteList.push_back(cloudSprite3);
	p->rotationSpeed = plusMinus90Degrees(rng);
	p->angle = random360Degrees(rng);
	//let's make it 'follow' after the ball
	b2Vec2 dir = B->body->GetLinearVelocity();
	float speed = dir.Length() * PTM_RATIO;
	dir.Normalize();
	p->direction = dir;
	p->startingSpeed = speed / 5;
	p->targetSpeed = speed / 10;
	p->totalTimeToLive = 0.2f;
	//get coords of contact
	p->coords = Physics2Pixels(contact.contactPoint);

	particleList.push_back(p);
}

//ensures that entities are only added ONCE to the deadEntityList
void AddToDeadList(Entity *e)
{
	bool unique = true;

	for (auto &ent : deadEntityList)
	{
		if (ent == e)
		{
			unique = false;
			break;
		}
	}

	if (unique) deadEntityList.push_back(e);
}

void Init()
{
	//seed random generator
	std::random_device rd;
	rng.seed(rd());

	//turn off the cursor
	blit3D->ShowCursor(false);

	//setup camera
	camera = new Camera2D();

	//set it's valid pan area
	camera->minX = -blit3D->screenWidth / 2;
	camera->minY = -blit3D->screenHeight / 2;
	camera->maxX = blit3D->screenWidth / 2;
	camera->maxY = blit3D->screenHeight / 2;

	camera->PanTo(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

	debugFont = blit3D->MakeAngelcodeFontFromBinary32("Media\\Score.bin");
	gameOverFont = blit3D->MakeAngelcodeFontFromBinary32("Media\\GameOverFont120.bin");

	//load the sprites
	greenBrickSprite = blit3D->MakeSprite(0, 0, 64, 32, "Media\\greenRectangle.png");
	greyBrickSprite = blit3D->MakeSprite(0, 0, 64, 32, "Media\\greyRectangle.png");
	yellowBrickSprite = blit3D->MakeSprite(0, 0, 64, 32, "Media\\yellowRectangle.png");
	bronzeBrickSprite = blit3D->MakeSprite(0, 0, 64, 32, "Media\\bronzeRectangle.png");
	purpleBrickSprite = blit3D->MakeSprite(0, 0, 64, 32, "Media\\purpleRectangle.png");
	backgroundSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\background.png");
	gameOverBackground = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\GameOver.png");

	//power up animated sprite lists
	Sprite* powerUp;
	powerUp = blit3D->MakeSprite(0,0,45, 50, "Media\\powerUp1.png");
	multiBallSprites[0] = powerUp;
	multiBallSprites[1] = powerUp;
	powerUp = blit3D->MakeSprite(0, 0, 45, 50, "Media\\powerUp2.png");
	multiBallSprites[2] = powerUp;
	multiBallSprites[4] = powerUp;
	powerUp = blit3D->MakeSprite(0, 0, 45, 50, "Media\\powerUp3.png");
	multiBallSprites[3] = powerUp;

	///power down animated sprite lists
	Sprite* powerDown;
	powerDown = blit3D->MakeSprite(0, 0, 45, 50, "Media\\powerDown1.png");
	fastBallSprites[0] = powerDown;
	fastBallSprites[1] = powerDown;
	powerDown = blit3D->MakeSprite(0, 0, 45, 50, "Media\\powerDown2.png");
	fastBallSprites[2] = powerDown;
	fastBallSprites[4] = powerDown;
	powerDown = blit3D->MakeSprite(0, 0, 45, 50, "Media\\powerDown3.png");
	fastBallSprites[3] = powerDown;

	logo = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\icon.png");

	paddleSprite = blit3D->MakeSprite(0, 0, 140, 37, "Media\\paddle2.png");
	
	ballSprite = blit3D->MakeSprite(0, 0, 20, 20, "Media\\ball.png");
	fireBallSprite = blit3D->MakeSprite(0, 0, 28, 28, "Media\\bigFireball.png");
	lifeSprite = blit3D->MakeSprite(0, 0, 28, 28, "Media\\life.png");

	//particle sprites
	cloudSprite1 = blit3D->MakeSprite(355, 419, 26, 26, "Media\\arinoid_master.png");
	cloudSprite2 = blit3D->MakeSprite(386, 419, 28, 29, "Media\\arinoid_master.png");
	cloudSprite3 = blit3D->MakeSprite(421, 423, 23, 21, "Media\\arinoid_master.png");

	explosionSprite1 = blit3D->MakeSprite(3, 165, 25, 24, "Media\\arinoid_master.png");
	explosionSprite2 = blit3D->MakeSprite(37, 163, 23, 24, "Media\\arinoid_master.png");
	explosionSprite3 = blit3D->MakeSprite(69, 166, 21, 20, "Media\\arinoid_master.png");
	explosionSprite4 = blit3D->MakeSprite(102, 168, 19, 17, "Media\\arinoid_master.png");
	explosionSprite5 = blit3D->MakeSprite(136, 170, 15, 14, "Media\\arinoid_master.png");
	explosionSprite6 = blit3D->MakeSprite(170, 172, 11, 11, "Media\\arinoid_master.png");
	explosionSprite7 = blit3D->MakeSprite(204, 174, 7, 7, "Media\\arinoid_master.png");

	boinkSprite1 = blit3D->MakeSprite(420, 388, 27, 26, "Media\\arinoid_master.png");
	boinkSprite2 = blit3D->MakeSprite(388, 388, 27, 26, "Media\\arinoid_master.png");
	boinkSprite3 = blit3D->MakeSprite(356, 388, 27, 26, "Media\\arinoid_master.png");

	//from here on, we are setting up the Box2D physics world model

	// Define the gravity vector.
	gravity.x = 0.f;
	gravity.y = 0.f;

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

								   //_________GROUND OBJECT_____________
								   //make an entity for the ground
	GroundEntity *groundEntity = new GroundEntity();
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	//add the userdata
	groundBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(groundEntity);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	groundEntity->body = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 1 edge shape at the bottom of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//collison masking
	boxShapeDef.filter.categoryBits = CMASK_GROUND;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_BALL | CMASK_POWERUP;		//it collides wth balls and powerups

																	//bottom
	groundBox.SetTwoSided(b2Vec2(0, -1), b2Vec2(blit3D->screenWidth / PTM_RATIO, -1));
	//Create the fixture
	groundEntity->body->CreateFixture(&boxShapeDef);
	
	//add to the entity list
	entityList.push_back(groundEntity);

	//now make the other 3 edges of the screen on a seperate body
	groundBodyDef.userData.pointer = NULL;
	b2Body *screenEdgeBody = world->CreateBody(&groundBodyDef);

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_BALL;		//it collides wth balls
	//note that even if we set the colison mask for colliding with the paddle, it wouldn't work, as
	//Box2D's automatic filtering disables collisions between STATIC objects (the edges) and 
	//KINEMATIC objects (the paddle)

													//left
	groundBox.SetTwoSided(b2Vec2(0, blit3D->screenHeight / PTM_RATIO), b2Vec2(0, 0));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//top
	groundBox.SetTwoSided(b2Vec2(0, blit3D->screenHeight / PTM_RATIO),
		b2Vec2(blit3D->screenWidth / PTM_RATIO, blit3D->screenHeight / PTM_RATIO));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//right
	groundBox.SetTwoSided(b2Vec2(blit3D->screenWidth / PTM_RATIO,
		0), b2Vec2(blit3D->screenWidth / PTM_RATIO, blit3D->screenHeight / PTM_RATIO));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//______MAKE A BALL_______________________
	BallEntity *ball = MakeBall(ballSprite,fireBallSprite);

	//move the ball to the center bottomof the window
	b2Vec2 pos(blit3D->screenWidth / 2, 53);
	pos = Pixels2Physics(pos);//convert coordinates from pixels to physics world
	ball->body->SetTransform(pos, 0.f);

	//add the ball to our list of (ball) entities
	ballEntityList.push_back(ball);

	// Create contact listener and use it to collect info about collisions
	contactListener = new MyContactListener();
	world->SetContactListener(contactListener);

	//paddle
	cursorX = blit3D->screenWidth / 2;
	paddleEntity = MakePaddle(blit3D->screenWidth / 2, 30, paddleSprite);
	entityList.push_back(paddleEntity);
}

void DeInit(void)
{
	if (camera != NULL) delete camera;

	//delete all particles
	for (auto& p : particleList) delete p;
	particleList.clear();

	//delete all the entities
	for (auto &e : entityList) delete e;
	entityList.clear();

	for (auto &b : ballEntityList) delete b;
	ballEntityList.clear();

	for (auto &b : brickEntityList) delete b;
	brickEntityList.clear();

	//delete the contact listener
	delete contactListener;

	//Free all physics game data we allocated
	delete world;

	//any sprites/fonts still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	if (gameState == START) return;

	//stop it from lagging hard if more than a small amount of time has passed
	if (seconds > 1.0 / 30) elapsedTime += 1.f / 30;
	else elapsedTime += (float)seconds;
	
	//did we finish the level?
	if (brickEntityList.size() == 0)
	{
		MakeLevel();
	}

	//move paddle to where the cursor is
	b2Vec2 paddlePos;
	paddlePos.y = 30;

	float halfWidthPixels = paddleEntity->halfWidth * PTM_RATIO;
	if (cursorX < halfWidthPixels) paddlePos.x = halfWidthPixels;
	else if (cursorX > blit3D->screenWidth - halfWidthPixels) paddlePos.x = blit3D->screenWidth - halfWidthPixels;
	else paddlePos.x = cursorX;

	paddlePos = Pixels2Physics(paddlePos);
	paddleEntity->body->SetTransform(paddlePos, 0);

	//make sure the balls keep moving at the right speed, and delete any way out of bounds
	for (int i = ballEntityList.size() - 1; i >= 0; --i)
	{
		b2Vec2 dir = ballEntityList[i]->body->GetLinearVelocity();


		//check if we are not moving enough vertically
		float minAmountY = 5.f * PTM_RATIO;
		if (dir.y < 0)
		{
			if (dir.y > -minAmountY)
			{
				dir.y = -minAmountY;
			}
		}
		else
		{
			if (dir.y < minAmountY)
			{
				dir.y = minAmountY;
			}
		}
		dir.Normalize();

		if (ballEntityList[i]->fastBallTimer > 0.f)	//duplicates speed if the fastBall power is active
		{
			ballEntityList[i]->fastBallTimer -= seconds;
			dir *= currentBallSpeed * 1.7f;						
		}
		else dir *= currentBallSpeed; //else, uses normal speed
		
		if (ballEntityList[i]->fireBallTimer > 0.f)	//reduces fireball timer
		{
			ballEntityList[i]->fireBallTimer -= seconds;
		}
		
		ballEntityList[i]->body->SetLinearVelocity(dir); //apply velocity to kinematic object

		b2Vec2 pos = ballEntityList[i]->body->GetPosition();
		if (pos.x < 0 || pos.y < 0
			|| pos.x * PTM_RATIO > blit3D->screenWidth
			|| pos.y * PTM_RATIO > blit3D->screenHeight)
		{
			//kill this ball, it is out of bounds
			world->DestroyBody(ballEntityList[i]->body);
			delete ballEntityList[i];
			ballEntityList.erase(ballEntityList.begin() + i);
		}
	}

	//sanity check, should always have at least one ball!
	if (ballEntityList.size() == 0)
	{
		//welp, let's make a ball
		BallEntity *ball = MakeBall(ballSprite, fireBallSprite);
		ballEntityList.push_back(ball);
		attachedBall = true;
	}

	//if ball is attached to paddle, move ball to where paddle is
	if (attachedBall)
	{
		assert(ballEntityList.size() > 0); //make sure there is at least one ball
		b2Vec2 ballPos = paddleEntity->body->GetPosition();
		ballPos.y = 53 / PTM_RATIO;
		ballEntityList[0]->body->SetTransform(ballPos, 0);
		ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
	}


	//don't apply physics unless at least a timestep worth of time has passed
	while (elapsedTime >= timeStep)
	{
		//update the physics world
		world->Step(timeStep, velocityIterations, positionIterations);

		// Clear applied body forces. 
		world->ClearForces();

		elapsedTime -= timeStep;

		//update game logic/animation
		for (auto &e : entityList) e->Update(timeStep);
		for (auto &b : ballEntityList) b->Update(timeStep);
		for (auto &b : brickEntityList) b->Update(timeStep);

		camera->Update(timeStep);

		//update the particle list and remove dead particles
		for (int i = particleList.size() - 1; i >= 0; --i)
		{
			if (particleList[i]->Update(timeStep))
			{
				//time to die!
				delete particleList[i];
				particleList.erase(particleList.begin() + i);
			}
		}

		//loop over contacts
		MyContact contact;
		for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
		{
			contact = contactListener->contacts[pos];

			//fetch the entities from the body userdata
			Entity *A = (Entity *)contact.fixtureA->GetBody()->GetUserData().pointer;
			Entity *B = (Entity *)contact.fixtureB->GetBody()->GetUserData().pointer;

			if (A != NULL && B != NULL) //if there is an entity for these objects...
			{
				//Takes the fixtures that collided
				b2Fixture* AFixture = contact.fixtureA;
				b2Fixture* BFixture = contact.fixtureB;
				
				if (A->typeID == ENTITYBALL)
				{
					//swap A and B
					Entity *C = A;
					A = B;
					B = C;

					AFixture = contact.fixtureA;
					BFixture = contact.fixtureB;
				}

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYBRICK)
				{
					//Refers to the ball as a BallEntity to check the fireBallTimer
					BallEntity* ballEntity = (BallEntity*)B;

					//If the fireball powerup is active, only detects collision from the sensor 
					//And if the powerUp is not active, only detects collision from the regular fixture
					if (BFixture->IsSensor() && ballEntity->fireBallTimer > 0			
						|| !BFixture->IsSensor() && ballEntity->fireBallTimer <= 0)
					{
						BrickEntity* be = (BrickEntity*)A;

						//add Score
						//Adds 10 points per brick hit
						//If fireball is active, gives the points that would have been gotten by hitting
						//the brick multiple times
						if(ballEntity->fireBallTimer > 0 && be->colour != BrickColour::PURPLE)
							score += 10 * ((int)be->colour + 1);
						else
							score += 10;

						
						if (be->HandleCollision(ballEntity))
						{
							//we need to remove this brick from the world, 
							//but can't do that until all collisions have been processed
							AddToDeadList(be);
						}

						//did we hit a brick that should spawn a power up?
						if (be->colour == BrickColour::PURPLE)
						{
							b2Vec2 position = be->body->GetPosition();
							position = Physics2Pixels(position);

							//Generates the powerUp randomly
							std::uniform_int_distribution<int> randomPowerUp(1, 10);
							PowerUpType powerUpType;
							int powerUpNum = randomPowerUp(rng);

							if (powerUpNum <= 2)								//2 in 10 chance of fireball
								powerUpType = PowerUpType::FIREBALL;
							else if (powerUpNum <= 5)							//3 in 10 chance of fastball
								powerUpType = PowerUpType::FASTBALL;
							else
								powerUpType = PowerUpType::MULTIBALL;			//5 in 10 chance of multiball

							//And creates the powerUp
							PowerUpEntity* p = MakePowerUp(powerUpType, position.x, position.y);
							entityList.push_back(p);

							//additional 20 points if the block is purple
							score += 20;
						}

						//add a particle effect
						BrickCollisionParticle(B, contact);

						//shake the screen!
						camera->Shake(1.5);

					}
					
				}
				//Only the regular fixture registers a collision with the paddle and the ground

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYPADDLE && !BFixture->IsSensor())
				{
					PaddleEntity *pe = (PaddleEntity *)A;
					//bend the ball's flight
					pe->HandleCollision(B->body);
				}

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYGROUND && !BFixture->IsSensor())
				{
					if (ballEntityList.size() > 1)
					{
						//remove this ball, but we have others
						AddToDeadList(B);
						//shake the screen a little
						camera->Shake(3);
					}
					else
					{
						//shake the screen!
						camera->Shake(20);
						//lose a life
						lives--;
						attachedBall = true; //attach the ball for launching
						ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
						ballEntityList[0]->fastBallTimer = 0.f;							//takes away the fast ball
						if (lives < 1)
						{
							//gameover
							gameState = GAMEOVER;
						}
					}

					//add a particle effect
					GroundCollisionParticle(B);
				}

				//check and see if paddle or ground hit a powerup
				if (A->typeID == ENTITYPOWERUP)
				{
					//swap A and B
					Entity *C = A;
					A = B;
					B = C;
				}

				if (B->typeID == ENTITYPOWERUP)
				{
					AddToDeadList(B);

					if (A->typeID == ENTITYPADDLE)
					{
						
						PowerUpEntity *p = (PowerUpEntity *)B;
						switch (p->powerUpType)
						{
						case PowerUpType::MULTIBALL:
							score += 50;					//Picking Up Multiball gives 50 points

							for (int j = ballEntityList.size() - 1; j >= 0; --j)
							{
								//make 2 new balls
								for (int i = 0; i < 2; ++i)
								{
									//add extra balls if we haven't reached pur limit
									if (ballEntityList.size() < 15)
									{
										BallEntity *b = MakeBall(ballSprite, fireBallSprite);
										b2Vec2 ballPos = ballEntityList[j]->body->GetPosition();
										ballPos.x += (i * 40 - 20) / PTM_RATIO;
										b->body->SetTransform(ballPos, 0);
										//kick the ball in a random direction upwards			
										b2Vec2 dir = deg2vec(90 + plusMinus70Degrees(rng)); //between 20-160 degrees
																				
										//make the ball move
										dir *= currentBallSpeed; //scale up the velocity
										b->body->SetLinearVelocity(dir); //apply velocity to kinematic object	
												
										//gives the ball fastball and fireball if the powerUp is still active
										b->fastBallTimer = ballEntityList[j]->fastBallTimer;
										b->fireBallTimer = ballEntityList[j]->fireBallTimer;

										ballEntityList.push_back(b);//add to the ballist
									}
								}
							}
							break;
						case PowerUpType::FASTBALL:
							
							//Picking Up Fastball gives no points

							//makes ball moves faster
							for (auto ball : ballEntityList) 
							{
								ball->fastBallTimer = 8.2f;
							}
							break;
						
						case PowerUpType::FIREBALL:
							score += 80;					//Fireball is rare, gives more points than multiball

							//changes balls to fireballs
							for (auto ball : ballEntityList)
							{
								ball->fireBallTimer = 7.6f;
							}
							break;
						default:
							assert("Unknown PowerUp type" == 0);
							break;
						}//end switch on powerup type
					}
				}
			}//end of checking if they are both NULL userdata
		}//end of collison handling

		 //clean up dead entities
		for (auto &e : deadEntityList)
		{
			//remove body from the physics world and free the body data
			world->DestroyBody(e->body);
			//remove the entity from the appropriate entityList
			if (e->typeID == ENTITYBALL)
			{
				for (int i = 0; i < ballEntityList.size(); ++i)
				{
					if (e == ballEntityList[i])
					{
						delete ballEntityList[i];
						ballEntityList.erase(ballEntityList.begin() + i);
						break;
					}
				}
			}
			else if (e->typeID == ENTITYBRICK)
			{
				for (int i = 0; i < brickEntityList.size(); ++i)
				{
					if (e == brickEntityList[i])
					{
						delete brickEntityList[i];
						brickEntityList.erase(brickEntityList.begin() + i);
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < entityList.size(); ++i)
				{
					if (e == entityList[i])
					{
						delete entityList[i];
						entityList.erase(entityList.begin() + i);
						break;
					}
				}
			}
		}

		deadEntityList.clear();
	}
}

void Draw(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	//clear colour: r,g,b,a 	
											// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	backgroundSprite->Blit(1920.f / 2, 1080.f / 2);
	camera->Draw();
	std::string scoreString;
	float widthText;

	switch (gameState)
	{
	case START:
		logo->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		break;
	
	case GAMEOVER:
		gameOverBackground->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		
		scoreString = "Score :    ";
		widthText = debugFont->WidthText(scoreString);
		gameOverFont->BlitText(930.f - (widthText/2), 430, scoreString);
		
		scoreString = std::to_string(score);
		widthText = debugFont->WidthText(scoreString);
		gameOverFont->BlitText(930.f - (widthText/2), 280, scoreString);
		break;

	case PLAYING:
		//loop over all entities and draw them
		for (auto &e : entityList) e->Draw();
		for (auto &b : brickEntityList) b->Draw();
		for (auto &b : ballEntityList) b->Draw();
		for (auto& p : particleList) p->Draw();


		//draw our life HUD
		for (int i = 0; i < lives; ++i)
		{
			lifeSprite->Blit(225 + i * 50, 1030);
		}

		std::string lifeString = "Lives:";
		debugFont->BlitText(50, 1050, lifeString);

		scoreString = "Score:    " + std::to_string(score);
		widthText = debugFont->WidthText(scoreString);
		debugFont->BlitText(1840 - widthText, 1050, scoreString);
		break;
	
	}
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	//suicide button is K
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		if (gameState == PLAYING)
		{
			//kill off the current first ball
			lives--;
			attachedBall = true; //attach the ball for launching

								 //safety check
			//assert(ballEntityList.size() > 0); //make sure there is at least one ball

			ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
			if (lives < 1)
			{
				//gameover
				gameState = GAMEOVER;
			}
		}
	}
	
	//Goes to next level when pressing N for testing purposes
	if (gameState == PLAYING && (key == GLFW_KEY_N && action == GLFW_PRESS))
	{
		MakeLevel();
	}
}

void DoCursor(double x, double y)
{
	//scale display size
	cursorX = static_cast<float>(x) * (blit3D->screenWidth / blit3D->trueScreenWidth);
}

void DoMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		switch (gameState)
		{
		case START:
			gameState = PLAYING;
			attachedBall = true;
			lives = 3;
			score = 0;
			level = 0;
			MakeLevel();
			break;

		case PLAYING:
			if (attachedBall)
			{
				attachedBall = false;
				//launch the ball

				//safety check
				assert(ballEntityList.size() >= 1); //make sure there is at least one ball	

				Entity *b = ballEntityList[0];
				//kick the ball in a random direction upwards			
				b2Vec2 dir = deg2vec(90 + plusMinus5Degrees(rng)); //between 85-95 degrees
														//make the ball move
				dir *= currentBallSpeed; //scale up the velocity
				b->body->SetLinearVelocity(dir); //apply velocity to kinematic object				
			}
			break;

		case GAMEOVER:
			gameState = START;
			break;
		}
	}

}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);
	blit3D->SetDoCursor(DoCursor);
	blit3D->SetDoMouseButton(DoMouseButton);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}