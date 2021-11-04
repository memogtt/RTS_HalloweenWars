#include "Precompiled.h"
#include "BFS.h"

#include "GridBasedGraph.h"

using namespace AI;

bool BFS::Search(GridBasedGraph& graph, int startX, int startY, int endX, int endY, std::function<bool(int, int)> isBlocked)
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
			break;
		}
		else
		{
			int neighbors = static_cast<int>(nextNode->neighbors.size());
			for (int i = 0; i < neighbors; ++i) {
				if (!nextNode->neighbors[i] || isBlocked(nextNode->neighbors[i]->column, nextNode->neighbors[i]->row))
					continue;

				if (!(nextNode->neighbors[i]->opened || nextNode->neighbors[i]->closed)){
					nextNode->neighbors[i]->parent = nextNode;
					nextNode->neighbors[i]->opened = true;
					mOpenList.push_back(nextNode->neighbors[i]);
				}

			}
			nextNode->closed = true;
			mClosedList.push_back(nextNode);
		}
	}

	return found;
}


