#include "vertex.h"
#include "connection.h"
#include <list>

bool compare_distances::operator() (const Vertex *lhs, const Vertex *rhs) const{
	if (lhs == rhs)
	{
		return (lhs < rhs); // This check is needed as an item is determined equal if both < and !< return false.
	}
	if (lhs->f_cost == rhs->f_cost)
	{
		return (lhs < rhs); // This is needed so that nodes with the same f-cost can be added to the set, this will break ties in f-cost by comparing addresses.
	}
	return (lhs->f_cost < rhs->f_cost); // Otherwise, compare distances rather than addresses.
}

void Connect(Vertex& a, Vertex& b, float distance) {
	a.connections.push_back(Connection(&b, distance));
	b.connections.push_back(Connection(&a, distance));
}


