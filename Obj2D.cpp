#include "Obj2D.h"

#include <Core/Engine.h>
#include <math.h>

Mesh* Obj2D::CreateRectangle(std::string name, float width, float height)
{
	glm::vec3 corner = glm::vec3(0, 0, 0);

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner),
		VertexFormat(corner + glm::vec3(width, 0, 0)),
		VertexFormat(corner + glm::vec3(width, height, 0)),
		VertexFormat(corner + glm::vec3(0, height, 0))
	};

	Mesh* rectangle = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1, 2, 3, 0, 2 };
	rectangle->InitFromData(vertices, indices);
	

	return rectangle;
}