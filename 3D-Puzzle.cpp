// 3D-Puzzle.cpp: creates a world where the player spawns and the objective is to cross the river.

// Contributors: Garland Lau, Jay Dahiya

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <time.h>
#include <unordered_map>
#include <vector>
#include "Camera.h"
#include "Cubemap.h"
#include "Draw.h"
#include "IO.h"
#include "GLXtras.h"
#include "Mesh.h"
#include "Quaternion.h"
#include "VecMat.h"
#include "Widgets.h"
#include "Sprite.h"


// Movement
const float	KEY_DIST = .2f; // 1.5f;

// Timer
Timer endGameTimer(3);

// Options
bool followHuman = true;
bool showHumanReferenceFrame = false;
bool gameEnds = false;

// window, camera, color
int			winWidth = 1000, winHeight = 800;
Quaternion	initialOrientation(.7f, -.13f, .18f, -.67f);
Camera		camera(0, 0, winWidth, winHeight, initialOrientation, vec3(0, 0, -24));
vec3		wht(1, 1, 1), blk(0, 0, 0), red(1, 0, 0), blu(0, 0, 1);

// lights
vector<vec3> lights = { {-2.0f, 5.0f, 0.0f}, {0.0f, 5.0f, 0.0f}, {2.0f, 5.0f, 0.0f}, {-4.0f, 5.0f, 0.0f}, {-3.0f, 5.0f, 0.0f}, {1.0f, 5.0f, 0.0f}, {3.0f, 5.0f, 0.0f}, {-1.0f, 5.0f, 0.0f}, {10.0f, 5.0f, 0.0f} };


// human
Mesh human;
vec3 humanPosition = vec3(0, 0, 1);

// Platform
Mesh platform;
vec3 platformPosition = vec3(-0.5f, 0, -0.5f);

// Platform 2
Mesh woodPlatform;
vec3 woodPlatformPosition = vec3(-0.5f, 0, -0.5f);

// Other Meshes
Mesh grass;
vec3 grassPosition = vec3(0,0,1);

// Selector for camera
Mesh* selected = &human;

// cube map
CubeMap cubeMap;
Mesh ground;

// Screen
Sprite DeathScreen;
Sprite TestScreen;
Sprite WinScreen;

// Geometry
vec3 SelectedOrigin() { 
	mat4 &m = selected->toWorld; 
	return vec3(m[0][3], m[1][3], m[2][3]);
}

// Lava
vec3 lavaCenter = vec3(0, -2, -36);
float lavaRadius = 9.3f;

// game status
enum GameState { GS_Run, GS_Lose, GS_Win };
GameState gameState = GS_Run;

// Checks if player is on platform, which protects player from lava
bool IsPlayerOnPlatform() {

	float platformFront = 8.5f;
	float platformBack = -11.5f;
	float platformLeft = 39.5f;
	float platformRight = 41.5f;
	
	if ((humanPosition.z <= (platformPosition.z - platformBack)) && (humanPosition.z >= (platformPosition.z - platformFront))
		&& (humanPosition.x <= (platformPosition.x + platformRight)) && (humanPosition.x >= (platformPosition.x + platformLeft)))
	{return true;}
	else return false;
}

// Game over if player touches lava
bool IsPlayerTouchingLava() {
	vec3* position = &humanPosition;
	return (position->z <= lavaCenter.z + lavaRadius) && (position->z >= lavaCenter.z - lavaRadius) && !IsPlayerOnPlatform();
}

// Player wins if they cross the lava river
bool IsPlayerWin() {
	vec3* position = &humanPosition;
	if (position->z < lavaCenter.z - lavaRadius) { return true; }
	else return false;
}

// Creates starting scene, with platforms and human at starting points
// Resets game after death
void CreateScene() {
	float h = -2;
	humanPosition = vec3(0, 0, 1);
	platformPosition = vec3(-0.5f, 0, -0.5f);
	woodPlatformPosition = vec3(-0.5f, 0, -0.5f);
	// have human face away from the camera, feet are on the ground, move towards camera
	human.toWorld = RotateY(180) * Translate(0, -1, 0) * Translate(0, 0, -30);
	platform.toWorld = RotateX(-90) * Translate(40, -30, -1.97f) * Scale(7, 10, 7);
	woodPlatform.toWorld = RotateX(-90) * Translate(-40, -30, -1.97f) * Scale(7, 10, 7);
	ground.toWorld = Scale(100, 1, 100) * Translate(0, h, 0);
	grass.toWorld = Scale(5, 1, 5) * Translate(0, -2, 3);

	// place camera
	vec3 o = SelectedOrigin();
	vec3 eye = o - vec3(0, 0, -5);
	mat4 mv = LookAt(eye, o, vec3(0, 1, 0));
	camera.SetModelview(mv);
}

// display
void Display(GLFWwindow* w) {
	glClearColor(.4f, .4f, .8f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// transform, upload light
	int shader = UseMeshShader();
	SetUniform(shader, "nLights", 1);
	SetUniform3v(shader, "lights", lights.size(), (float*)lights.data(), camera.modelview);
	SetUniform(shader, "twoSidedShading", true);
	SetUniform(shader, "amb", .4f);

	// render scene
	cubeMap.Display(camera);
	ground.Display(camera, 1);
	human.Display(camera, 0);
	platform.Display(camera, 0);
	woodPlatform.Display(camera, 0);
	grass.Display(camera, 0);

	// annotations
	glDisable(GL_DEPTH_TEST);
	
	if (showHumanReferenceFrame) {
		UseDrawShader();
		Frame(human.toWorld, camera.modelview, camera.persp, .25f, wht);
	}
	
	gameState = IsPlayerTouchingLava() ? GS_Lose : IsPlayerWin() ? GS_Win : GS_Run;

	if (gameState == GS_Lose) {
		if (endGameTimer.paused)
			endGameTimer.Start();
		DeathScreen.Display();
		if (endGameTimer.Progress() >= 1.5) {
			gameState = GS_Run;
			endGameTimer.Reset();
			CreateScene();
		}
	}

	if (gameState == GS_Win) {
		if (endGameTimer.paused)
			endGameTimer.Start();
		WinScreen.Display();
		if (endGameTimer.Progress() >= 1.5) {
			gameState = GS_Run;
			endGameTimer.Reset();
			CreateScene();
		}
	}

	glFlush();
}

unordered_map<int, time_t> keyDowntime;

// Human player movement and world boundaries
void SetSelected(int k) {
	vec3 move;
	vec3* position = selected == &human ? &humanPosition : (selected == &platform ? &platformPosition : &woodPlatformPosition);
	float lim = 100.0f;
	if (k == 'W' && (position->z + KEY_DIST) > -lim)
		move = vec3(0, 0, -KEY_DIST);
	if (k == 'S' && (position->z + KEY_DIST) < lim-40.f)
		move = vec3(0, 0, KEY_DIST);
	if (k == 'A' && (position->x + KEY_DIST) > -lim+5.f)
		move = vec3(-KEY_DIST, 0, 0);
	if (k == 'D' && (position->x + KEY_DIST) < lim-5.f)
		move = vec3(KEY_DIST, 0, 0);
	selected->toWorld = Translate(move) * selected->toWorld;
	*position += move;
	if (followHuman)
		camera.Move(move);
}

void Stabilize(mat4& m) {
	// set y-axis in YZ plane
	vec3 xm = Vec3(m * vec4(1, 0, 0, 0)), ym = Vec3(m * vec4(0, 1, 0, 0)), zm = Vec3(m * vec4(0, 0, 1, 0)), o = Vec3(m * vec4(0, 0, 0, 1));
	vec3 yaxis(0, 1, 0);
	vec3 xms = normalize(cross(yaxis, zm));
	vec3 zms = normalize(zm);
	vec3 yms = normalize(cross(zms, xms));
	m = ReferenceFrame(xms, yms, zms, o);
}

void SetCamera(int k = 0) {
	// create transform to rotate scene around origin of selected mesh
	float deg = k == GLFW_KEY_DOWN ? -2.f : k == GLFW_KEY_LEFT ? -5.f : k == GLFW_KEY_UP ? 2.f : 5.f;
	mat4 m = k == GLFW_KEY_LEFT || k == GLFW_KEY_RIGHT ? RotateY(deg) :
	k == GLFW_KEY_DOWN || k == GLFW_KEY_UP ? RotateX(deg) : mat4();
	vec3 o = SelectedOrigin();
	if (selected != &human){
		mat4 m = selected->toWorld;
		o = Vec3(m * vec4(0, -1, 0, 1));
	}
	mat4 rm = Translate(o) * m * Translate(-o);

	// compute ground plane equation
	vec3 go = Vec3(ground.toWorld * vec4(0, 0, 0, 1));
	vec3 gn = normalize(Vec3(ground.toWorld * vec4(0, 1, 0, 0)));
	vec4 plane(gn.x, gn.y, gn.z, -dot(go, gn));

	// compute new camera location
	mat4 newModelview = camera.modelview * rm;
	if(false)
		Stabilize(newModelview);        // set transformed xaxis perpendicular to true vertical
	mat4 inv = Invert(newModelview);
	vec4 newPosition = inv * vec4(0, 0, 0, 1);

	// test if new position is above ground
	if (dot(newPosition, plane) > 0)
		camera.SetModelview(newModelview);
}

void CheckKeys() {
	int humanKeys[] = { 'W', 'A', 'S', 'D' };
	for (auto k : humanKeys) {
		time_t t = keyDowntime[k];
		if (KeyDown(k) && t > 0 && (float)(clock()-t)/CLOCKS_PER_SEC > .1f)
			SetSelected(k);
	}
	int camKeys[] = { GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_DOWN, GLFW_KEY_UP };
	for (auto k : camKeys) {
		time_t t = keyDowntime[k];
		if (KeyDown(k) && t > 0 && (float)(clock()-t)/CLOCKS_PER_SEC > .1f)
			SetCamera(k);
	}
}

// Callbacks
void Keyboard(int k, bool press, bool shift, bool control) {
	keyDowntime[k] = 0; 
	if (press) {
		keyDowntime[k] = clock();
		if (k == 'F') followHuman = !followHuman;
		if (k == 'R') showHumanReferenceFrame = !showHumanReferenceFrame;
		if (k == 'W' || k == 'A' || k == 'S' || k == 'D')
			SetSelected(k);
		if (k == GLFW_KEY_LEFT || k == GLFW_KEY_RIGHT || k == GLFW_KEY_DOWN || k == GLFW_KEY_UP)
			SetCamera(k);
		if (k == 'B') {
			//selected = selected == &human ? (selected == &woodPlatform ? &platform : &woodPlatform) : &human;
			if (selected == &human)
				selected = &platform;
			else if (selected == &platform)
				selected = &woodPlatform;
			else
				selected = &human;
			vec3 o = SelectedOrigin();
			vec3 h = Vec3(human.toWorld*vec4(0,0,0,1));
			o.y = h.y;
			vec3 eye = o - vec3(0, 0, -5);
			mat4 mv = LookAt(eye, o, vec3(0, 1, 0));
			camera.SetModelview(mv);
		}
	}
}

void Resize(int width, int height) { 
	camera.Resize(width, height);
	glViewport(0, 0, width, height); 
}

// Application
const char* usage = R"(
	wasd:              move human
	arrows:            rotate around selected object
	b:                 toggle camera perspectives
	f:                 toggle camera follow
	r:                 toggle show human reference frame
)";

// replace with your filepath
string filepath = "";

int main(int argc, char** argv) {
	GLFWwindow* w = InitGLFW(100, 100, winWidth, winHeight, "3D Puzzle");

	// read, transform cubemap
	cubeMap.Read(filepath + "/cubeMap.jpg");
	cubeMap.transform = Scale(100);// *RotateX(-90);

	// read human
	human.Read(filepath + "/human.obj", filepath + "/humanTexture.jpg");

	// read platform 1
	platform.Read(filepath + "/platform.obj", filepath + "/metalTexture.png");

	// read platform 2
	woodPlatform.Read(filepath + "/platform.obj", filepath + "/SeamlessPine.png");
	
	// read ground
	ground.Read(filepath + "/Square2.obj", filepath + "/Ground.png");

	// read grass
	grass.Read(filepath + "/grass.obj", filepath + "grassTexture.png");

	// Create Scene
	CreateScene();

	// read DeathScreen	
	DeathScreen.Initialize(filepath + "DeathScreen.png");
	DeathScreen.SetScale(vec2(1.0f, 1.0f));

	// read win screen
	WinScreen.Initialize(filepath + "/gameWin.jpg");
	WinScreen.SetScale(vec2(1.0f, 1.0f));
	
	// callbacks
	RegisterResize(Resize);
	RegisterKeyboard(Keyboard);
	printf("Usage:%s", usage);

	// event loop
	while (!glfwWindowShouldClose(w)) {
		CheckKeys();
		Display(w);
		glfwSwapBuffers(w);
		glfwPollEvents();
	}
	
}
