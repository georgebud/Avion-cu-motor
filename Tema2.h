#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include "Camera.h"

#include <utility>
#include <cstdlib>
#include <ctime>

class Tema2 : public SimpleScene
{

	float FOV = 45.f;
public:
	Tema2();
	~Tema2();

	struct Triplet
	{
		float first, second, third;

		bool operator() (Triplet i, Triplet j)
		{
			return (i.first < j.first);
		}
	} triplet;

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

protected:
	Tema::Camera* camera;
	glm::mat4 projectionMatrix;
	bool renderCameraTarget;
	bool flagC = false;

	glm::vec3 lightPosition;
	unsigned int materialShininess;
	float materialKd;
	float materialKs;
	
	//airplane
	float rot_elice = 0;
	float rot_plane = 0;
	float speed_elice = 30;
	float speed_plane = 33;
	float planeX = -0.8, planeY = 1.2, planeZ = 0, translateY = 0;
	float constY = 1;

	//mouse cursor
	float dts;
	int prevY, currY;
	bool moveDown = false, moveUp = false;
	bool onGoing = false;

	//stars & energy bar
	std::vector<Mesh*> stars;
	float starX = -0.65;
	float barSpeed = 0.03;
	float energyX = 0.7;
	float energy = 1;
	float energyLost = 0.3;

	//clouds
	std::vector<Mesh*> clouds;
	std::vector<Triplet> positionCloud;
	float cloudX, cloudY, cloudZ;
	bool cloudMove = false;
	float cloudLimit = -15;
	int cloudNmb = 25;

	//obstacles
	std::vector<Triplet> obstaclePos;
	float obstaclesNmb = 9;
	float obstacleX;
	float obstacleLimit = -8;
	bool obstacleMove = false;
	float rot_obs = 0;

	//fuel
	std::vector<Triplet> fuelPos;
	float fuelBlocks;
	float fuelX;
	float fuelLimit = -4;
	bool fuelMove = false;
	float rot_fuel;

	//sea
	float rot_sea = 0;
	float seaY = 0;
	float sea_level = -11;
	bool sea_misc = false;

	bool GameOver = false;
	int counter = 0;
};
