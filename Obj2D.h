#pragma once

#include <string>

#include <include/glm.h>
#include <Core/GPU/Mesh.h>

namespace Obj2D
{
	// Create square with given bottom left corner, length and color
	Mesh* CreateRectangle(std::string name, float widt, float height);
}

