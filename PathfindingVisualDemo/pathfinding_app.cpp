#include "pathfinding.h"
#include "pathfinding_app.h"
#include <cassert>
#include <stdlib.h>
#include <vector>
#include <set>
#include <algorithm>
#include <string>

PathfindingApp::PathfindingApp() : window(sf::VideoMode(936, 720), "Pathfinding"), start_node(), end_node(), start_selected(false), end_selected(false),
	path_found(false), current_algorithm(DIJKSTRA), path_length(0), start_x(3), start_y(9), end_x(22), end_y(9), text_dijkstras("Dijkstras algorithm", font, 12),
	text_astar_manhatten("A* (Manhatten)", font, 12), text_astar_diagonal("A* (Diagonal)", font, 12), speed_multiplier(0)
{
	graph = InitialiseGrid();
	// Declare and load a font
	if (!font.loadFromFile("arial.ttf"))
	{
		exit(-1);
	}
	panels[0].setFillColor(sf::Color(0x00, 0x00, 0x00, 0x77));
	panels[0].setSize(sf::Vector2f(375.0f, 140.0f));
	panels[0].setPosition(sf::Vector2f(10.0f, 10.f));

	panels[1].setFillColor(sf::Color(0x00, 0x00, 0x00, 0x77));
	panels[1].setSize(sf::Vector2f(200.0f, 50.0f));
	panels[1].setPosition(sf::Vector2f(window.getSize().x - 210.0f, 10.0f));
}

void PathfindingApp::Run()
{
	// MAIN LOOP:
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					start_selected = false;
					end_selected = false;
				}
			}
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Space)
				{
					ClearGrid();
					path_found = false;
					path_length = 0;
				}
				if (event.key.code == sf::Keyboard::Return)
				{
					ClearGrid();
					path_found = false;
					path_length = 0;
					if (current_algorithm == DIJKSTRA)
					{
						std::list<Vertex*> path = DijkstrasAlgorithm();
						path_line = DrawPath(path);
					}
					else
					{
						std::list<Vertex*> path = AStarAlgorithm();
						path_line = DrawPath(path);
					}
				}
				if (event.key.code == sf::Keyboard::W)
				{
					switch (current_algorithm)
					{
					case A_STAR_DIAGONAL:
						current_algorithm = DIJKSTRA;
						break;
					case A_STAR_MANHATTAN:
						current_algorithm = A_STAR_DIAGONAL;
						break;
					case DIJKSTRA:
						current_algorithm = A_STAR_MANHATTAN;
						break;
					default:
						assert(false); // ERROR
						break;
					}
				}
				if (event.key.code == sf::Keyboard::S)
				{
					switch (current_algorithm)
					{
					case A_STAR_DIAGONAL:
						current_algorithm = A_STAR_MANHATTAN;
						break;
					case A_STAR_MANHATTAN:
						current_algorithm = DIJKSTRA;
						break;
					case DIJKSTRA:
						current_algorithm = A_STAR_DIAGONAL;
						break;
					default:
						assert(false); // ERROR
						break;
					}
				}
				if (event.key.code == sf::Keyboard::D)
				{
					if (speed_multiplier != 4)
					{
						speed_multiplier++; // This slows the rate of the algorithm by multiplying this factor in the sleep function.
					}
				}
				if (event.key.code == sf::Keyboard::A)
				{
					if (speed_multiplier != 0)
					{
						speed_multiplier--; // This speeds it up until it reaches realtime.
					}
				}
			}
		}
		Draw();
	}
}

Grid PathfindingApp::InitialiseGrid()
{
	Grid graph(26, std::vector<Vertex*>(20));
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
	Diagonals were not used in the final release as it was causing some errors with the algorithm, and I dont like the way it cuts corners anyway.
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
	}/*/
	return graph;
}

void PathfindingApp::Draw()
{
	window.clear(sf::Color(0xF9, 0xF9, 0xF9));
	for (int y = 0; y < 20; y++)
	{
		for (int x = 0; x < 26; x++)
		{
			squares[x][y].setPosition(sf::Vector2f(x*36.0f, y*36.0f));
			squares[x][y].setSize(sf::Vector2f(35.0f, 35.0f));
			squares[x][y].setOutlineThickness(1.0f);
			squares[x][y].setOutlineColor(sf::Color(0x44, 0x44, 0x44, 0x66));
			if (x == start_x && y == start_y)
			{
				squares[x][y].setFillColor(sf::Color::Green);
				start_node = graph[x][y];
			}
			else if (x == end_x && y == end_y)
			{
				squares[x][y].setFillColor(sf::Color::Red);
				end_node = graph[x][y];
			}
			else if (squares[x][y].getFillColor() != colour_blocked)
			{
				if ((squares[x][y].getFillColor() != colour_open_set) && (squares[x][y].getFillColor() != colour_closed_set))
				{
					squares[x][y].setFillColor(sf::Color::Transparent);
				}
			}
			if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
			{
				if ((sf::Mouse::getPosition(window).x <= squares[x][y].getPosition().x + squares[x][y].getLocalBounds().width) && (sf::Mouse::getPosition(window).x >= squares[x][y].getPosition().x))
				{
					if ((sf::Mouse::getPosition(window).y <= squares[x][y].getPosition().y + squares[x][y].getLocalBounds().height) && (sf::Mouse::getPosition(window).y >= squares[x][y].getPosition().y))
					{
						if (squares[x][y].getFillColor() == colour_blocked)
						{
							squares[x][y].setFillColor(sf::Color::Transparent);
							graph[x][y]->blocked = false;
						}
					}
				}
			}
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if ((sf::Mouse::getPosition(window).x <= squares[x][y].getPosition().x + squares[x][y].getLocalBounds().width) && (sf::Mouse::getPosition(window).x >= squares[x][y].getPosition().x))
				{
					if ((sf::Mouse::getPosition(window).y <= squares[x][y].getPosition().y + squares[x][y].getLocalBounds().height) && (sf::Mouse::getPosition(window).y >= squares[x][y].getPosition().y))
					{
						if (start_selected)
						{
							if (squares[x][y].getFillColor() == sf::Color::Transparent)
							{
								start_x = x;
								start_y = y;
								start_node = graph[x][y];
							}
						}
						else if (end_selected)
						{
							if (squares[x][y].getFillColor() == sf::Color::Transparent)
							{
								end_x = x;
								end_y = y;
								end_node = graph[x][y];
							}
						}
						else
						{
							if (squares[x][y].getFillColor() == sf::Color::Transparent)
							{
								squares[x][y].setFillColor(colour_blocked);
								graph[x][y]->blocked = true;
							}
							else if (squares[x][y].getFillColor() == sf::Color::Green)
							{
								start_selected = true;
							}
							else if (squares[x][y].getFillColor() == sf::Color::Red)
							{
								end_selected = true;
							}
						}
					}
				}
			}
			window.draw(squares[x][y]);
		}
	}
	if (path_found)
	{
		for (sf::RectangleShape line_segment : path_line)
		{
			window.draw(line_segment);
		}
	}
	for (auto panel : panels)
	{
		window.draw(panel);
	}

	// TEXT:
	str_path_length = "Path length: ";
	str_path_length += std::to_string(path_length); // This adds the number to the strin
	str_pause_duration = "Pause duration: ";
	str_algorithm_duration = "Algorithm duration: ";
	str_algorithm_duration += std::to_string(algorithm_duration);
	if (speed_multiplier == 0)
	{
		str_pause_duration = "Pause duration: REALTIME";
	}
	else
	{
		str_pause_duration += std::to_string(kPauseIncrement * speed_multiplier) + "ms";
	}
	sf::Text text_instruction1("Drag the RED and GREEN squares", font, 12);
	sf::Text text_instruction2("Use the LEFT and RIGHT mouse buttons to draw/remove obstacles", font, 12);
	sf::Text text_instruction3("Use the <W> and <S> keys to change the algorithm", font, 12);
	sf::Text text_instruction4("Use <A> and <D> keys to change algorithm speed", font, 12);
	sf::Text text_instruction5("Press <SPACE> to clear screen", font, 12);
	sf::Text text_instruction6("Press <RETURN> to run algorithm", font, 12);
	sf::Text text_pause_duration(str_pause_duration, font, 12);
	sf::Text text_path_length(str_path_length, font, 12);
	sf::Text text_algorithm_duration(str_algorithm_duration, font, 12);
	text_instruction1.setPosition(sf::Vector2f(15.0, 10.0f));
	text_instruction2.setPosition(sf::Vector2f(15.0f, 25.0f));
	text_instruction3.setPosition(sf::Vector2f(15.0f, 40.0f));
	text_instruction4.setPosition(sf::Vector2f(15.0f, 55.0f));
	text_instruction5.setPosition(sf::Vector2f(15.0f, 70.0f));
	text_instruction6.setPosition(sf::Vector2f(15.0f, 85.0f));
	text_pause_duration.setPosition(sf::Vector2f(15.0f, 100.0f));
	text_path_length.setPosition(sf::Vector2f(15.0f, 115.0f));
	text_algorithm_duration.setPosition(sf::Vector2f(15.0f, 130.0f));
	text_dijkstras.setPosition(sf::Vector2f(window.getSize().x - 200.0f, 40.0f));
	text_astar_diagonal.setPosition(sf::Vector2f(window.getSize().x - 200.0f, 10.0f));
	text_astar_manhatten.setPosition(sf::Vector2f(window.getSize().x - 200.0f, 25.0f));
	switch (current_algorithm)
	{
		case A_STAR_DIAGONAL:
			text_astar_diagonal.setColor(sf::Color::Red);
			text_astar_manhatten.setColor(sf::Color::White);
			text_dijkstras.setColor(sf::Color::White);
			break;
		case A_STAR_MANHATTAN:
			text_astar_diagonal.setColor(sf::Color::White);
			text_astar_manhatten.setColor(sf::Color::Red);
			text_dijkstras.setColor(sf::Color::White);
			break;
		case DIJKSTRA:
			text_astar_diagonal.setColor(sf::Color::White);
			text_astar_manhatten.setColor(sf::Color::White);
			text_dijkstras.setColor(sf::Color::Red);
			break;
		default:
			assert(false); // ERROR
			break;
	}

	window.draw(text_instruction1);
	window.draw(text_instruction2);
	window.draw(text_instruction3);
	window.draw(text_instruction4);
	window.draw(text_instruction5);
	window.draw(text_instruction6);
	window.draw(text_path_length);
	window.draw(text_algorithm_duration);
	window.draw(text_pause_duration);
	window.draw(text_astar_diagonal);
	window.draw(text_astar_manhatten);
	window.draw(text_dijkstras);
	window.display();
}

void PathfindingApp::ClearGrid()
{
	for (int y = 0; y < 20; y++)
	{
		for (int x = 0; x < 26; x++)
		{
			if (squares[x][y].getFillColor() != colour_blocked)
			{
				squares[x][y].setFillColor(sf::Color::Transparent);
			}
		}
	}
}

std::list<Vertex*> PathfindingApp::DijkstrasAlgorithm()
{
	sf::Clock timer; // A clock used to measure the time that the algorithm has been running.
	// compare_distances is a functor that orders the set by distance, rather than address:
	std::set<Vertex*, compare_distances> open_set; // Ordered from lowest distance/f-cost.
	std::set<Vertex*> closed_set;

	float current_distance = 0;
	Vertex* current_node;
	Vertex* next_node; // Vertex pointer to represent the next node to add.
	start_node->g_cost = current_distance; // Distance to start node is 0.
	start_node->f_cost = start_node->g_cost; // For dijkstras algorithm f-cost = g-cost as h-cost is 0. (just represents distance from start node)
	start_node->parent = nullptr; // Start node has no parent.
	bool no_path = false;

	open_set.insert(start_node);
	squares[start_node->coordinates_.x][start_node->coordinates_.y].setFillColor(colour_open_set); // This colours the square representing this node.
	while (no_path == false)
	{
		current_node = *open_set.begin(); // This gives the node with the lowest f-cost by selecting the first item in the set.
		current_distance = current_node->g_cost; // g-cost is the distance from the start node.
		if (current_node == end_node)
		{
			break; // We have found a path.
		}
		open_set.erase(open_set.begin());
		closed_set.insert(current_node); // Mark current node as visited/add it to the closed set.
		squares[current_node->coordinates_.x][current_node->coordinates_.y].setFillColor(colour_closed_set); // Change the nodes colour.
		for (auto connection_ : current_node->connections) // Loop through all the vertex connections (neigbours).
		{
			next_node = connection_.node; // The node that this connection leads to.
			// If this node has already been added to the closed set, OR its blocked:
			if ((closed_set.find(next_node) != closed_set.end()) || (next_node->blocked == true))
			{
				continue; // Move on to the next node.
			}
			else
			{
				float total_distance = current_distance + connection_.distance; // Calculate total distance to this node through the current_node.
				if (open_set.find(next_node) == open_set.end()) // If the node is NOT in the open set.
				{
					// Update g/h/f costs and then add to the open set
					squares[next_node->coordinates_.x][next_node->coordinates_.y].setFillColor(colour_open_set); // This should colour the square.
					next_node->g_cost = total_distance; // Write the distance to it through this node.
					next_node->f_cost = next_node->g_cost; // For dijkstras algorithm f_cost = g_cost as h_cost is 0.
					next_node->parent = current_node; // Set this node as it's parent
					open_set.insert(next_node); // Add this node to the open set.
				}
				else if (total_distance < next_node->g_cost) // If this node IS in the open set and if this path gives a shorter distance:
				{
					next_node->g_cost = total_distance; // Relax the distance.
					next_node->f_cost = next_node->g_cost;
					next_node->parent = current_node; // Set this node as it's parent
				}
			}
		}
		if (open_set.empty())
		{
			no_path = true;
		}
		sf::sleep(sf::milliseconds(kPauseIncrement*speed_multiplier)); // Wait for this long.
		Draw(); // Draw the progress for each iteration.
	}
	// Trace path.
	std::list<Vertex*> path;
	Vertex *path_node = end_node; // Current node being added to the path.
	if (no_path == false)
	{
		while (path_node != start_node)
		{
			path.push_front(path_node); // Add to path.
			path_node = path_node->parent; // Next node to add.
		}
		path.push_front(start_node);
		path_found = true;
		path_length = end_node->g_cost; // Path length is the final length to the end node.
	}
	else
	{
		path_found = false;
	}
	algorithm_duration = timer.getElapsedTime().asSeconds(); // Set this application variable
	return path;
}

std::list<Vertex*> PathfindingApp::AStarAlgorithm()
{
	sf::Clock timer; // A clock used to measure the time that the algorithm has been running.
	// compare_distances is a functor that orders the set by distance/ f-cost, rather than address.
	std::set<Vertex*, compare_distances> open_set;
	std::set<Vertex*> closed_set;

	float current_distance = 0; // Distance to start node is 0.
	Vertex* current_node;
	Vertex* next_node; // To hold a pointer to a node that this connects to.
	start_node->g_cost = current_distance; 
	if (current_algorithm == A_STAR_MANHATTAN) // This is to determine what heuristic to use.
	{
		start_node->h_cost = ManhattanDistance(start_node);
	}
	else if (current_algorithm == A_STAR_DIAGONAL)
	{
		start_node->h_cost = DiagonalDistance(start_node);
	}
	start_node->f_cost = start_node->g_cost + start_node->h_cost;
	start_node->parent = nullptr; // Start node has no parent
	bool no_path = false;

	open_set.insert(start_node);
	squares[start_node->coordinates_.x][start_node->coordinates_.y].setFillColor(colour_open_set);
	while (no_path == false)
	{
		current_node = *open_set.begin(); // This gives the node with the lowest f-cost as the set is sorted by distance/f-cost.
		current_distance = current_node->g_cost;
		if (current_node == end_node)
		{
			break; // We have found a path.
		}
		open_set.erase(open_set.begin());
		closed_set.insert(current_node); // Mark current node as visited/add it to the closed set.
		squares[current_node->coordinates_.x][current_node->coordinates_.y].setFillColor(colour_closed_set);

		for (auto connection_ : current_node->connections) // Loop through all the vertex connections (neigbours).
		{
			next_node = connection_.node; // Get the node that this connection leads to
			// If this node has already been added to the closed set, OR its blocked:
			if ((closed_set.find(next_node) != closed_set.end()) || (next_node->blocked == true))
			{
				continue; // Move on to the next node.
			}
			else
			{
				float total_distance = current_distance + connection_.distance; // Calculate total distance to this node through the current_node.
				if (open_set.find(next_node) == open_set.end()) // If the node is NOT already in the open set.
				{
					// Update g/h/f costs then add it to the open set:
					squares[next_node->coordinates_.x][next_node->coordinates_.y].setFillColor(colour_open_set); // This should colour the square.
					next_node->g_cost = total_distance;
					if (current_algorithm == A_STAR_MANHATTAN)
					{
						next_node->h_cost = ManhattanDistance(next_node); // Estimated distance to end node from this node.
					}
					else if (current_algorithm == A_STAR_DIAGONAL)
					{
						next_node->h_cost = DiagonalDistance(next_node);
					}
					next_node->f_cost = next_node->g_cost + next_node->h_cost;
					next_node->parent = current_node; // Set this node as it's parent
					open_set.insert(next_node); // Add this node to the open set.
				}
				else if (total_distance < next_node->f_cost) // If this node IS in the open set and this path gives a shorter distance:
				{
					next_node->g_cost = total_distance; // Relax the distance.
					next_node->f_cost = next_node->g_cost + next_node->h_cost; // Recalculate f-cost.
					next_node->parent = current_node; // Set this node as it's parent
				}
			}
		}
		if (open_set.empty())
		{
			no_path = true;
		}
		sf::sleep(sf::milliseconds(kPauseIncrement*speed_multiplier)); // Wait for this long.
		Draw(); // Draw the progress for each iteration.
	}
	// Trace path.
	std::list<Vertex*> path;
	Vertex *path_node = end_node; // Current node being added to the path.
	if (no_path == false)
	{
		while (path_node != start_node)
		{
			path.push_front(path_node); // Add to path.
			path_node = path_node->parent; // Next node to add.
		}
		path.push_front(start_node);
		path_found = true;
		path_length = end_node->g_cost; // Path length is the final length to the end node.
	}
	else
	{
		path_found = false;
	}
	algorithm_duration = timer.getElapsedTime().asSeconds(); // Set this application variable
	return path;
}

float PathfindingApp::DiagonalDistance(Vertex * node) // Heuristic (estimate of distance to endnode)
{
	// Absolute value of horizontal and vertical distance from this node to the end node.
	float dx = abs(static_cast<float>(node->coordinates_.x) - static_cast<float>(end_node->coordinates_.x)); // Casted to floats.
	float dy = abs(static_cast<float>(node->coordinates_.y) - static_cast<float>(end_node->coordinates_.y));
	return (dx + dy) + (kSquareRoot2 - 2) * std::min(dx, dy); // Takes diagonals in to account.
}

float PathfindingApp::ManhattanDistance(Vertex *node)
{
	float dx = abs(static_cast<float>(node->coordinates_.x) - static_cast<float>(end_node->coordinates_.x));
	float dy = abs(static_cast<float>(node->coordinates_.y) - static_cast<float>(end_node->coordinates_.y));
	return dx + dy;
}

std::vector<sf::RectangleShape> PathfindingApp::DrawPath(const std::list<Vertex*> path)
{
	std::vector<sf::RectangleShape> path_line;
	for (Vertex* node : path)
	{
		if (node->parent != nullptr) // If not the start node.
		{
			float x_difference = static_cast<float>(node->parent->coordinates_.x) - static_cast<float>(node->coordinates_.x); // We need to work these out to determine the direction of the path.
			float y_difference = static_cast<float>(node->parent->coordinates_.y) - static_cast<float>(node->coordinates_.y);
			if (x_difference == 0 && y_difference == 1) // Parent below.
			{
				float x_pos = node->coordinates_.x*36.0f + 18.5f;
				float y_pos = node->coordinates_.y*36.0f + 18.5f;
				sf::RectangleShape line_segment(sf::Vector2f(4.0f, 36.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == 0 && y_difference == -1) // Parent above.
			{
				float x_pos = node->coordinates_.x*36.0f + 18.5f;
				float y_pos = node->coordinates_.y*36.0f + 18.5f;
				sf::RectangleShape line_segment(sf::Vector2f(4.0f, 36.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(180); // Rotate 180 degrees so that it connects this node to it's parent.
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == 1 && y_difference == 0) // Parent to the right
			{
				float x_pos = node->coordinates_.x*36.0f + 18.5f;
				float y_pos = node->coordinates_.y*36.0f + 18.5f;
				sf::RectangleShape line_segment(sf::Vector2f(36.0f, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == -1 && y_difference == 0) // Parent to the left
			{
				float x_pos = node->coordinates_.x*36.0f + 18.5f;
				float y_pos = node->coordinates_.y*36.0f + 18.5f;
				sf::RectangleShape line_segment(sf::Vector2f(36.0f, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(180);
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == 1 && y_difference == 1) // Parent bottom-right.
			{
				float x_pos = 18.5f + 36.0f*node->coordinates_.x;
				float y_pos = 18.5f + 36.0f*node->coordinates_.y;
				sf::RectangleShape line_segment(sf::Vector2f(kDiagonalDistance, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(45);
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == -1 && y_difference == 1) // Parent bottom-left.
			{
				float x_pos = 18.5f + 36.0f*node->coordinates_.x;
				float y_pos = 18.5f + 36.0f*node->coordinates_.y;
				sf::RectangleShape line_segment(sf::Vector2f(kDiagonalDistance, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(135);
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == -1 && y_difference == -1) // Parent top-left.
			{
				float x_pos = 18.5f + 36.0f*node->coordinates_.x;
				float y_pos = 18.5f + 36.0f*node->coordinates_.y;
				sf::RectangleShape line_segment(sf::Vector2f(kDiagonalDistance, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(-135);
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
			else if (x_difference == 1 && y_difference == -1) // Parent top-right.
			{
				float x_pos = 18.5f + 36.0f*node->coordinates_.x;
				float y_pos = 18.5f + 36.0f*node->coordinates_.y;
				sf::RectangleShape line_segment(sf::Vector2f(kDiagonalDistance, 4.0f));
				line_segment.setPosition(sf::Vector2f(x_pos, y_pos)); // This should place this segment on the nodes position.
				line_segment.rotate(-45);
				line_segment.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xFF));
				path_line.push_back(line_segment);
			}
		}
	}
	return path_line;
}

PathfindingApp::~PathfindingApp()
{
}
