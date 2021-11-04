#include "Precompiled.h"
#include "Dijkstra.h"

#include "GridBasedGraph.h"

using namespace AI;

bool Dijkstra::Search(GridBasedGraph& graph, int startX, int startY, int endX, int endY, 
	std::function<bool(int, int)> isBlocked,
	std::function<float(int, int,int,int)> getG
	)
{
	graph.ResetSearchParams();

	auto node = graph.GetNode(startX, startY);
	mOpenList.push_back(node);
	node->opened = true;

	auto endNode = graph.GetNode(endX, endY);

	bool found = false;
	//while (!mOpenList.empty()) // FOR SP TO ALL NODES
	while (!found && !mOpenList.empty()) 
	{
		GridBasedGraph::Node* nextNode = mOpenList.front();
		mOpenList.pop_front();

		if (nextNode == endNode)
		{
			found = true;
			mClosedList.push_back(nextNode);
		}
		else
		{
			int neighbors = static_cast<int>(nextNode->neighbors.size());
			float parentG = (nextNode->parent) ? nextNode->parent->g : 0.0f;

			for (int i = 0; i < neighbors; ++i) {
				if (!nextNode->neighbors[i] || 
					isBlocked(nextNode->neighbors[i]->column, nextNode->neighbors[i]->row) ||
					nextNode->neighbors[i]->closed)
					continue;

				float newCostG = parentG + getG(nextNode->column, nextNode->row, nextNode->neighbors[i]->column, nextNode->neighbors[i]->row);

				if (!nextNode->neighbors[i]->opened){
					nextNode->neighbors[i]->parent = nextNode;
					nextNode->neighbors[i]->opened = true;
					nextNode->neighbors[i]->g = newCostG;
					
					mOpenList.push_back(nextNode->neighbors[i]);
				}
				else if (!nextNode->neighbors[i]->closed) {
					if (newCostG < nextNode->neighbors[i]->g)
					{
						nextNode->neighbors[i]->parent = nextNode;
						nextNode->neighbors[i]->g = newCostG;
					}
				}


			}
			
			mOpenList.sort([](const GridBasedGraph::Node* a,const GridBasedGraph::Node* b) {return a->g < b->g; });

			nextNode->closed = true;
			mClosedList.push_back(nextNode);
		}
	}

	return found;
}


