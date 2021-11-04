#include "Precompiled.h"
#include "AStar.h"

#include "GridBasedGraph.h"

using namespace AI;

bool AStar::Search(GridBasedGraph& graph, int startX, int startY, int endX, int endY, 
	std::function<bool(int, int)> isBlocked,
	std::function<float(int, int,int,int)> getG,
	std::function<float(int, int, int, int)> getH
)
{
	graph.ResetSearchParams();

	auto node = graph.GetNode(startX, startY);
	mOpenList.push_back(node);
	node->opened = true;

	auto endNode = graph.GetNode(endX, endY);

	bool found = false;
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
				float costH = newCostG + getH(nextNode->neighbors[i]->column, nextNode->neighbors[i]->row, endNode->column, endNode->row);


				if (!nextNode->neighbors[i]->opened) {
					nextNode->neighbors[i]->parent = nextNode;
					nextNode->neighbors[i]->opened = true;
					nextNode->neighbors[i]->g = newCostG;
					nextNode->neighbors[i]->h = costH;

					mOpenList.push_back(nextNode->neighbors[i]);
				}
				else if (!nextNode->neighbors[i]->closed) {
					if (costH < nextNode->neighbors[i]->h)
					{
						nextNode->neighbors[i]->g = newCostG;
						nextNode->neighbors[i]->parent = nextNode;
					}
				}


			}

			mOpenList.sort([](const GridBasedGraph::Node* a, const GridBasedGraph::Node* b) {return a->h < b->h; });

			nextNode->closed = true;
			mClosedList.push_back(nextNode);
		}
	}

	return found;
}


