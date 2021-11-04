#include "Precompiled.h"
#include "GridBasedGraph.h"

using namespace AI;

void GridBasedGraph::Initialize(int columns, int rows)
{
	mNodes.resize(columns * rows);
	mRows = rows;
	mColumns = columns;

	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < columns; ++x)
		{

			Node* n = GetNode(x, y);
			n->column = x;
			n->row = y;

			// TODO: Find a way to make it in a loop
			//for (int i = 0; i < GetNode(x, y)->neighbors.size(); ++i) {
			//	n->neighbors[i] = GetNode(x, y);
			//	n->neighbors.push_back( GetNode(x, y));
			//}

			n->neighbors[North] = GetNode(x, y-1);
			n->neighbors[South] = GetNode(x, y+1);
			n->neighbors[East] = GetNode(x+1, y);
			n->neighbors[West] = GetNode(x-1, y);
			n->neighbors[NorthEast] = GetNode(x+1, y-1);
			n->neighbors[NorthWest] = GetNode(x-1, y-1);
			n->neighbors[SouthEast] = GetNode(x+1, y+1);
			n->neighbors[SouthWest] = GetNode(x-1, y+1);
		}
	}

}

void GridBasedGraph::ResetSearchParams()
{
	for (auto& node : mNodes) {
			node.parent = nullptr;
			node.g = 0.0f;
			node.h = 0.0f;
			node.opened = false;
			node.closed = false;

	}
	//for (int i = 0; i < mNodes.size(); ++i)
	//{
	//	mNodes[i].parent = nullptr;
	//	mNodes[i].opened = false;
	//	mNodes[i].closed = false;
	//}
}

