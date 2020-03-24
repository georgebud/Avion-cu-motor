#include "Tema2.h"
#include "Transform3D.h"
#include "Obj2D.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include <Core/Engine.h>

using namespace std;


Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

int Random01()
{
	int random = (rand() % 10);
	return random % 2;
}

float RandomNb(float min, float max)
{
	assert(max > min);
	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;

	return min + (random * range);
}

bool SphereCollision(float x1, float y1, float z1, float radius1, float x2, float y2, float z2, float radius2)
{
	float Rx, Ry, Rz;
	Rx = x2 - x1;
	Ry = y2 - y1;
	Rz = z2 - z1;

	Rx *= Rx;
	Ry *= Ry;
	Rz *= Rz;

	float Res = Rx + Ry + Rz;

	float rSquared = radius1 + radius2;
	rSquared *= rSquared;

	return Res < rSquared;
}

void Tema2::Init()
{
	renderCameraTarget = false;

	camera = new Tema::Camera();
	camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

	srand(time(NULL));

	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

	// init airplane
	{
		{
			Mesh* mesh = new Mesh("airplane_body");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "airplane-body.obj");
			meshes[mesh->GetMeshID()] = mesh;
		}

		{
			Mesh* mesh = new Mesh("airplane_head");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "airplane-head.obj");
			meshes[mesh->GetMeshID()] = mesh;
		}

		{
			Mesh* mesh = new Mesh("airplane_elice");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "airplane-elice.obj");
			meshes[mesh->GetMeshID()] = mesh;
		}
	}
	
	// init clouds
	{
		for (int i = 0; i < cloudNmb; ++i) {
			Mesh* mesh;
			int num = Random01();

			if (num == 0) {
				mesh = new Mesh("cloud1");
				mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "nor1.obj");
				meshes[mesh->GetMeshID()] = mesh;
				clouds.push_back(mesh);
			}
			else if (num == 1) {
				mesh = new Mesh("cloud2");
				mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "nor2.obj");
				meshes[mesh->GetMeshID()] = mesh;
				clouds.push_back(mesh);
			}
			positionCloud.push_back({RandomNb(-5, 12), RandomNb(0, 4.2), RandomNb(-7, 2)});
		}
		std::sort(positionCloud.begin(), positionCloud.end(), triplet);

		cloudX = 0;
		cloudY = 0;
		cloudZ = 0;
	}

	// init obstacles
	{
		for (int i = 0; i < obstaclesNmb; ++i) {
			Mesh* mesh = new Mesh("asteroid");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "asteroyd.obj");
			meshes[mesh->GetMeshID()] = mesh;
			obstaclePos.push_back({RandomNb(5 + i, 20), RandomNb(0.5, 3), RandomNb(-0.2, 0.2) });
		}
		std::sort(obstaclePos.begin(), obstaclePos.end(), triplet);

		obstacleX = 0;
	}

	// init fuel blocks
	{
		fuelBlocks = (int)(RandomNb(1, 7));
		float x = RandomNb(5, 10);
		float y = RandomNb(0.5, 2.2);

		for (int i = 0; i < fuelBlocks; ++i) {
			Mesh* mesh = new Mesh("diamond");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "diamond.obj");
			meshes[mesh->GetMeshID()] = mesh;	
			x += 0.18;
			fuelPos.push_back({ x , y, 0 });
			if (i < 4)
				y -= 0.07;
			else
				y += 0.09;
		}

		fuelX = 0;
		rot_fuel = 0;
	}

	// init lifes
	{
		for (int i = 0; i < 3; ++i) {
			Mesh* mesh = new Mesh("star");
			mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "star.obj");
			stars.push_back(mesh);
		}
	}

	//init energy bar
	{
		Mesh* borderBar = Obj2D::CreateRectangle("borderBar", 0.22, 0.1);
		AddMeshToList(borderBar);
		
		Mesh* Bar = Obj2D::CreateRectangle("Bar", 0.21, 0.08);
		AddMeshToList(Bar);
		
		Mesh* energyBar = Obj2D::CreateRectangle("energyBar", 0.21, 0.08);
		AddMeshToList(energyBar);
	}

	//init sea
	{
		Mesh* mesh = new Mesh("sea");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere2.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	// Create a shader program for drawing face polygon with the color of the normal
	{
		Shader* shader = new Shader("ShaderTema2");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;

		shader = new Shader("2DShader");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/2DShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
		
		shader = new Shader("SeaShader");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/SeaVertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	//Light & material properties
	{
		lightPosition = glm::vec3(0, 1, 1);
		materialShininess = 80;
		materialKd = 0.5;
		materialKs = 0.5;
	}

}

void Tema2::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0.7647, 0.5019, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds)
{
	dts = deltaTimeSeconds;

	//cloud rendering
	{
		if (positionCloud[0].first > cloudLimit) {
			cloudMove = true;
		}
		else {
			for (int i = 0; i < cloudNmb - 1; i++) {
				positionCloud[i].first = positionCloud[i + 1].first;
				positionCloud[i].second = positionCloud[i + 1].second;
				positionCloud[i].third = positionCloud[i + 1].third;
				clouds[i] = clouds[i + 1];
			}
			positionCloud[cloudNmb - 1].first = positionCloud[cloudNmb - 2].first + RandomNb(0, 4);
			positionCloud[cloudNmb - 1].second = RandomNb(0, 4.2);
			positionCloud[cloudNmb - 1].third = RandomNb(-7, 1);
			
			Mesh* mesh;
			if (Random01() == 0) {
				clouds[cloudNmb - 1] = meshes["cloud1"];
			}
			else {
				clouds[cloudNmb - 1] = meshes["cloud2"];
			}
		}

		if (cloudMove) {
			//cloudX -= 0.005 * deltaTimeSeconds;
			cloudX = -2 * deltaTimeSeconds;
			cloudY = 0.08 * deltaTimeSeconds;
			cloudZ = 0.15 * deltaTimeSeconds;

			for (int i = 0; i < cloudNmb; ++i) {
				positionCloud[i].first += cloudX;
				
				if (positionCloud[i].first > -3) {
					positionCloud[i].second += cloudY;
				}
				else if (positionCloud[i].first < -3)
				{
					positionCloud[i].second -= cloudY;
				}

				positionCloud[i].third += cloudZ;

				glm::mat4 modelMatrix = glm::mat4(1);
				modelMatrix = Transform3D::Translate(positionCloud[i].first, positionCloud[i].second, positionCloud[i].third);
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.04));
				//RenderMesh(clouds[i], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.5607, 0.8227, 1));
				RenderMesh(clouds[i], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.8745, 0.9294, 0.9607));
			}
		}
	}

	// obstacle rendering
	{
		if (obstaclePos[0].first > obstacleLimit) {
			obstacleMove = true;
		}
		else {
			for (int i = 0; i < obstaclesNmb - 1; i++) {
				obstaclePos[i].first = obstaclePos[i + 1].first;
				obstaclePos[i].second = obstaclePos[i + 1].second;
				obstaclePos[i].third = obstaclePos[i + 1].third;
			}
			obstaclePos[obstaclesNmb - 1].first = obstaclePos[obstaclesNmb - 2].first + RandomNb(1, 5);
			obstaclePos[obstaclesNmb - 1].second = RandomNb(0.5, 2.5);
			obstaclePos[obstaclesNmb - 1].third = RandomNb(-0.10, 0.10);
		}

		if (obstacleMove) {
			obstacleX = -2.5 * deltaTimeSeconds;

			for (int i = 0; i < obstaclesNmb; ++i) {

				obstaclePos[i].first += obstacleX;
				rot_obs -= 0.5 * deltaTimeSeconds;

				glm::mat4 modelMatrix = glm::mat4(1);
				modelMatrix = Transform3D::Translate(obstaclePos[i].first, obstaclePos[i].second, obstaclePos[i].third);
				modelMatrix *= Transform3D::RotateOX(rot_obs);
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15));
				RenderMesh(meshes["asteroid"], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.3, 0.3, 0.3));


				if (SphereCollision(planeX + 0.22, planeY + translateY, planeZ, 0.20, obstaclePos[i].first, obstaclePos[i].second, obstaclePos[i].third, 0.08) && !GameOver) {
					for (int j = i; j < obstaclesNmb - 1; j++) {
						obstaclePos[j].first = obstaclePos[j + 1].first;
						obstaclePos[j].second = obstaclePos[j + 1].second;
						obstaclePos[j].third = obstaclePos[j + 1].third;
					}
					obstaclePos[obstaclesNmb - 1].first = obstaclePos[obstaclesNmb - 2].first + RandomNb(0.5, 2);
					obstaclePos[obstaclesNmb - 1].second = RandomNb(0.5, 2.5);
					obstaclePos[obstaclesNmb - 1].third = RandomNb(-0.10, 0.10);

					if (stars.size() > 1) {
						stars.resize(stars.size() - 1);
					}
					else if (stars.size() == 1) {
						printf("\n ----No lives remaining----\n\t<GAME OVER!>\n");
						GameOver = true;
						break;
					}
				}
			}

			if (GameOver) {
				if (stars.size() == 1)
					stars.resize(stars.size() - 1);

				cloudX = -0.5 * deltaTimeSeconds;

				if (rot_plane > -35) {
					rot_plane -= speed_plane * 2 * deltaTimeSeconds;
				}
				if (planeY + translateY > 0) {
					translateY -= 0.7 * deltaTimeSeconds;
				}
				else if (planeY + translateY > -5) {
					translateY -= 5 * deltaTimeSeconds;
					if (energy > 0) barSpeed += 0.1;
				}
			}
		}
	}

	// fuel blocks rendering
	{
		if (fuelPos.size() > 0 && fuelPos[0].first > fuelLimit) {
			fuelMove = true;
		}
		else {
			fuelBlocks = (int)(RandomNb(1, 9));
			fuelPos.clear();

			float x = RandomNb(5, 10);
			float y = RandomNb(0.5, 2.5);

			for (int i = 0; i < fuelBlocks; ++i) {
				x += 0.18;
				fuelPos.push_back({ x , y, RandomNb(-0.10, 0.10) });
				if (i < 4)
					y -= 0.07;
				else
					y += 0.09;
			}
		}

		if (fuelMove) {
			fuelX = -2 * deltaTimeSeconds;
			rot_fuel += 2.5 * deltaTimeSeconds;

			for (int i = 0; i < fuelPos.size(); ++i) {
				fuelPos[i].first += fuelX;

				glm::mat4 modelMatrix = glm::mat4(1);
				modelMatrix = Transform3D::Translate(fuelPos[i].first, fuelPos[i].second, fuelPos[i].third);
				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8));
				modelMatrix *= Transform3D::RotateOX(rot_fuel + i);
				RenderMesh(meshes["diamond"], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.611, 0.968, 0.250));
			
				if (SphereCollision(planeX + 0.22, planeY + translateY, planeZ, 0.20, fuelPos[i].first, fuelPos[i].second, fuelPos[i].third, 0.02) && !GameOver) {
					energy += 0.07;
					if (energy > 1)
						energy = 1;

					for (int j = i; j < fuelPos.size() - 1; j++) {
						fuelPos[j].first = fuelPos[j + 1].first;
						fuelPos[j].second = fuelPos[j + 1].second;
						fuelPos[j].third = fuelPos[j + 1].third;
					}
					//resize -1
					if (fuelPos.size() > 0)
						fuelPos.resize(fuelPos.size() - 1);
				}
			}
		}
	}

	//airplane rendering
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = Transform3D::Translate(planeX, planeY + translateY, planeZ);

		if (rot_plane < 0 && !moveUp && !moveDown) {
			rot_plane += speed_plane * deltaTimeSeconds;
			if (rot_plane > 0) rot_plane = 0;
		}
		else if (rot_plane > 0 && !moveUp && !moveDown) {
			rot_plane -= speed_plane * deltaTimeSeconds;
			if (rot_plane < 0) rot_plane = 0;
		}

		modelMatrix = glm::rotate(modelMatrix, RADIANS(rot_plane), glm::vec3(0, 0, 1));

		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7));
		RenderMesh(meshes["airplane_body"], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.8823, 0.8980, 0.2));
		RenderMesh(meshes["airplane_head"], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.23, 0.24, 0.1));

		rot_elice += -speed_elice * deltaTimeSeconds;
		modelMatrix *= Transform3D::RotateOX(rot_elice);
		RenderMesh(meshes["airplane_elice"], shaders["ShaderTema2"], modelMatrix, glm::vec3(0.8, 0.8, 0.8));
	}

	//sea rendering
	{
		rot_sea -= -8 * deltaTimeSeconds;

		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = Transform3D::Translate(0, sea_level, 0);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(5));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(5 + rot_sea), glm::vec3(0, 0, 1));
		RenderMesh(meshes["sea"], shaders["SeaShader"], modelMatrix, glm::vec3(0.264, 0.564, 0.849));

	}

	//star rendering
	{
		for (int i = 0; i < stars.size(); ++i) {
			glm::mat4 modelMatrix = glm::mat4(1);	
			modelMatrix = glm::scale(modelMatrix, glm::vec3(1.4));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(starX, 0.6, 0));
			RenderMesh(stars[0], shaders["2DShader"], modelMatrix, glm::vec3(1, 0.33, 0));
			starX += 0.05;
		}
		starX = -0.65;
	}

	//energy bar rendering
	{
		{
			if (energy > 0) {
				energy -= barSpeed * deltaTimeSeconds;
			}
			else if (energy > 1) {
				energy = 1;
			}
			else if (energy <= 0) { 
				cloudX = -0.5 * deltaTimeSeconds;
				GameOver = true;
				printf("\n----Fuel finised----\n\t<GAME OVER!>\n");

				if (rot_plane > -35) {
					rot_plane -= speed_plane * 2 * deltaTimeSeconds;
				}
				if (planeY + translateY > 0) {
					translateY -= 0.7 * deltaTimeSeconds;
				}
				else if (planeY + translateY > -5) translateY -= 5 * deltaTimeSeconds;
			}

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(energyX, 0.85, 0));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(energy, 1, 0));
			RenderMesh(meshes["energyBar"], shaders["2DShader"], modelMatrix, glm::vec3(0.07, 0.6, 0.05)); //energy bar
		}
		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(energyX, 0.85, 0));
			RenderMesh(meshes["Bar"], shaders["2DShader"], modelMatrix, glm::vec3(1, 1, 1)); //white bar
		}
		{
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(energyX - 0.005, 0.84, 0));
			RenderMesh(meshes["borderBar"], shaders["2DShader"], modelMatrix, glm::vec3(0, 0, 0)); //black bar
		}
	}

	// Camera positioning
	{
		if (flagC) {
			camera->Set(glm::vec3(planeX - 1.5, planeY + translateY + 0.3, planeZ + 0.02), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
			camera->RotateFirstPerson_OX(0.08);
			camera->RotateFirstPerson_OX(rot_plane * 0.02 * 0.2);
		}
	}

}

void Tema2::FrameEnd()
{
	//DrawCoordinatSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set shader uniforms for light & material properties
	// Set light position uniform
	GLint loc_light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3fv(loc_light_position, 1, glm::value_ptr(lightPosition));

	// Set eye position (camera position) uniform
	glm::vec3 eyePosition = GetSceneCamera()->transform->GetWorldPosition();
	GLint loc_eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3fv(loc_eye_position, 1, glm::value_ptr(eyePosition));

	// Set material property uniforms (shininess, kd, ks, object color) 
	GLint loc = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(loc, materialShininess);

	loc = glGetUniformLocation(shader->program, "material_kd"); //componenta difuza
	glUniform1f(loc, materialKd);

	loc = glGetUniformLocation(shader->program, "material_ks"); //componenta speculara
	glUniform1f(loc, materialKs);

	loc = glGetUniformLocation(shader->program, "object_color");
	glUniform3fv(loc, 1, glm::value_ptr(color));

	// Bind model matrix
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = camera->GetViewMatrix();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

// Documentation for the input functions can be found in: "/Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// move the camera only if MOUSE_RIGHT button is pressed
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float cameraSpeed = 2.0f;

		if (window->KeyHold(GLFW_KEY_W)) {
			//translate the camera forward
			camera->TranslateForward(deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			//translate the camera to the left
			camera->TranslateRight(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_S)) {
			//translate the camera backwards
			camera->TranslateForward(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			//translate the camera to the right
			camera->TranslateRight(deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_Q)) {
			//translate the camera down
			camera->TranslateUpward(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_E)) {
			//translate the camera up
			camera->TranslateUpward(deltaTime * cameraSpeed);
		}
	}
	
}

void Tema2::OnKeyPress(int key, int mods)
{
	// add key press event

	if (window->KeyHold(GLFW_KEY_C)) {
		flagC = !flagC;

		if (!flagC) { //Third Person mode
			camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		}
	}
}

void Tema2::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event

	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float sensivityOX = 0.001f;
		float sensivityOY = 0.001f;

		if (window->GetSpecialKeyState() == 0) {
			renderCameraTarget = false;
			// rotate the camera in First-person mode around OX and OY using deltaX and deltaY
			// use the sensitivity variables for setting up the rotation speed
			camera->RotateFirstPerson_OX(-1 * sensivityOX * deltaY);
			camera->RotateFirstPerson_OY(-1 * sensivityOY * deltaX);
		}

		if (window->GetSpecialKeyState() && GLFW_MOD_CONTROL) {
			renderCameraTarget = true;
			// rotate the camera in Third-person mode around OX and OY using deltaX and deltaY
			// use the sensitivity variables for setting up the rotation speed
			camera->RotateThirdPerson_OX(-1 * sensivityOX * deltaY);
			camera->RotateThirdPerson_OY(-1 * sensivityOY * deltaX);
		}

	}

	if (!GameOver) {
		if (onGoing) {
			currY = mouseY;
			if (currY > prevY) {
				//printf("DOWN Translation & Rotation\n");
				moveDown = true;
				moveUp = false;
				if (translateY > -0.85)
					translateY -= 0.005 * deltaY;

				if (rot_plane > -30) {
					rot_plane -= 0.2 * deltaY;
				}
			}
			else if (currY < prevY) {
				//printf("UP Translation & Rotation\n");
				moveUp = true;
				moveDown = false;
				if (translateY < 1.5)
					translateY -= 0.005 * deltaY;

				if (rot_plane < 30) {
					rot_plane -= 0.2 * deltaY;
				}
			}
			moveUp = false;
			moveDown = false;
			prevY = currY;
		}
		else {
			onGoing = true;
			currY = mouseY;
			prevY = currY;
		}
	}

}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}
