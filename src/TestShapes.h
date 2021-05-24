#pragma once

#include "sausage.h"
#include "Mesh.h"

Mesh* GetPlane() {
	static  std::vector<float> vertices = {
		0.5f,  0.5f, 0.0f,  // top right
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	static  std::vector<unsigned int> indices = {
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
	Mesh* mesh = CreateMesh(vertices, indices);
	return mesh;
}

Mesh* GetCube() {
	static  std::vector<Vertex> vertices = {
		// front
		Vertex{vec3(-1.0, -1.0,  1.0), vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(1.0, -1.0,  1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(1.0,  1.0,  1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(-1.0,  1.0,  1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		// back
		Vertex{vec3(-1.0, -1.0, -1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(1.0, -1.0, -1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(1.0,  1.0, -1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
		Vertex{vec3(-1.0,  1.0, -1.0, vec3(0), vec2(0), vec3(0), vec3(0)},
			};
	static  std::vector<unsigned int> indices = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	Mesh* mesh = CreateMesh(vertices, indices);
	return mesh;
}

