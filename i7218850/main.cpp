#include "cgg/Graphics.h"
#include <iostream>
#include <ctime>

float g_time = 0;
int g_ship = 0;
int g_rand;
float g_timeCheck = 0;
int g_asteroid = 0;
int g_score = 0;
int g_bullet = 0;
int g_asteroidNumber = 6;
int g_lives = 3;
int g_hit = -1;
Vec2 g_lasthit = (40.0f,40.0f);
bool g_inGame = true;
bool g_spawnCollision = true;
Matrix2 g_shipMX;
std::vector<Matrix2> g_asteroidMX;
std::vector<Vec2> g_asteroidDirectionMX;
std::vector<Matrix2> g_bulletMX;
float g_shipAngle = 0;
float g_velocity;
Vec2 g_min(-19, -14);
// The points that make up the asteroid
Vec2 g_asteroidPoints[] = 
{
	Vec2( 0.0f,1.0f),			
	Vec2( 2.0f,0.0f), 
	Vec2( 4.0f,-3.0f),			 
	Vec2( 4.0f,-4.0f),
	Vec2( 3.0f,-4.0f),
	Vec2( 2.0f,-5.0f),
	Vec2( 0.0f,-4.0f),
	Vec2( -1.0f,-2.0f),
	Vec2( 0.0f,1.0f)
};
// The points that make up the ship
Vec2 g_shipPoints[] = 
{
	Vec2( 0.0f, 1.0f), 
	Vec2( 1.0f,-1.0f), 
	Vec2( 0.20f,-0.65f), 
	Vec2(-0.20f,-0.65f),			 
	Vec2(-1.0f,-1.0f),
	Vec2( 0.0f, 1.0f)
};
// The points that make up the bullet
Vec2 g_bulletPoints[] = 
{
	Vec2(-0.1f, 0.1f),
	Vec2( 0.1f, 0.1f),
	Vec2( 0.1f,-0.1f),
	Vec2(-0.1f,-0.1f)
};

float randomFloat()
{ 
	return (float)rand() / RAND_MAX; 
} 
 
float randomFloat(float min, float max) 
{ 
	return randomFloat() * (max - min) + min; 
}

Vec2 randVec2(Vec2 min, Vec2 max)
{ 
	Vec2 v; 
	v.x = randomFloat(min.x, max.x); 
	v.y = randomFloat(min.y, max.y); 
	return v; 
}

Vec2 randDirectionUniformDist()
{ 
	float f = randomFloat(0, TWO_PI); 
	Vec2 v; v.x = cos(f); 
	v.y = sin(f); 
	return normalize(v); 
}

float collisionCheck(Vec2 P, Vec2 D, Vec2 a, Vec2 b)
{
	Vec2 ab = b - a;

	float determinant = ab.x * D.y - ab.y * D.x;
	if(fabs(determinant) > 0.00001f)
	{
		//check distance to the intersection
		float d = (ab.y * (P.x - a.x) - ab.x * (P.y - a.y)) / determinant;
		//find where on the line ab it intersects
		float g;
		//make sure there is not impossible division (0)
		if(fabs(ab.x) > 0.00001f)
 		{
 			g = (P.x + (D.x * d) - a.x) / ab.x;
 		}
 		else
 		{
 			g = (P.y + (D.y * d) - a.y) / ab.y;
 		}
		//check for collision (0<g>1)
		if(g >= 0 && g <= 1.0f)
 		{
 			return d;
 		}
	}
	//no intersect
	return -1;
}

// Declare an update function pointer type. (Takes a float as an argument, returns void, and the function pointer type is called 'UpdateFn')
typedef void (*UpdateFn)(float);

// Declare a draw function pointer type. (Takes no arguments, returns void, and the function pointer type is called 'DrawFn')
typedef void (*DrawFn)();

// the 3 update funcs
void updateScreen1(float dt);
void updateScreen2(float dt);
void updateScreen3(float dt);

// the 3 draw funcs
void drawScreen1();
void drawScreen2();
void drawScreen3();

// A global pointer to the update function for the current screen
UpdateFn g_update = updateScreen1;

// A global pointer to the draw function for the current screen
DrawFn g_draw = drawScreen1;

//------------------------------------------------------------------------------------------------------------------------------------
// called when we initialise the app
//------------------------------------------------------------------------------------------------------------------------------------
void init()
{
	// give our app a title 
	setWindowTitle("Kaleidoscopic Asteroids");

	// sets the size of the 2D screen coordinates. I'm just initialising the defaults here. You can use whatever you want!
	// args are: minx, maxx, miny, maxy
	setScreenCoordinates(-20.0f, 20.0f, -15.0f, 15.0f);
	
	// enable fullscreen mode if in release mode
	#if !defined(_DEBUG)
	fullScreen(true);
	#endif

	srand(time(NULL)); //uses the time to make a seed
	g_asteroidDirectionMX.resize(g_asteroidNumber);
	g_asteroidMX.resize(g_asteroidNumber);
	for(int i = 0; i < g_asteroidNumber; ++i)
	{
		g_spawnCollision = true;
		while (g_spawnCollision)
		{
			g_asteroidMX[i].w = randVec2(g_min, -g_min);
			if (g_asteroidMX[i].w.y - 3.0f <= g_shipMX.w.y + 3.0f && g_asteroidMX[i].w.y + 3.0f >= g_shipMX.w.y - 3.0f 
				&& g_asteroidMX[i].w.x - 3.0f <= g_shipMX.w.x + 3.0f && g_asteroidMX[i].w.x + 3.0f >= g_shipMX.w.x - 3.0f)
			{
			}
			else
			{
				g_spawnCollision = false;
			}
		}
		g_asteroidDirectionMX[i] = randDirectionUniformDist();
	}

	// construct the ships shape
	{
		g_ship = beginShape();
		
		//sets the array of the colours for the ship
		Vec3 colours[] = 
		{
			Vec3( 1.0f, 0.0f, 0.0f), 
			Vec3( 0.0f, 1.0f, 0.0f), 
			Vec3( 0.0f,0.0f, 1.0f), 
			Vec3( 1.0f,1.0f, 0.0f), 
			Vec3(0.0f,1.0f, 1.0f)
		};
		//types of display options
		begin(kPolygon);
			for(int i = 0; i < sizeof(g_shipPoints) / sizeof(Vec2); ++i) //sets the value i < than to the number of array
			{
				setColour(colours[i]);
				addVertex(g_shipPoints[i]);
			}
		end();
		endShape();
	}
	// construct the asteroid shape
	{
		g_asteroid = beginShape();
		//sets the array of the colours for the asteroid
		Vec3 colours[] = 
		{
			Vec3( 1.0f, 0.0f, 0.0f), 
			Vec3( 0.0f, 1.0f, 0.0f), 
			Vec3( 0.0f, 0.0f, 1.0f), 
			Vec3( 1.0f, 1.0f, 0.0f), 
			Vec3( 0.0f, 1.0f, 1.0f),
			Vec3( 1.0f, 0.0f, 0.0f), 
			Vec3( 0.0f, 1.0f, 0.0f), 
			Vec3( 0.0f, 0.0f, 1.0f), 
			Vec3( 1.0f, 1.0f, 0.0f)
		};
		//types of display options
		begin(kPolygon);
			for(int i = 0; i < sizeof(g_asteroidPoints) / sizeof(Vec2); ++i) //sets the value i < than to the number of array
			{
				setColour(colours[i]);
				addVertex(g_asteroidPoints[i]);
			}
		end();
		endShape();
	}
	//draw bullet
	{
		g_bullet = beginShape();
		
		//sets the array of the colours for the bullet
		Vec3 colours[] = 
		{
			Vec3( 1.0f, 0.0f, 0.0f), 
			Vec3( 0.0f, 1.0f, 0.0f), 
			Vec3( 0.0f,0.0f, 1.0f), 
			Vec3( 1.0f,1.0f, 0.0f), 
			Vec3(0.0f,1.0f, 1.0f)
		};
		//types of display options
		begin(kPolygon);
			for(int i = 0; i < sizeof(g_bulletPoints) / sizeof(Vec2); ++i) //sets the value i < than to the number of array
			{
				setColour(colours[i]);
				addVertex(g_bulletPoints[i]);
			}
		end();
		endShape();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------
// called when we need to update the app. 'dt' is the time delta (the number of seconds since the last frame)
//------------------------------------------------------------------------------------------------------------------------------------
void updateScreen1(float dt)
{
	g_time += dt;
	
	//set background colour to change
	float r = 0.5f + 0.5f*cos(g_time*0.1347652371f);
	float g = 0.5f + 0.5f*sin(g_time);
	float b = 0.5f + 0.5f*cos(g_time);
	setBackgroundColour(r, g, b);
		
	// speed = 10 units per second
	float speed = 10.0f * dt;
	float aspeed = PI * dt;
	
	//exit fullscreen mode if Esc is pressed
	if(isKeyPressed(kKeyF1) && g_time > 0.5f)
	{
		g_time = 0;
		g_inGame = false;
		g_update = updateScreen2;
		g_draw = drawScreen2;
	}

	if(isKeyPressed(' ') && g_time > 0.5f && g_time >= g_timeCheck + 0.15) 
	{ 
		g_timeCheck = g_time;

		Matrix2 m;
		m.x = g_shipMX.x; 
		m.y = g_shipMX.y;
		m.w = g_shipMX.x * 0.0f + g_shipMX.y * 2.0f + g_shipMX.w;

		g_bulletMX.push_back(m);
	}

	// units per second (units/sec) 
	g_velocity = 20.0f * dt; 

	// compute new position of the bullet
	for (int i=0; i<g_bulletMX.size();i++)
	{
		g_bulletMX[i].w += g_bulletMX[i].y * g_velocity;
	}

	//if w key pressed move ships position up
	if(isKeyPressed('w') || isKeyPressed('W'))
	{
		g_shipMX.w += g_shipMX.y * speed;
	}
	//if s key pressed move ships position down
	if(isKeyPressed('s') || isKeyPressed('S'))
	{
		g_shipMX.w -= g_shipMX.y * speed;
	}
	//if w key pressed move ships position right
	if(isKeyPressed('d') || isKeyPressed('D'))
	{
		g_shipAngle -= aspeed;
	}
	//if w key pressed move ships position left
	if(isKeyPressed('a') || isKeyPressed('A'))
	{
		g_shipAngle += aspeed;
	}
	//if w key pressed move ships position straiffs right
	if(isKeyPressed('e') || isKeyPressed('E'))
	{
		g_shipMX.w += g_shipMX.x * speed;
	}
	//if w key pressed move ships position straiffs left
	if(isKeyPressed('q') || isKeyPressed('Q'))
	{
		g_shipMX.w -= g_shipMX.x * speed;
	}

	// compute rotation component of ship matrix
	float ca = cos(g_shipAngle);
	float sa = sin(g_shipAngle);
	g_shipMX.x.x =  ca;
	g_shipMX.x.y =  sa;
	g_shipMX.y.x = -sa;
	g_shipMX.y.y =  ca;
	
	//random asteroid Direction
	for(int i = 0; i < g_asteroidNumber; ++i)
	{
		g_asteroidMX[i].w += g_asteroidDirectionMX[i] * ((speed/4) + (g_time/10000));
		
	}

	//check for closest intersect
	float closest = 10000.0f;

	g_hit = -1;
	// loop through each bullet
	for(int j = 0; j < g_bulletMX.size(); ++j)
	{
		// loop through each asteroid
		for(int a = 0; a < g_asteroidNumber; ++a)
		{
			// loop through each line that makes up the asteroid
			for(int i = 1; i < sizeof(g_asteroidPoints) / sizeof(Vec2); ++i)
			{
				// These are the same two coordinates used to draw the line. 
				float isect = collisionCheck(g_bulletMX[j].w, g_bulletMX[j].y, g_asteroidPoints[i-1] + g_asteroidMX[a].w, g_asteroidPoints[i] + g_asteroidMX[a].w);
  
				// if intersection found (e.g. distance returned is not negative!)
				if(isect >= 0)
				{
					// we only care about the closest intersection!
					if(isect < closest)
					{
						closest = isect;
#if 0
						std::cout<<closest;
#endif
						if (closest < g_velocity)
						{
							g_score += 100;
							srand(time(NULL)); //uses the time to make a seed
							g_rand = rand() % 4;
							switch(g_rand)
							{
								case 0:
									g_asteroidMX[a].w.y = 17.0f;
									g_asteroidMX[a].w.x = randomFloat(-24, 24);
									break;
								
								case 1:
									g_asteroidMX[a].w.y = -17.0f;
									g_asteroidMX[a].w.x = randomFloat(-24, 24);
									break;

								case 2:
									g_asteroidMX[a].w.x = 24.0f;
									g_asteroidMX[a].w.y = randomFloat(-17, 17);
									break;

								case 3:
									g_asteroidMX[a].w.x = -24.0f;
									g_asteroidMX[a].w.y = randomFloat(-17, 17);
									break;
							}
							g_asteroidDirectionMX[a] = randDirectionUniformDist();
							g_hit = j;
						}
					}
				}
			}
		}
	}

	// loop through each bullet
	for(int j = 0; j < g_bulletMX.size(); ++j)
	{
		// loop through each line that makes up the ship
		for(int i = 1; i < sizeof(g_shipPoints) / sizeof(Vec2); ++i)
		{
			// These are the same two coordinates used to draw the line. 
			float isect = collisionCheck(g_bulletMX[j].w, g_bulletMX[j].y, g_shipPoints[i-1] + g_shipMX.w, g_shipPoints[i] + g_shipMX.w);
  
			// if intersection found (e.g. distance returned is not negative!)
			if(isect >= 0)
			{
				// we only care about the closest intersection!
				if(isect < closest)
				{
					closest = isect;
#if 0
					std::cout<<closest;
#endif
					if (closest < g_velocity)
					{
						g_lives--;
						g_hit = j;
					}
				}
			}
		}
	}

	// loop through each asteroid
	for(int a = 0; a < g_asteroidNumber; ++a)
	{
		// loop through each line that makes up the asteroid
		for(int i = 1; i < sizeof(g_asteroidPoints) / sizeof(Vec2); ++i)
		{
			// These are the same two coordinates used to draw the line. 
			float isect = collisionCheck(g_shipMX.w, g_shipMX.y, g_asteroidPoints[i-1] + g_asteroidMX[a].w, g_asteroidPoints[i] + g_asteroidMX[a].w);
  
			// if intersection found (e.g. distance returned is not negative!)
			if(isect >= 0)
			{
				// we only care about the closest intersection!
				if(isect < closest)
				{
					closest = isect;
#if 0
					std::cout<<closest;
#endif
					if (closest < g_velocity)
					{
						g_lives--;
						g_hit = -2;
						srand(time(NULL)); //uses the time to make a seed
							g_rand = rand() % 4;
							switch(g_rand)
							{
								case 0:
									g_asteroidMX[a].w.y = 17.0f;
									g_asteroidMX[a].w.x = randomFloat(-24, 24);
									break;
								
								case 1:
									g_asteroidMX[a].w.y = -17.0f;
									g_asteroidMX[a].w.x = randomFloat(-24, 24);
									break;

								case 2:
									g_asteroidMX[a].w.x = 24.0f;
									g_asteroidMX[a].w.y = randomFloat(-17, 17);
									break;

								case 3:
									g_asteroidMX[a].w.x = -24.0f;
									g_asteroidMX[a].w.y = randomFloat(-17, 17);
									break;
							}
						g_asteroidDirectionMX[a] = randDirectionUniformDist();
					}
				}
			}
		}
	}

	srand(time(NULL)); //uses the time to make a seed
	if (g_hit != -1 && g_hit != -2)
	{
		g_lasthit = g_bulletMX[g_hit].w;
		g_bulletMX.erase(g_bulletMX.begin() + g_hit);
		g_rand = rand() % 4;
	}
	else if (g_hit == -2)
	{
		g_lasthit = g_shipMX.w;
		g_rand = rand() % 4;
	}

	g_hit = -1;

	if (g_lives <= 0)
	{
		g_time = 0;
		g_inGame = false;
		g_update = updateScreen3;
		g_draw = drawScreen3;
	}

	//loops around the screen
	if (g_shipMX.w.y >= 16.0 || g_shipMX.w.y <= -16.0)
	{
		g_shipMX.w.y *= -1;
	}
	if (g_shipMX.w.x >= 22.0 || g_shipMX.w.x <= -22.0)
	{
		g_shipMX.w.x *= -1;
	}

	for(int i = 0; i < g_asteroidNumber; ++i)
	{
		if (g_asteroidMX[i].w.y >= 20.0 || g_asteroidMX[i].w.y <= -20.0)
		{
			g_asteroidMX[i].w.y *= -1;
		}
		if (g_asteroidMX[i].w.x >= 26.0 || g_asteroidMX[i].w.x <= -26.0)
		{
			g_asteroidMX[i].w.x *= -1;
		}
	}

	for(int i = 0; i < g_bulletMX.size(); ++i)
	{
		if (g_bulletMX[i].w.y >= 16.0 || g_bulletMX[i].w.y <= -16.0)
		{
			g_bulletMX[i].w.y *= -1;
		}
		if (g_bulletMX[i].w.x >= 21.0 || g_bulletMX[i].w.x <= -21.0)
		{
			g_bulletMX[i].w.x *= -1;
		}
	}
}

void drawScreen1()
{
	// draw something in 2D just so you can see it does actually work!
	drawShape(g_shipMX, g_ship);
	
	for(int i = 0; i < g_bulletMX.size(); ++i)
	{
		drawShape(g_bulletMX[i], g_bullet);
	}

	for(int i = 0; i < g_asteroidMX.size(); ++i)
	{
		drawShape(g_asteroidMX[i], g_asteroid);
	}
	srand(time(NULL)); //uses the time to make a seed
	int temp1,temp2,temp3;
	if (g_lasthit.y != 40.0f && g_lasthit.x != 40.0f)
	{
		temp1 = rand() % 101;
		temp2 = rand() % 101;
		temp3 = rand() % 101;
		setColour((float)temp1 * 0.1f , (float)temp2 * 0.1f, (float)temp3 * 0.1f);
		switch (g_rand)
		{
			case 0:
				drawText(g_lasthit.x, g_lasthit.y, "POOF");
			break;
			case 1:
				drawText(g_lasthit.x, g_lasthit.y, "POP");
			break;
			case 2:
				drawText(g_lasthit.x, g_lasthit.y, "POW");
			break;
			case 3:
				drawText(g_lasthit.x, g_lasthit.y, "PING");
			break;
		}
	}
	//score to screen
	setColour(0.0f, 0.0f, 0.0f);
	drawText(-19.0f, 14.0f, "Score: %i", g_score);
	drawText(17.0f, 14.0f, "Lives: %i", g_lives);
	drawText(-2.0f, 14.0f, "Hit F1 for Help");
}

void updateScreen2(float dt)
{
	g_time += dt;

	if(isKeyPressed(kKeyF1) && g_time > 0.5f)
	{
		g_time = 0;
		g_inGame = true;
		g_update = updateScreen1;
		g_draw = drawScreen1;
	}
	if(isKeyPressed(kKeyEscape) && g_time > 0.5f)
	{
		exit(0);
	}
}

void drawScreen2()
{
	setColour(1.0f, 0.0f, 1.0f);
	setBackgroundColour(0.0f, 1.0f, 1.0f);
	drawText(-2.0f, 14.0f, "HELP MENU!");
	drawText(-19.0f, 14.0f, "Hit F1 to return to the game");
	drawText(12.0f, 14.0f, "Hit ESCAPE to quit the game");
	drawText(-15.0f, 10.0f, "The aim of the game:");
	drawText(-15.0f, 9.0f, "The aim of the game is to shoot the asteroids, when you desrtoy them you increase the score.");
	drawText(-15.0f, 8.0f, "You have 3 lives, when you loose all of your lives then GAME OVER.");
	drawText(-15.0f, 7.0f, "You can loose your lives by either colliding with an asteroid.....");
	drawText(-15.0f, 6.0f, "OR you can be hit by your own bullets as when anything goes off the the side of the screen then it will reappear on the other side.");
	drawText(-15.0f, 4.0f, "Controls:");
	drawText(-15.0f, 3.0f, "A = Rotate left");
	drawText(-15.0f, 2.0f, "D = Rotate right");
	drawText(-15.0f, 1.0f, "W = Forwards");
	drawText(-15.0f, 0.0f, "S = Backwards");
	drawText(-15.0f, -1.0f, "Q = Strafe left");
	drawText(-15.0f, -2.0f, "E = Strafe right");
	drawText(-15.0f, -3.0f, "SPACE = Fires a bullet");
}

void updateScreen3(float dt)
{
	g_time += dt;

	if(isKeyPressed(' ') && g_time > 0.5f)
	{
		g_score = 0; 
		g_lives = 3;
		g_time = 0;
		g_lasthit = (40.0f,40.0f);
		g_timeCheck = 0;
		g_bulletMX.clear();
		g_inGame = true;
		for(int i = 0; i < g_asteroidNumber; ++i)
		{
			g_spawnCollision = true;
			while (g_spawnCollision)
			{
				g_asteroidMX[i].w = randVec2(g_min, -g_min);
				if (g_asteroidMX[i].w.y - 3.0f <= g_shipMX.w.y + 3.0f && g_asteroidMX[i].w.y + 3.0f >= g_shipMX.w.y - 3.0f 
					&& g_asteroidMX[i].w.x - 3.0f <= g_shipMX.w.x + 3.0f && g_asteroidMX[i].w.x + 3.0f >= g_shipMX.w.x - 3.0f)
				{
				}
				else
				{
					g_spawnCollision = false;
				}
			}
			g_asteroidDirectionMX[i] = randDirectionUniformDist();
		}
		g_update = updateScreen1;
		g_draw = drawScreen1;
	}
	if(isKeyPressed(kKeyEscape) && g_time > 0.5f)
	{
		exit(0);
	}
}

void drawScreen3()
{
	setColour(1.0f, 0.0f, 1.0f);
	setBackgroundColour(0.0f, 1.0f, 1.0f);
	drawText(-2.0f, 12.0f, "GAME OVER!");
	drawText(-2.0f, 8.0f, "SCORE: %i", g_score);
	drawText(-3.5f, 4.0f, "Press SPACE to play again");
	drawText(-0.5f, 3.0f, "OR");
	drawText(-3.0f, 2.0f, "Press ESCAPE to quit");
}


void update(float dt)
{
	// Use our global function pointer to call the correct update function
	g_update(dt);
}

Vec3 torus(float u, float v)
{
  float r1 = 2.0f;
  float r0 = 4.0f;
  Vec3 p;
  p.x = cos(u) * (r0 + r1 * cos(v));
  p.y = sin(u) * (r0 + r1 * cos(v));
  p.z = r1 * sin(v);
  return p;
}

void drawParametricSurface()
{
	// the min/max parametric values for the surface
	float umin = 0;
	float umax = TWO_PI;
	float vmin = 0;
	float vmax = TWO_PI;

	for(float u = umin; u <= umax; u += 0.05f)
	{
		for(float v = vmin; v <= vmax; v += 0.05f)
		{
			Vec3 P;
			P = torus(u, v);
			drawPoint(P);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------
// called when we want to draw the 3D data in our app.
//------------------------------------------------------------------------------------------------------------------------------------
void draw3D()
{
	static float fx = 0;
	static float fy = 0;
	rotate(fx += 0.02217362187, Vec3(1,0,0));
	rotate(fy += 0.01821653821, Vec3(0,1,0));
	if (g_inGame == true)
	{
		setColour(0.0f, 0.0f, 0.0f);
		drawParametricSurface();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------
// Called after draw3D. Allows you to draw a 2D overlay.
//------------------------------------------------------------------------------------------------------------------------------------
void draw()
{
	// Use our global function pointer to call the correct draw function
	g_draw();
}

//------------------------------------------------------------------------------------------------------------------------------------
/// called when the left mouse button is pressed
//------------------------------------------------------------------------------------------------------------------------------------
void mousePress(int /*button*/, int x, int y)
{
	Vec2 p = getScreenCoordinates(x, y);
}

//------------------------------------------------------------------------------------------------------------------------------------
/// called when the left mouse button is released
//------------------------------------------------------------------------------------------------------------------------------------
void mouseRelease(int /*button*/, int x, int y)
{
	Vec2 p = getScreenCoordinates(x, y);
}

//------------------------------------------------------------------------------------------------------------------------------------
/// called when the mouse is moved (and the left button is held)
//------------------------------------------------------------------------------------------------------------------------------------
void mouseMove(int x, int y)
{
	Vec2 p = getScreenCoordinates(x, y);
}

//------------------------------------------------------------------------------------------------------------------------------------
/// main  \o/
//------------------------------------------------------------------------------------------------------------------------------------
int main()
{
	// just run the app.... 
	return runApp();
}
