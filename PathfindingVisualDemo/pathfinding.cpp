#include <iostream>
#include <set>
#include <cmath>
#include <algorithm>
#include <vector>
#include "vertex.h"
#include "pathfinding.h"
using namespace std;

// This will initialise a graph based grid of vertices connected with diagonals of length root 2 and other connections equal to 1:
Grid InitialiseGrid()
{
	Grid graph(26, vector<Vertex*>(20));
	// Fills grid:
	for (UInt32 h = 0; h < 20; h++)
	{
		for (UInt32 w = 0; w < 26; w++)
		{
			graph[w][h] = new Vertex(w, h);
		}
	}
	// Connects horizontal/verticals:
	for (UInt32 h = 0; h < 20; h++)
	{
		for (UInt32 w = 0; w < 26; w++)
		{
			if (w < 25)
			{
				Connect(*graph[w][h], *graph[w + 1][h], 1); // Connects horizontals.
			}
			if (h < 19)
			{
				Connect(*graph[w][h], *graph[w][h + 1], 1); // Connects verticals.
			}
		}
	}
	/*/
	// First diagonal.
	for (UInt32 h = 0; h < 19; h++) 
	{
		for (UInt32 w = 0; w < 25; w++)
		{
			Connect(*graph[w][h], *graph[w+1][h+1], kSquareRoot2);
		}
	}
	// Second diagonol.
	for (UInt32 h = 0; h < 19; h++)
	{
		for (UInt32 w = 1; w < 26; w++)
		{
			Connect(*graph[w][h], *graph[w - 1][h + 1], kSquareRoot2);
		}
	}
	/*/
	return graph;
}

std::list<Vertex*> DijkstrasAlgorithm(Vertex &start, Vertex &end)
{
	// compare_distances is a functor that orders the set by distance, rather than address.
	std::set<Vertex*, compare_distances> open_set; // Ordered from lowest distance/f-cost.
	std::set<Vertex*> closed_set;

	float current_distance = 0;
	Vertex* current_node;
	start.g_cost = current_distance; // Distance to start node is 0.
	start.f_cost = start.g_cost; // For dijkstras algorithm f-cost = g-cost as h-cost is 0.
	start.parent = nullptr; // Start node has no parent.
	bool no_path = false;

	open_set.insert(&start);
	while (no_path == false)
	{
		current_node = *open_set.begin(); // This gives the node with the lowest f-cost by selecting the last item.
		current_distance = current_node->g_cost;
		if (current_node == &end) 
		{
			break; // We have found a path.
		}
		open_set.erase(open_set.begin()); 
		closed_set.insert(current_node); // Mark current node as visited/add it to the closed set.
		for (auto connection_ : current_node->connections) // Loop through all the vertex connections (neigbours).
		{
			// If this node has already been added to the closed set, OR its blocked:
			if ((closed_set.find(connection_.node) != closed_set.end())|| (connection_.node->blocked == true)) 
			{
				continue; // Move on to the next node.
			} 
			else
			{
				float total_distance = current_distance + connection_.distance; // Calculate total distance to this node through the current_node node.
				if (open_set.find(connection_.node) == open_set.end()) // If the node is NOT in the open set.
				{
					open_set.insert(connection_.node); // Add this node to the open set.
					connection_.node->g_cost = total_distance; // Write the distance to it through this node.
					connection_.node->f_cost = connection_.node->g_cost; // For dijkstras algorithm f_cost = g_cost as h_cost is 0.
					connection_.node->parent = current_node;
				}
				else if (total_distance < connection_.node->g_cost) // If this node is in the open set and this path gives a shorter distance:
				{
					connection_.node->g_cost = total_distance; // Relax the distance.
					connection_.node->f_cost = connection_.node->g_cost;
					connection_.node->parent = current_node;
				}
			}
		}
		if (open_set.empty())
		{
			no_path = true;
		}
	}    
	// Trace path.
	std::list<Vertex*> path;
	Vertex *path_node = &end; // Current node being added to the path.
	if (no_path == false)
	{
		while (path_node != &start)
		{
			path.push_front(path_node); // Add to path.
			path_node = path_node->parent;
		}
		path.push_front(&start);
	}
	else
	{
		std::cout << "No Path." << std::endl << std::endl;
	}
	return path;
}

std::list<Vertex*> AStarAlgorithm(Vertex &start, Vertex &end, sf::RectangleShape graph[][20])
{
	// Compare_distances is a functor that orders the set by distance, rather than address.
	std::set<Vertex*, compare_distances> open_set; 
	std::set<Vertex*> closed_set;

	float current_distance = 0;
	Vertex* current_node;
	start.g_cost = current_distance; // Distance to start node is 0.
	start.h_cost = DiagonalDistance(&start, end);
	start.f_cost = start.g_cost + start.h_cost;
	start.parent = nullptr; // Start node has no parent
	bool no_path = false;

	open_set.insert(&start);
	graph[start.coordinates_.x][start.coordinates_.y].setFillColor(colour_open_set);
	while (no_path == false)
	{
		current_node = *open_set.begin(); // This gives the node with the lowest f-cost as the set is sorted by distance/f-cost.
		current_distance = current_node->g_cost; 
		if (current_node == &end)
		{
			break; // We have found a path.
		}
		open_set.erase(open_set.begin()); 
		closed_set.insert(current_node); // Mark current node as visited/add it to the closed set.
		graph[current_node->coordinates_.x][current_node->coordinates_.y].setFillColor(colour_closed_set);
		for (auto connection_ : current_node->connections) // Loop through all the vertex connections (neigbours).
		{
			// If this node has already been added to the closed set, OR its blocked:
			if ((closed_set.find(connection_.node) != closed_set.end()) || (connection_.node->blocked == true))
			{
				continue; // Move on to the next node.
			}
			else
			{
				float total_distance = current_distance + connection_.distance; // Calculate total distance to this node through the current_node.
				if (open_set.find(connection_.node) == open_set.end()) // If the node is NOT already in the open set.
				{
					connection_.node->g_cost = total_distance;
					connection_.node->h_cost = DiagonalDistance(connection_.node, end); // Estimated distance to end node.
					connection_.node->f_cost = connection_.node->g_cost + connection_.node->h_cost;
					open_set.insert(connection_.node); // Add this node to the open set.
					graph[connection_.node->coordinates_.x][connection_.node->coordinates_.y].setFillColor(colour_open_set);
					connection_.node->parent = current_node; // Set this nodes parent as the current node.
				}
				else if (total_distance < connection_.node->f_cost) // If this node is in the open set and this path gives a shorter distance:
				{
					connection_.node->g_cost = total_distance; // Relax the distance.
					connection_.node->f_cost = connection_.node->g_cost + connection_.node->h_cost; // Recalculate f-cost.
					connection_.node->parent = current_node;
				}
			}
		}
		if (open_set.empty())
		{
			no_path = true;
		}
	}
	// Trace path.
	std::list<Vertex*> path;
	Vertex *path_node = &end; // Current node being added to the path.
	if (no_path == false)
	{
		while (path_node != &start)
		{
			path.push_front(path_node); // Add to path.
			path_node = path_node->parent;
		}
		path.push_front(&start);
	}
	else
	{
		std::cout << "No Path." << std::endl;
	}
	return path;
}

// Heuristic:
float DiagonalDistance(Vertex* node, Vertex &end)
{
	// Absolute value of horizontal and vertical distance from this node to the end node.
	float dx = abs(static_cast<float>(node->coordinates_.x) - static_cast<float>(end.coordinates_.x)); // Casted to floats.
	float dy = abs(static_cast<float>(node->coordinates_.y) - static_cast<float>(end.coordinates_.y));
	return (dx + dy) + (kSquareRoot2 - 2) * min(dx, dy); // Takes diagonals in to account.
}