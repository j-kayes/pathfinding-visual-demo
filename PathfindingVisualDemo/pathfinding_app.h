#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "pathfinding.h"
#include "pathfinding_app.h"

class PathfindingApp
{
private:
	Grid graph; // Our 26x20 grid/graph.
	Vertex *start_node;
	Vertex *end_node;
	Algorithm current_algorithm; // A value to determine what algorithm to use.
	sf::RenderWindow window;
	sf::RectangleShape squares[26][20];
	sf::RectangleShape panels[2];
	sf::Font font;
	std::string str_path_length;
	std::string str_pause_duration;
	std::string str_algorithm_duration;
	sf::Text text_dijkstras, text_astar_manhatten, text_astar_diagonal;
	std::vector<sf::RectangleShape> path_line;
	float path_length;
	float algorithm_duration;
	bool start_selected, end_selected, path_found;
	int start_x, start_y;
	int end_x, end_y;
	int speed_multiplier;

public:
	PathfindingApp();
	~PathfindingApp();

	void Run();
	Grid InitialiseGrid();
	void Draw();
	void ClearGrid();
	std::list<Vertex*> DijkstrasAlgorithm();
	std::list<Vertex*> AStarAlgorithm();
	float DiagonalDistance(Vertex* node);
	float ManhattanDistance(Vertex* node);
	std::vector<sf::RectangleShape> DrawPath(const std::list<Vertex*> path);
};

