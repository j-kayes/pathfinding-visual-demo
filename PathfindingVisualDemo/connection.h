#pragma once
#include"vertex.h"
struct Vertex; // we need to let the compiler know of the existence of the vertex
struct Connection
{
	Vertex *node;
	float distance;
	Connection(Vertex *n, float dist)
		: node(n), distance(dist) {};
};
