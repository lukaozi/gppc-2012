#include "SubgoalGraph.h"

SubgoalGraph::SubgoalGraph()
{
	search = MAX_SEARCH;
	finalized = false;

	traversable = NULL;
	subgoal = NULL;
	cellInfo = NULL;

	location = NULL;
	pruned = NULL;
	neighbors = NULL;
	nNeighbors = NULL;
	hasExtraEdge = NULL;

	generated = NULL;
	gCost = NULL;
	parent = NULL;
	open = NULL;

	dist = NULL;
	#ifdef PAIRWISE_MIDPOINTS
		next = NULL;
	#endif
}
#ifdef SG_RUNNING_IN_HOG
SubgoalGraph::SubgoalGraph(Map* map)
#else
SubgoalGraph::SubgoalGraph(std::vector<bool> &bits, int width, int height, const char *filename, double memoryLimit, double timeLimit)
#endif
{
	SubgoalGraph();

#ifdef SG_RUNNING_IN_HOG
	LoadMap(map);
#else
	LoadMap(bits, width, height);
#endif

	SetDirections();

#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif

	IdentifySubgoals();
	ComputeClearances();
	LinkSubgoals();

#ifdef SG_STATISTICS
	t.EndTimer();
	std::cout<<"Graph built in "<<t.GetElapsedTime()*1000<<"ms"<<std::endl;
	t.StartTimer();
#endif
	
#ifdef PRUNE_GRAPH
	PruneSubgoals();
#endif
	
#ifdef SG_STATISTICS
	t.EndTimer();
	std::cout<<"Graph pruned in "<<t.GetElapsedTime()*1000<<"ms"<<std::endl;
	PrintGraphStatistics();
#endif

	FinalizeGraph();
	MemoryAnalysis(memoryLimit);
	
#ifdef PAIRWISE_DISTANCES
	if (pairwiseUsage > 0)
		CalculatePairwiseDistances();
#endif

	SaveGraph(filename);
}

#ifdef SG_RUNNING_IN_HOG
void SubgoalGraph::LoadMap(Map* map)
{
	height = map->GetMapHeight() + 2;	// Add the padding
	width = map->GetMapWidth() + 2;

#else
void SubgoalGraph::LoadMap(std::vector<bool> &bits, int _width, int _height)
{
	height = _height + 2;	// Add the padding
	width = _width + 2;
#endif

	mapSize = height*width;

#ifdef USE_BOOL
	traversable = new bool[mapSize];
	for (unsigned int i = 0; i < mapSize; i++)
		traversable[i] = false;
#else
	unsigned int reducedMapSize = (mapSize + 7)/8;
	traversable = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		traversable[i] = 0;
#endif

	int nTraversable = 0;

	for (unsigned int x = 0; x < width-2; x++){
		for (unsigned int y = 0; y < height-2; y++){
#ifdef SG_RUNNING_IN_HOG
			if(map->GetTerrainType(x,y) == kGround)
#else
			if(bits[y*_width+x])
#endif
			{
				SetTraversable(ToMapLoc(xyLoc(x,y)));
				nTraversable++;
			}
		}
	}

#ifdef SG_STATISTICS
	std::cout<<"Height: "<<height<<std::endl;
	std::cout<<"Width: "<<width<<std::endl;
	std::cout<<"Map size: "<<mapSize<<std::endl;
	std::cout<<"Traversable cells: "<<nTraversable<<std::endl;
#endif
}

SubgoalGraph::SubgoalGraph(const char *filename)
{
	SubgoalGraph();
	LoadGraph(filename);
	if (!useSubgoals)	// If we will be using buckets
		return;
	
	finalized = true;
	search = 0;
	for (subgoalId sg = 0; sg < nSubgoals+2; sg++)
		generated[sg] = 0;

}
SubgoalGraph::~SubgoalGraph()
{
	if(neighbors)
	{
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if(neighbors[sg])
				delete [] neighbors[sg];
		}
	}

	if (dist)
	{
		for (subgoalId sg = 0; sg < nGlobalSubgoals; sg++)
		{
			if (dist[sg])		delete [] dist[sg];
		}
	}
#ifdef PAIRWISE_MIDPOINTS
	if (next)
	{
		for (subgoalId sg = 0; sg < nGlobalSubgoals; sg++)
		{
			if (next[sg])		delete [] next[sg];
		}
	}
#endif

	if(traversable)	delete [] traversable;
	if(subgoal)		delete [] subgoal;
	if(cellInfo)	delete [] cellInfo;

	if(location)	delete [] location;
	if(pruned)		delete [] pruned;
	if(neighbors)	delete [] neighbors;
	if(nNeighbors)	delete [] nNeighbors;
	if(hasExtraEdge)delete [] hasExtraEdge;

	if(generated)	delete [] generated;
	if(gCost)		delete [] gCost;
	if(parent)		delete [] parent;
	if(open)		delete [] open;

	//if(dist)	delete [] dist;
}

void SubgoalGraph::SetDirections()
{
	deltaMapLoc[0] = -width;	// North
	deltaMapLoc[2] = 1;			// East
	deltaMapLoc[4] = width;		// South
	deltaMapLoc[6] = -1;		// West
	deltaMapLoc[1] = deltaMapLoc[0] + deltaMapLoc[2];	// North-East
	deltaMapLoc[3] = deltaMapLoc[2] + deltaMapLoc[4];	// South-East
	deltaMapLoc[5] = deltaMapLoc[4] + deltaMapLoc[6];	// South-West
	deltaMapLoc[7] = deltaMapLoc[6] + deltaMapLoc[0];	// North-West

	// Create the extra copies
	for (direction d = 0; d < 8; d++)
	{
		deltaMapLoc[d + 8] = deltaMapLoc[d];
		deltaMapLoc[d + 16] = deltaMapLoc[d];
	}
}
void SubgoalGraph::IdentifySubgoals()
{
#ifdef USE_BOOL
	subgoal = new bool[mapSize];
	for (unsigned int i = 0; i < mapSize; i++)
		subgoal[i] = false;
#else
	unsigned int reducedMapSize = (mapSize + 7)/8;
	subgoal = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		subgoal[i] = 0;
#endif

	cellInfo = new subgoalId[mapSize];
	std::vector<xyLoc> locationVector;
	nSubgoals = 0;

	for (mapLoc l = 0; l < mapSize; l++)
	{
		if(IsTraversable(l))	// If the cell is traversable
		{
			for (direction d = 1; d <= 7; d+=2)	// Check its corners
			{
				// If there is an obstacle in the diagonal direction but no obstacles in the associated cardinal directions
				if(!IsTraversable(l + deltaMapLoc[d]) && IsTraversable(l + deltaMapLoc[d-1]) && IsTraversable(l + deltaMapLoc[d+1]))
				{
					// We have found a subgoal
					SetSubgoal(l);
					cellInfo[l] = nSubgoals;	// This will be the id of the subgoal
					nSubgoals++;
					locationVector.push_back(ToXYLoc(l));
					break;	// Don't look at any more corners
				}
			}
		}
	}

	nGlobalSubgoals = nSubgoals;	// initially every subgoal is global
	location = new xyLoc[nSubgoals+2];
	generated = new uint16_t[nSubgoals+2];	// +2 for possible start and goal states
	gCost = new cost[nSubgoals+2];
	parent = new subgoalId[nSubgoals+2];

#ifdef USE_BOOL
	pruned = new bool[nSubgoals+2];
	for (unsigned int i = 0; i < nSubgoals+2; i++)
		pruned[i] = false;
#else
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	pruned = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
		pruned[i] = 0;
#endif


#ifdef USE_BOOL
	open = new bool[nSubgoals + 2];
#else
	open = new char[(nSubgoals + 9)/8];
#endif

	for (unsigned int i = 0; i < nSubgoals; i++)
	{
		location[i] = locationVector[i];
	}
#ifdef SG_STATISTICS
	std::cout<<"Initial subgoal graph has "<<nSubgoals<<" subgoals."<<std::endl;
#endif
}
void SubgoalGraph::ComputeClearances()
{
	direction d = 0;	// North clearances
	for (int x = 0; x < (int)width-2; x++)
	{
		int clearance = 0;
		for (int y = 0; y < (int)height-2; y++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 4;	// South clearances
	for (int x = 0; x < (int)width-2; x++)
	{
		int clearance = 0;
		for (int y = (int)height-3; y >= 0; y--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 6;	// West clearances
	for (int y = 0; y < (int)height-2; y++)
	{
		int clearance = 0;
		for (int x = 0; x < (int)width-2; x++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 2;	// East clearances
	for (int y = 0; y < (int)height-2; y++)
	{
		int clearance = 0;
		for (int x = (int)width-3; x >= 0; x--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}
}
void SubgoalGraph::LinkSubgoals()
{
	std::vector<subgoalId> safeHReachableNeigbors;
	nNeighbors = new uint16_t[nSubgoals+2];
	neighbors = new subgoalId*[nSubgoals+2];
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		GetSafeHReachableSubgoals(location[sg], safeHReachableNeigbors);
		edgeVector.push_back(safeHReachableNeigbors);
		neighborhoodVector.push_back(safeHReachableNeigbors);
		nEdges += safeHReachableNeigbors.size();
	}
#ifdef SG_STATISTICS
	std::cout<<"Initial subgoal graph has "<<nEdges<<" edges."<<std::endl;
#endif
}
void SubgoalGraph::PruneSubgoals()
{
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		bool necessary = IsPruned(sg);	// This is just a short-cut to not do any of the following stuff if the subgoal is already pruned

		std::vector<subgoalId> neighbors = neighborhoodVector[sg];
		for (unsigned int i = 0; i+1 < neighborhoodVector[sg].size() && !necessary; i++)	// Try to find a pair of subgoals that needs this one
			for (unsigned int j = i+1; j < neighborhoodVector[sg].size() && !necessary; j++)
			{
				necessary = IsNecessaryToConnect(sg, neighborhoodVector[sg].at(i), neighborhoodVector[sg].at(j));
			}

		if (!necessary)	// If there is no reason not to prune this subgoal, prune it
		{
			PruneSubgoal(sg);
		}
	}

	// Treat start and goal as pruned subgoals
	SetPruned(nSubgoals);
	SetPruned(nSubgoals+1);
}
void SubgoalGraph::FinalizeGraph()
{

#ifdef USE_BOOL
	hasExtraEdge = new bool[nSubgoals + 2];
	for (unsigned int i = 0; i < nSubgoals + 2; i++)
		hasExtraEdge[i] = false;
#else
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
		hasExtraEdge[i] = 0;
#endif

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		nNeighbors[sg] = edgeVector[sg].size();
		neighbors[sg] = new subgoalId[nNeighbors[sg]+1];	// Allocate space for all of sg's neighbors + 1 for use during search

		for (unsigned i = 0; i < nNeighbors[sg]; i++)	// Fill the array, except for the last spot
			neighbors[sg][i] = edgeVector[sg].at(i);

		edgeVector[sg].clear();
		neighborhoodVector[sg].clear();
	}
	edgeVector.clear();
	neighborhoodVector.clear();

	finalized = true;
}
void SubgoalGraph::MemoryAnalysis(double memoryLimit)
{
	int bitsPerMapCell = 1 + 1 + 32;	
	// traversable, subgoal, cellInfo
	
	int bitsPerSubgoal = 32 + 32 + 16 + 32 + 32 + 32 + 16 + 1 + 1 + 1; 
	// location + neighborPtr + nNeighbor + extraEdge + gCost + parent + search counter + open + pruned + hasExtraEdge
	
	int bitsPerEdge = 32;
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		nEdges += nNeighbors[sg];
	
	double estimatedMapMemory = (bitsPerMapCell * mapSize)/(8*1024);	//kb
	double estimatedSubgoalMemory = (bitsPerSubgoal*(nSubgoals+2))/(8*1024);	//kb
	double estimatedEdgeMemory = (bitsPerEdge*nEdges)/(8*1024);	//kb
	double estimatedMemory = (bitsPerMapCell * mapSize + bitsPerSubgoal*(nSubgoals+2) + bitsPerEdge*nEdges)/(8*1024);	//kb
	
	double pairwiseSize = nGlobalSubgoals*(nGlobalSubgoals+1)/2;
	double costMatrixMemory = (pairwiseSize*32 + nGlobalSubgoals*32)/(8*1024);	//kb
	double parentMatrixMemory = (pairwiseSize*16 + nGlobalSubgoals*32)/(8*1024);	//kb

#ifdef SG_STATISTICS	
	std::cout<<"Memory limit: "<<memoryLimit/(double)1024<<" mb."<<std::endl;
	std::cout<<"Estimated memory usage (map): "<<estimatedMapMemory/(double)1024<<" mb."<<std::endl;
	std::cout<<"Estimated memory usage (subgoals): "<<estimatedSubgoalMemory/(double)1024<<" mb."<<std::endl;
	std::cout<<"Estimated memory usage (edges): "<<estimatedEdgeMemory/(double)1024<<" mb."<<std::endl;
	std::cout<<"Estimated memory usage (all): "<<estimatedMemory/(double)1024<<" mb."<<std::endl;
#ifdef PAIRWISE_DISTANCES
	std::cout<<"Estimated memory usage (with cost matrix): "<<(estimatedMemory + costMatrixMemory)/(double)1024<<" mb."<<std::endl;
	//std::cout<<"Estimated memory usage (with cost and parent matrices): "<<(estimatedMemory + costMatrixMemory + parentMatrixMemory)/(double)1024<<" mb."<<std::endl;
#endif
#endif
	
	useSubgoals = true;	
	pairwiseUsage = 0;
	
	if (memoryLimit <= estimatedMemory)
	{
#ifdef SG_STATISTICS
		std::cout<<"Memory limit exceeded, not using subgoals"<<std::endl;
#endif
		useSubgoals = false;
	}
#ifdef PAIRWISE_DISTANCES
	else if (estimatedMemory + costMatrixMemory + parentMatrixMemory < memoryLimit)
	{
#ifdef SG_STATISTICS
		std::cout<<"Enough space for cost and parent matrices"<<std::endl;
#endif
		pairwiseUsage = 2;
	}

	else if (estimatedMemory + costMatrixMemory < memoryLimit)
	{
#ifdef SG_STATISTICS
		std::cout<<"Enough space for cost matrix"<<std::endl;
#endif
		pairwiseUsage = 1;
	}
#endif
	else
	{
#ifdef SG_STATISTICS
		std::cout<<"Using the subgoal graph as it is"<<std::endl;	
#endif
	}
}

void SubgoalGraph::CalculatePairwiseDistances()
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	// First, rearrange the subgoals so that it is global ones followed by local ones (followed by the allocated start and goal slots)
	subgoalId front = 0;
	subgoalId back = nSubgoals-1;

	std::vector<subgoalId> subgoalMap;
	subgoalMap.resize(nSubgoals);

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		subgoalMap[sg] = sg;

	while(true)
	{
		while (!IsPruned(front) && front < back)
			front++;

		while (IsPruned(back) && back > front)
			back--;

		if (front < back)
		{
			subgoalMap[front] = back;
			subgoalMap[back] = front;

			SetUnpruned(front);
			SetPruned(back);

			int tempNNeighbors = nNeighbors[front];
			nNeighbors[front] = nNeighbors[back];
			nNeighbors[back] = tempNNeighbors;

			subgoalId* tempPtr = neighbors[front];
			neighbors[front] = neighbors[back];
			neighbors[back] = tempPtr;

			xyLoc tempXY = location[front];
			location[front] = location[back];
			location[back] = tempXY;

			cellInfo[ToMapLoc(location[front])] = front;
			cellInfo[ToMapLoc(location[back])] = back;
		}
		else
			break;
	}

	// Correct the edges as well
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		for (int i = 0; i < nNeighbors[sg]; i++)
			neighbors[sg][i] = subgoalMap[neighbors[sg][i]];

	// Floyd-Warshall algorithm to compute pairwise distances between all global subgoals

	// Set self-costs to 0; rest to infinity
	dist = new cost*[nGlobalSubgoals];

	for (int sg = 0; sg < nGlobalSubgoals; sg++)
	{
		dist[sg] = new cost[sg+1];
		for (int sg2 = 0; sg2 < sg; sg2++)
			dist[sg][sg2] = INFINITE_COST/2;

		dist[sg][sg] = 0;
	}

	// Add the edge costs
	for (int sg = 0; sg < nGlobalSubgoals; sg++)
	{
		for (int i = 0; i < nNeighbors[sg]; i++)
		{
			subgoalId sg2 = neighbors[sg][i];
			if (!IsPruned(sg2))
			{
				cost c = HCost(sg,sg2);
				if (sg > sg2)	dist[sg][sg2] = c;
				else			dist[sg2][sg] = c;
			}
		}
	}

	// Find the pairwise distances
	for (int sg = 0; sg < nGlobalSubgoals; sg++)
		for (int sg1 = 0; sg1 < nGlobalSubgoals; sg1++)
			for (int sg2 = 0; sg2 <= sg1; sg2++)
			{
				cost alt = (sg>sg1)?dist[sg][sg1]:dist[sg1][sg];
				alt += (sg>sg2)?dist[sg][sg2]:dist[sg2][sg];
				dist[sg1][sg2] = (dist[sg1][sg2] < alt)?dist[sg1][sg2]:alt;
			}

#ifdef PAIRWISE_MIDPOINTS
	// To travel from a subgoal to the other, choose the next subgoal to go
	next = new uint16_t*[nGlobalSubgoals];
	for (int sg1 = 0; sg1 < nGlobalSubgoals; sg1++)
	{
		next[sg1] = new uint16_t[sg1+1];
		for (int sg2 = 0; sg2 <= sg1; sg2++)
		{
			if (sg1 == sg2)
				next[sg1][sg2] = sg1;
				
			else	// look for a connecting neighbor
			{
				cost highCost = 0;
				for (int i = 0; i < nNeighbors[sg1]; i++)
				{
					int sg = neighbors[sg1][i];
					cost c1 = (sg>sg1)?dist[sg][sg1]:dist[sg1][sg];
					cost c2 = (sg>sg2)?dist[sg][sg2]:dist[sg2][sg];
					
					if (c1 + c2 == dist[sg1][sg2] && c1 > highCost)
					{
						highCost = c1;
						next[sg1][sg2] = sg;
					}
				}
			}
		}
	}
#endif
#ifdef SG_STATISTICS
	std::cout<<"Pairwise distances computed in "<<t.EndTimer()*1000<<"ms"<<std::endl;
#endif
}
void SubgoalGraph::SaveGraph(const char *filename)
{
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	
	out.write((char*)&useSubgoals,sizeof(bool));
	
	if (!useSubgoals)
		return;

	out.write((char*)&pairwiseUsage,sizeof(char));
	out.write((char*)&height,sizeof(unsigned int));
	out.write((char*)&width,sizeof(unsigned int));
	out.write((char*)&mapSize,sizeof(unsigned int));
	out.write((char*)&nSubgoals,sizeof(unsigned int));
	out.write((char*)&nGlobalSubgoals,sizeof(unsigned int));
	out.write((char*)&deltaMapLoc[0],sizeof(int)*24);

	out.write((char*)cellInfo, sizeof(subgoalId)*(mapSize));
#ifdef USE_BOOL
	out.write((char*)traversable, mapSize);
	out.write((char*)subgoal, mapSize);
	out.write((char*)pruned, nSubgoals+2);
#else
	unsigned int reducedMapSize = (mapSize + 7)/8;
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	out.write(traversable, reducedMapSize);
	out.write(subgoal, reducedMapSize);
	out.write(pruned, reducedNSubgoals);
#endif
	out.write((char*)location, sizeof(xyLoc)*(nSubgoals+2));
	out.write((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals+2));
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		//assert(!this->HasExtraEdge(sg));
		out.write((char*)neighbors[sg], sizeof(subgoalId)*nNeighbors[sg]);
	}

#ifdef PAIRWISE_DISTANCES
	if (pairwiseUsage > 0)
		for (int sg = 0; sg < nGlobalSubgoals; sg++)
			out.write((char*)dist[sg], sizeof(cost)*(sg+1));
#endif
	
	out.close();
}
void SubgoalGraph::LoadGraph(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	
	in.read((char*)&useSubgoals,sizeof(bool));
	if (!useSubgoals)
		return;
	
	in.read((char*)&pairwiseUsage,sizeof(char));
	in.read((char*)&height,sizeof(unsigned int));
	in.read((char*)&width,sizeof(unsigned int));
	in.read((char*)&mapSize,sizeof(unsigned int));
	in.read((char*)&nSubgoals,sizeof(unsigned int));
	in.read((char*)&nGlobalSubgoals,sizeof(unsigned int));
	in.read((char*)&deltaMapLoc[0],sizeof(int)*24);

	// Allocate space
	cellInfo = new subgoalId[mapSize];
#ifdef USE_BOOL
	traversable = new bool[mapSize];
	subgoal = new bool[mapSize];
	pruned = new bool[nSubgoals+2];
	open = new bool[nSubgoals+2];
	hasExtraEdge = new bool[nSubgoals+2];
	for (subgoalId sg = 0; sg < nSubgoals+2; sg++)
	{
		open[sg] = false;
		hasExtraEdge[sg] = false;
	}
#else
	unsigned int reducedMapSize = (mapSize + 7)/8;
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	traversable = new char[reducedMapSize];
	subgoal = new char[reducedMapSize];
	pruned = new char[reducedNSubgoals];
	open = new char[reducedNSubgoals];
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
	{
		open[i] = 0;
		hasExtraEdge[i] = 0;
	}
#endif
	location = new xyLoc[nSubgoals+2];
	nNeighbors = new uint16_t[nSubgoals+2];

	generated = new uint16_t[nSubgoals+2];
	gCost = new cost[nSubgoals+2];
	parent = new subgoalId[nSubgoals+2];

	in.read((char*)cellInfo, sizeof(subgoalId)*(mapSize));
#ifdef USE_BOOL
	in.read((char*)traversable, mapSize);
	in.read((char*)subgoal, mapSize);
	in.read((char*)pruned, nSubgoals+2);
#else
	in.read(traversable, reducedMapSize);
	in.read(subgoal, reducedMapSize);
	in.read(pruned, reducedNSubgoals);
#endif
	in.read((char*)location, sizeof(xyLoc)*(nSubgoals+2));
	in.read((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals+2));

	neighbors = new subgoalId*[nSubgoals+2];
	// Read the edges
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		neighbors[sg] = new subgoalId[nNeighbors[sg] + 1];	// + 1 for the extra edge during search
		in.read((char*)neighbors[sg], sizeof(subgoalId)*nNeighbors[sg]);
	}

#ifdef PAIRWISE_DISTANCES
	// Read the pairwise distances (if written)
	if (pairwiseUsage > 0)
	{
		dist = new cost*[nGlobalSubgoals];
		for (int sg = 0; sg < nGlobalSubgoals; sg++)
		{	
			dist[sg] = new cost[sg+1];
			in.read((char*)dist[sg], sizeof(cost)*(sg+1));
		}
	}
#endif

#ifdef SG_STATISTICS			
	std::cout<<"Graph read from file.."<<std::endl;
#endif
	initializeTime = 0.0;
	searchTime = 0.0;
	finalizeTime = 0.0;

	in.close();
}

void SubgoalGraph::GetMoveDirectionDetails(xyLoc & from, xyLoc & to, direction & c, direction & d, int & nTotal, int & nTotalDiag)
{
	int dx = (from.x>to.x)?(from.x-to.x):(to.x-from.x);
	int dy = (from.y>to.y)?(from.y-to.y):(to.y-from.y);

	nTotal = (dx>dy)?(dx):(dy);	// total number of diagonal and cardinal moves that must be made to reach the target
	nTotalDiag = (dx<dy)?(dx):(dy);	// total number of diagonal moves that must be made to reach the target

	c = (dx>dy)?((from.x<to.x)?2:6):((from.y<to.y)?4:0);	// only make this kind of cardinal moves
	d = (from.x<to.x)?((from.y<to.y)?3:1):((from.y<to.y)?5:7);	// only make this kind of diagonal moves
}
bool SubgoalGraph::IsHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nTotalCard, nTotalDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nTotalDiag);
	nTotalCard = nTotal-nTotalDiag;

	int *diagCount = new int[nTotal+1];
	diagCount[0] = 0;
	for(int i = 1; i <= nTotal; i++)
		diagCount[i] = -1;

	mapLoc loc = ToMapLoc(from);
	int i = 0;
	while (i < nTotal)
	{
		//if we can move cardinally, do it
		if (i - diagCount[i] < nTotalCard && diagCount[i+1] < diagCount[i] && IsTraversable(loc + deltaMapLoc[c]))	//0 <= x+cardX && x+cardX < width && 0 <= y+cardY && y+cardY < height &&
		{
			diagCount[i+1] = diagCount[i];
			i++;
			loc += deltaMapLoc[c];
		}

		//else, if we can move diagonally, do it
		else if (diagCount[i] < nTotalDiag && diagCount[i+1] <= diagCount[i] && IsTraversable(loc + deltaMapLoc[d]) && IsTraversable(loc + deltaMapLoc[d-1]) && IsTraversable(loc + deltaMapLoc[d+1]))
		{
			diagCount[i+1] = diagCount[i]+1;
			i++;
			loc += deltaMapLoc[d];
		}

		//else, backtrack
		else
		{
			if (i == 0)	// cannot backtrack
			{
				delete [] diagCount;
				return false;
			}

			i--;

			if (diagCount[i] == diagCount[i+1])
				loc -= deltaMapLoc[c];
			else
				loc -= deltaMapLoc[d];
		}
	}

	loc = ToMapLoc(from);
	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	for (int i = 1; i <= nTotal; i++)
	{
		if (diagCount[i] > diagCount[i-1])	// Made a diagonal move
			loc += deltaMapLoc[d];
		else
			loc += deltaMapLoc[c];

		path.push_back(loc);
	}

	delete [] diagCount;
	return true;
}
bool SubgoalGraph::IsQuickHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nDiag);
	nCard = nTotal-nDiag;

	mapLoc loc = ToMapLoc(from);

	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	while (nCard)
	{
		loc += deltaMapLoc[c];
		if (!IsTraversable(loc))
			return false;

		path.push_back(loc);
		nCard--;
	}

	while (nDiag)
	{
		loc += deltaMapLoc[d];
		if (!IsTraversable(loc) || !IsTraversable(loc + deltaMapLoc[d+3]) || !IsTraversable(loc + deltaMapLoc[d+5]))
			return false;

		path.push_back(loc);
		nDiag--;
	}

	return true;
}
bool SubgoalGraph::IsLookaheadHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nDiag);
	nCard = nTotal-nDiag;

	/*
	 		6 	3
		7	5	2
		C	4	1

		Current is the current position of the agent, assume it is trying to move north/nort-east.
		It can either move to 4 or 5.
		It can observe any of the numbered cells to determine its movement (observing 1,2, and 3 is the lookahead)

		Note that the loop constraint forces us to have at least one diagonal and at least one cardinal move remaining
		Therefore, if the given start and goal states are in the map limits, we don't need to check if 1,2,4,5, or 7 is in the map bounds
	 */

	int offset1 = deltaMapLoc[c] + deltaMapLoc[c];
	int offset2 = deltaMapLoc[c] + deltaMapLoc[d];
	//int offset3 = deltaMapLoc[d] + deltaMapLoc[d];
	int offset4 = deltaMapLoc[c];
	int offset5 = deltaMapLoc[d];
	//int offset6 = offset3 - deltaMapLoc[c];
	int offset7 = offset5 - deltaMapLoc[c];

	mapLoc loc = ToMapLoc(from);


	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}

	while (nDiag && nCard)
	{
		if (!IsTraversable(loc + offset4))	// If 4 is an obstacle, no movement possible
			return false;

		else if (!IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))	// If 5 or 7 is blocked, we can not move diagonally
		{																								// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset1))	// If 1 is an obstacle and we move cardinally, then we can't move on the next turn
		{															// so, we move diagonally
			loc += offset5; nDiag--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset2))	// If 2 is an obstacle and we move diagonally, then we can't move on the next turn
		{															// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else	// Whether 6 or 3 is an obstacle does not force us to move a certain way, so we make a heuristic decision between cardinal and diagonal
		{
			if (nCard > nDiag)	//If we have more cardinal moves remaining, prefer cardinal move
			{
				loc += offset4; nCard--;	path.push_back(loc);
			}
			else
			{
				loc += offset5; nDiag--;	path.push_back(loc);
			}
		}
	}

	// We have only cardinal or only diagonal moves (or no moves) remaining, proceed quickly without lookahead
	while(nDiag)
	{
		if(!IsTraversable(loc + offset4) || !IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))
			return false;

		loc += offset5; nDiag--;	path.push_back(loc);
	}

	while(nCard)
	{
		if(!IsTraversable(loc + offset4))
			return false;

		loc += offset4; nCard--;	path.push_back(loc);
	}

	return true;
}

void SubgoalGraph::GetSafeHReachableSubgoals(xyLoc & from, std::vector<subgoalId> & subgoals)
{
	mapLoc origin = ToMapLoc(from);
	subgoals.clear();
	int clearance[9];

	// Compute cardinal clearances
	for (direction c = 0; c <= 6; c+=2)
	{
		clearance[c] = 0;

		mapLoc loc = origin + deltaMapLoc[c];
		while (IsTraversable(loc) && !IsSubgoal(loc))
		{
			loc += deltaMapLoc[c];
			clearance[c]++;
		}

		if (IsSubgoal(loc))
			subgoals.push_back(ToSubgoalId(loc));
	}

	clearance[8] = clearance[0];	// Report North twice, to avoid using the modulus operation

	// Compute diagonal clearances
	for (direction d = 1; d <= 7; d+=2)
	{
		clearance[d] = 0;

		mapLoc loc = origin + deltaMapLoc[d];
		while ( IsTraversable(loc) && !IsSubgoal(loc) &&
				IsTraversable(loc + deltaMapLoc[d+3]) && 	// loc is already updated with the diagonal move, so the associated
				IsTraversable(loc + deltaMapLoc[d+5]))		// cardinal directions are +3, +5 instead of +1 +7 (-7 == -1 % 8)
		{
			loc += deltaMapLoc[d];
			clearance[d]++;
		}

		if (IsSubgoal(loc) && IsTraversable(loc + deltaMapLoc[d+3]) && IsTraversable(loc + deltaMapLoc[d+5]))
			subgoals.push_back(ToSubgoalId(loc));
	}

	/* Now, explore the 8 areas that the cardinal and diagonal separators create.
	 * Each area is at most as large as a parallelogram whose dimensions are the
	 * clearances of its associated cardinal and diagonal directions.
	 */
	for (direction d = 1; d <= 7; d+=2)
	{
		for (direction c = d-1; c <= d+1; c+=2)
		{
#ifdef USE_CLEARANCES
			//int maxExt = GetClearance(from,c);
			int maxExt = clearance[c]+1;
			mapLoc loc = origin;
#else
			int maxExt = clearance[c];
#endif

			for (int i = 1; i <= clearance[d]; i++)	// Go along the diagonal, from each cell on the diagonal ..
			{
#ifdef USE_CLEARANCES
				loc +=deltaMapLoc[d];
				int cardClearance = GetTotalClearance(loc,c);
				//int cardClearance = GetClearance(loc,c);
				if (cardClearance < maxExt)
				{
					//if (cardClearance == CLEARANCE_LIMIT)
					//{
					//	cardClearance = GetTotalClearance(loc,c);
					//}
					
					mapLoc loc2 = loc + deltaMapLoc[c]*cardClearance;
					if (IsSubgoal(loc2))
						subgoals.push_back(ToSubgoalId(loc2));
				}

				maxExt = (maxExt < cardClearance)?(maxExt):(cardClearance);
#else
				mapLoc loc = origin + deltaMapLoc[d]*i + deltaMapLoc[c];	// .. start sweeping along the cardinal direction
				int ext = 0;	// we may not have to explore the whole area

				while (ext < maxExt && IsTraversable(loc) && !IsSubgoal(loc))	// may be sped up with precomputed cardinal clearances
				{
					loc += deltaMapLoc[c];
					ext++;
				}

				if (ext < maxExt && IsSubgoal(loc))
					subgoals.push_back(ToSubgoalId(loc));

				if (ext < maxExt)
					maxExt = ext;
#endif
			}
		}
	}
}

cost SubgoalGraph::CostOtherPath(subgoalId & sg, subgoalId & sg1, subgoalId & sg2, cost limit)
{
	// If sg2 is already pruned, make sure it is reachable by adding the relevant global-to-local edges
	if(IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			subgoalId tempSg = edgeVector[sg2].at(i);
			edgeVector[tempSg].push_back(sg2);
		}
	}

	// Temporarily remove all the outgoing edges from sg, essentially removing it from the search// Temporarily remove all the outgoing edges from sg, essentially removing it from the search
	std::vector<subgoalId> edges = edgeVector[sg];
	edgeVector[sg].clear();

	/* Do the search and get the cost. Since we are only checking if we can do better
	 * than costThrough, add a search limit to stop the search if we exceed costThrough
	 */
	cost alternateCost = SubgoalAStarSearch(sg1,sg2, limit);
	//cost alternateCost = SubgoalAStarSearch(sg1,sg2);

	// Add back the outgoing edges from sg
	edgeVector[sg] = edges;

	// If sg2 is already pruned, make sure to remove the global-to-local edges we added earlier
	if(IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			edgeVector[edgeVector[sg2].at(i)].pop_back();
		}
	}
	return alternateCost;
}
bool SubgoalGraph::IsNecessaryToConnect(subgoalId sg, subgoalId sg1, subgoalId sg2)
{
	/* We want to make sure that if sg is pruned, then sg1 and sg2 will still be somehow connected optimally.
	 */

	/* Method 1: If h(sg1,sg2) == h(sg1,sg) + h(sg,sg2), then that means
	 * sg1 and sg2 are h-reachable, therefore sg is not necessary to connect
	 */
	cost costThrough = HCost(sg1, sg) + HCost(sg, sg2);
	if (costThrough == HCost(sg1, sg2))
		return false;

	if (IsHReachable(location[sg1], location[sg2]))
		return false;

	/* Method 5 (last resort): Remove sg from the graph, do an A* search from sg1 to sg2.
	 * If the path found is larger then h(sg1,sg) + h(sg,sg2), then sg is necessary to connect
	 */

	if (CostOtherPath(sg, sg1,sg2, costThrough+1) <= costThrough)
		return false;

	return true;
}
void SubgoalGraph::PruneSubgoal(subgoalId sg)
{
	nGlobalSubgoals--;
	SetPruned(sg);
	std::vector<subgoalId> neighbors = neighborhoodVector[sg];

	// Since this is now a local subgoal, remove all incoming edges
	for (unsigned int i = 0; i< neighborhoodVector[sg].size(); i++)
	{
		RemoveEdge(neighborhoodVector[sg].at(i), sg);	// Note that they will only be removed from the edgeVectors
	}

	// Add the necessary connections between surrounding subgoals
	for (unsigned int i = 0; i+1 < neighborhoodVector[sg].size(); i++)
		for (unsigned int j = i+1; j < neighborhoodVector[sg].size(); j++)
		{
			subgoalId sg1 = neighborhoodVector[sg].at(i);
			subgoalId sg2 = neighborhoodVector[sg].at(j);

			if (!IsPruned(sg1) || !IsPruned(sg2))
			{
				cost hCost = HCost(sg1,sg2);
				// Only connect sg1-sg2 if sg1 and sg2 are h-reachable and sg is on an h-reachable path
				bool connect = (hCost == HCost(sg,sg1) + HCost(sg,sg2));
/*
				// Do not connect if there is another connecting subgoal
				for (unsigned int i = 0; connect && i < neighborhoodVector[sg1].size(); i++)
				{
					subgoalId sg3 = neighborhoodVector[sg1].at(i);
					if (!IsPruned(sg3) && sg3 != sg)
						for (unsigned int j = 0; connect && j < neighborhoodVector[sg2].size(); j++)
						{
							if(sg3 == neighborhoodVector[sg2].at(j))
							{
								if(hCost == HCost(sg1, sg3) + HCost(sg3,sg2))
								{
									connect = false;
								}
								break;
							}
						}
				}
*/
				if (connect && (CostOtherPath(sg, sg1, sg2, hCost+1) != hCost))
				{
					AddEdge(sg1,sg2);
					AddEdge(sg2,sg1);
				}
			}

		}

	// Finally, remove all the local-to-local edges
	for (unsigned int i = 0; i< edgeVector[sg].size(); i++)
	{
		if(IsPruned(edgeVector[sg].at(i)))
		{
			edgeVector[sg].at(i) = edgeVector[sg].back();
			edgeVector[sg].pop_back();
			i--;
		}
	}
}
void SubgoalGraph::AddEdge(subgoalId sg1, subgoalId sg2)
{
	for (unsigned int i = 0; i < neighborhoodVector[sg1].size(); i++)
	{
		if (neighborhoodVector[sg1].at(i) == sg2)
			return;
	}

	neighborhoodVector[sg1].push_back(sg2);
	if (!IsPruned(sg2))
	{
		edgeVector[sg1].push_back(sg2);
	}
}
void SubgoalGraph::RemoveEdge(subgoalId sg1, subgoalId sg2)
{
	for (unsigned int i = 0; i < edgeVector[sg1].size(); i++)
	{
		if (edgeVector[sg1].at(i) == sg2)
		{
			edgeVector[sg1].at(i) = edgeVector[sg1].back();
			edgeVector[sg1].pop_back();
			return;
		}
	}
}

void SubgoalGraph::AddToOpen(subgoalId sg, cost fVal)
{
#ifdef REPLACE_POPPED
	if(canReplaceTop)	// If the top element of the heap can be replaced,
	{
		theHeap[0] = (heapElement(sg,fVal));	// .. replace it
		PercolateDown(0);		// and percolate down
		canReplaceTop = false;	// the top element is no longer replaceable
	}
	else
#endif
	{	// add element as usual
		theHeap.push_back(heapElement(sg,fVal));
		PercolateUp(theHeap.size()-1);
	}
}
void SubgoalGraph::PopMin()
{
#ifdef REPLACE_POPPED
	canReplaceTop = true;	// Don't pop it immediately, just mark it as replaceable
#else
	theHeap[0] = theHeap.back();
	theHeap.pop_back();
	PercolateDown(0);
#endif
}
#ifdef REPLACE_POPPED
void SubgoalGraph::PopReplacableTop()
{	// Force the heap to remove the top element, without waiting for a replacement
	if (canReplaceTop)
	{
		theHeap[0] = theHeap.back();
		theHeap.pop_back();
		PercolateDown(0);
		canReplaceTop = false;
	}
}
#endif
heapElement SubgoalGraph::GetMin()
{
	return theHeap.front();
}

void SubgoalGraph::ResetSearch()
{
	// Set last search and generated values to 0, so that when the search is incremented, all the states will be not-generated
	search = 0;
	for (subgoalId sg = 0; sg < nSubgoals+2; sg++)
		generated[sg] = 0;
}
void SubgoalGraph::PercolateUp(int index)
{
	heapElement elem = theHeap[index];
	int parent;
	parent = (index-1) >> 1;

	while(index > 0 && theHeap[parent].fVal > elem.fVal)
	{
		theHeap[index] = theHeap[parent];
		index = parent;
		parent = (index-1) >> 1;
	}

	theHeap[index] = elem;
}
void SubgoalGraph::PercolateDown(int index)
{
	heapElement elem = theHeap[index];
	int maxSize = theHeap.size();

	while(true)
	{
		int child1 = (index << 1) + 1;
		if (child1 >= maxSize)
			break;

		int child2 = child1+1;

		// If the first child has smaller key
		if (child2 == maxSize || theHeap[child1].fVal <= theHeap[child2].fVal)
		{
			if (theHeap[child1].fVal < elem.fVal)
			{
				theHeap[index] = theHeap[child1];
				index = child1;
			}
			else
				break;
		}

		else if (theHeap[child2].fVal < elem.fVal)
		{
			theHeap[index] = theHeap[child2];
			index = child2;
		}
		else
			break;
	}

	theHeap[index] = elem;
}

void SubgoalGraph::ToXYLocPath(std::vector<mapLoc> & mapLocPath, std::vector<xyLoc> & xyLocPath)
{
	xyLocPath.clear();
	for (unsigned int i = 0; i < mapLocPath.size(); i++)
	{
		xyLocPath.push_back(ToXYLoc(mapLocPath[i]));
	}
}
cost SubgoalGraph::SubgoalAStarSearch(subgoalId & start, subgoalId & goal, cost searchLimit, std::vector<subgoalId> & abstractPath)
{
	if (search >= MAX_SEARCH)
		ResetSearch();

	search++;
	theHeap.clear();
	theStack.clear();

#ifdef REPLACE_POPPED
	canReplaceTop = false;
#endif

	generated[start] = search;
	gCost[start] = 0;
	SetOpen(start);
	AddToOpen(start, HCost(start,goal));

	generated[goal] = search;
	gCost[goal] = searchLimit;	// If this point is reached, end the search early (assume no path found)
	SetOpen(goal);
	// DO NOT ADD GOAL TO THE OPEN LIST YET

	#ifdef SG_STATISTICS
		int nHeapExpansions = 0;
		#ifdef USE_STACK
			int nStackExpansions = 0;
		#endif
	#endif

	subgoalId* successors;
	unsigned int nSuccessors;
	subgoalId sg;

#ifdef USE_STACK
	//heapElement* bestElement = &theHeap[0];
	cost currFCost;

	//while(bestElement && bestElement->fVal < gCost[goal])
	while((!theStack.empty() && gCost[goal] > theStack[0].fVal) || (!theHeap.empty() && gCost[goal] > theHeap[0].fVal))	// 0-1 Expansions per loop
#else
	while(!theHeap.empty() && gCost[goal] > theHeap[0].fVal)	// 0-1 Expansions per loop
#endif
	{
		// Select the element to expand
		#ifdef USE_STACK
			if (!theStack.empty())	// If the stack has elements, expand the top one
			{
				sg = theStack.back().sg;
				currFCost = theStack.back().fVal;
				theStack.pop_back();
				#ifdef SG_STATISTICS
					nStackExpansions++;
				#endif
			}
			else	// Expand from the heap
			{
				sg = theHeap[0].sg;
				currFCost = theHeap[0].fVal;
				PopMin();
				#ifdef SG_STATISTICS
					nHeapExpansions++;
				#endif
			}
		#else	// We are only using the heap
			sg = theHeap[0].sg;
			PopMin();
			#ifdef SG_STATISTICS
				nHeapExpansions++;
			#endif
		#endif

		if(IsOpen(sg))	// If it has been closed already, don't re-expand it
		{
			// Expand the state

			SetClosed(sg);

			// Get the successors
			if (finalized)	// If the graph is finalized, use 'neighbors' to get the successors
			{
				successors = neighbors[sg];
				nSuccessors = nNeighbors[sg];
			}
			else	// Else, use 'edgeVector' to get the successors
			{
				successors = edgeVector[sg].data();
				nSuccessors = edgeVector[sg].size();
			}

			// Go over all the successors
			for (unsigned int i = 0; i < nSuccessors; i++)
			{
				subgoalId succ = successors[i];

				// Generate the successor, if it has not been generated for this search
				if(generated[succ] != search)
				{
					generated[succ] = search;
					gCost[succ] = gCost[sg] + HCost(sg, succ);
					parent[succ] = sg;
					SetOpen(succ);

					// Insert it into the open list
					#ifdef USE_STACK
						if (gCost[succ] + HCost(succ,goal) == currFCost)
							theStack.push_back(heapElement(succ, gCost[succ] + HCost(succ,goal)));
						else
							AddToOpen(succ, gCost[succ] + HCost(succ,goal));
					#else
						AddToOpen(succ, gCost[succ] + HCost(succ,goal));
					#endif
				}

				// If it was already generated and is not closed
				else if (IsOpen(succ))
				{
					cost newGCost = gCost[sg] + HCost(sg,succ);
					if (newGCost < gCost[succ])
					{
						gCost[succ] = newGCost;
						parent[succ] = sg;

						// Insert it into the open list (may add code to delete the old version with the higher f-value from the open list)
						#ifdef USE_STACK
							if (gCost[succ] + HCost(succ,goal) == currFCost)
								theStack.push_back(heapElement(succ, gCost[succ] + HCost(succ,goal)));
							else
								AddToOpen(succ, gCost[succ] + HCost(succ,goal));
						#else
							AddToOpen(succ, gCost[succ] + HCost(succ,goal));
						#endif
					}
				}
			}
		}
		#ifdef REPLACE_POPPED
			PopReplacableTop();
		#endif
	}

	#ifdef SG_STATISTICS
	#ifdef SG_STATISTICS_PER_SEARCH
		if (finalized)
		{
			std::cout<<"Heap expansions: "<<nHeapExpansions<<std::endl;
			#ifdef USE_STACK
				std::cout<<"Stack expansions: "<<nStackExpansions<<std::endl;
			#endif
			std::cout<<"Open list size: "<<theHeap.size() + theStack.size()<<std::endl;
		}
	#endif
	#endif

	if (gCost[goal] != searchLimit)	// Follow the parent pointers to extract the path
	{
		abstractPath.clear();
		subgoalId cur = goal;
		while (cur != start)
		{
			abstractPath.push_back(cur);
			cur = parent[cur];
		}

		abstractPath.push_back(cur);
		std::reverse(abstractPath.begin(), abstractPath.end());
		return gCost[goal];
	}
	else
	{
		return INFINITE_COST;
	}
}
void SubgoalGraph::ConnectStartAndGoalToGraph(
		xyLoc & start, xyLoc & goal, subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & extraEdgeList)
{
	mapLoc mapStart = ToMapLoc(start);
	mapLoc mapGoal = ToMapLoc(goal);

	if (IsSubgoal(mapStart))	// If the start location is already a subgoal, use it
		sgStart = cellInfo[mapStart];

	else	// Create a new subgoal for it, with id nSubgoals (at the end of all the actual subgoals)
	{
		sgStart = nSubgoals;
		location[sgStart] = start;
		GetSafeHReachableSubgoals(start, startSafeHReachableSubgoals);
		neighbors[sgStart] = startSafeHReachableSubgoals.data();
		nNeighbors[sgStart] = startSafeHReachableSubgoals.size();
	}

	if (IsSubgoal(mapGoal))	// If the goal location is already a subgoal, use it
	{
		sgGoal = cellInfo[mapGoal];
		if (IsPruned(sgGoal))	// If it is a local subgoal, we need to add global to local edges
		{
			for (unsigned int i = 0; i < nNeighbors[sgGoal]; i++)
			{
				subgoalId sg = neighbors[sgGoal][i];

				neighbors[sg][nNeighbors[sg]] = sgGoal;
				nNeighbors[sg]++;
				SetExtraEdgeFlag(sg);
				extraEdgeList.push_back(sg);
			}
		}
	}
	else	// Create a new subgoal for it, with id nSubgoals+1 (at the end of all the actual subgoals, after the start)
	{
		sgGoal = nSubgoals + 1;
		location[sgGoal] = goal;
		GetSafeHReachableSubgoals(goal, goalSafeHReachableSubgoals);

		// First, add incoming edges from all the safeHReachableSubgoals
		for (unsigned int i = 0; i < goalSafeHReachableSubgoals.size(); i++)
		{
			subgoalId sg = goalSafeHReachableSubgoals[i];

			neighbors[sg][nNeighbors[sg]] = sgGoal;
			nNeighbors[sg]++;
			SetExtraEdgeFlag(sg);
			extraEdgeList.push_back(sg);
			gCost[sg] = HCost(sg, sgGoal);	// Also use the g-value of that state to store its h-value (needed for the next step)
		}
		gCost[sgGoal] = 0;
		SetExtraEdgeFlag(sgGoal); // So that in the next step nobody tries to add new edges for sgGoal

		/* Next, make sure that all the pruned subgoals among the safeHReachableSubgoals have incoming edges.
		 * Note that, if we arbitrarily add edges, we may need to add more than one edge for a subgoal.
		 * Instead, we store only one edge, who can be followed to optimally reach the goal.
		 */
		for (unsigned int i = 0; i < goalSafeHReachableSubgoals.size(); i++)
		{
			subgoalId sg = goalSafeHReachableSubgoals[i];

			if (IsPruned(sg))	// Only do the additional step for pruned subgoals
			{
				for (unsigned int j = 0; j < nNeighbors[sg]; j++)
				{
					subgoalId sg2 = neighbors[sg][j];

					if(!HasExtraEdge(sg2))	// Has no other extra edge, just make sg2->sg the extra edge
					{
						neighbors[sg2][nNeighbors[sg2]] = sg;
						nNeighbors[sg2]++;
						SetExtraEdgeFlag(sg2);
						extraEdgeList.push_back(sg2);
						gCost[sg2] = HCost(sg, sg2) + gCost[sg];	// Store the cost of the best known path to the goal
					}

					else	// Already has an extra edge, make sg2->sg the extra edge only if it yields a shorter path
					{
						cost newCost = HCost(sg, sg2) + gCost[sg];
						if (newCost < gCost[sg2])
						{
							neighbors[sg2][nNeighbors[sg2]-1] = sg;	// We subtract 1 because nNeighbors is already updated
							gCost[sg2] = newCost;
						}
					}
				}
			}
		}
	}	// Initialized

}

cost SubgoalGraph::CheckCommonLocal(subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	static std::vector<subgoalId> locals;
	locals.clear();
	localPath.clear();
	cost minCost = INFINITE_COST;
	subgoalId connectingSg;
	
	
	for (int i = 0; i < goalSafeHReachableSubgoals.size(); i++)
		if (IsPruned(goalSafeHReachableSubgoals[i]))
			locals.push_back(goalSafeHReachableSubgoals[i]);
	
	for (int i = 0; i < startSafeHReachableSubgoals.size(); i++)
		if (IsPruned(startSafeHReachableSubgoals[i]))
		{
			for (int j = 0; j < goalSafeHReachableSubgoals.size(); j++)
			{
				if (goalSafeHReachableSubgoals[j] == startSafeHReachableSubgoals[i])
				{
					cost costThrough = HCost(sgStart, goalSafeHReachableSubgoals[j]) + HCost(goalSafeHReachableSubgoals[j], sgGoal);
					if (costThrough < minCost)
					{
						minCost = costThrough;
						connectingSg = goalSafeHReachableSubgoals[j];
					}
					break;
				}
			}
		}
	
	if (minCost != INFINITE_COST)
	{
		localPath.push_back(sgStart);
		localPath.push_back(connectingSg);
		localPath.push_back(sgGoal);
	}
	return minCost;
}

cost SubgoalGraph::CheckAllLocalPairs(subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	// check for any connecting pair (don't try to find the best one)
	//static std::vector<subgoalId> locals;
	//locals.clear();
	//localPath.clear();
	//cost minCost = INFINITE_COST;
	
	for (int i = 0; i < startSafeHReachableSubgoals.size(); i++)
		if (IsPruned(startSafeHReachableSubgoals[i]))
		{
			for (int j = 0; j < goalSafeHReachableSubgoals.size(); j++)
			{
				if (IsHReachable(location[goalSafeHReachableSubgoals[j]],location[startSafeHReachableSubgoals[i]]))
				{
					localPath.clear();
					localPath.push_back(sgStart);
					localPath.push_back(startSafeHReachableSubgoals[i]);
					localPath.push_back(goalSafeHReachableSubgoals[j]);
					localPath.push_back(sgGoal);
					
					return 	HCost(sgStart, startSafeHReachableSubgoals[i]) + 
							HCost(startSafeHReachableSubgoals[i], goalSafeHReachableSubgoals[j]) + 
							HCost(goalSafeHReachableSubgoals[j], sgGoal);
				}
			}
		}
		
	return INFINITE_COST;
}
cost SubgoalGraph::TryLocalPair(subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	localPath.clear();

	if ((sgStart == nSubgoals || IsPruned(sgStart)) && (sgGoal == nSubgoals + 1 || IsPruned(sgGoal)))
	{
		subgoalId startLocal;
		cost startCost;

		if (sgStart != nSubgoals)	// then start itself is the local subgoal we were looking for
		{
			startLocal = sgStart;
			startCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			startLocal = nSubgoals + 2; // uninitialized
			startCost = INFINITE_COST;

			for (unsigned int i = 0; i < startSafeHReachableSubgoals.size(); i++)
			{
				if (IsPruned(startSafeHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, startSafeHReachableSubgoals[i]) + HCost(startSafeHReachableSubgoals[i], sgGoal);
					if (costThrough < startCost)
					{
						startLocal = startSafeHReachableSubgoals[i];
						startCost = costThrough;
					}
				}
			}
		}
		if (startLocal == nSubgoals+2)
			return INFINITE_COST;

		subgoalId goalLocal;
		cost goalCost;

		if (sgGoal != nSubgoals + 1)	// then start itself is the local subgoal we were looking for
		{
			goalLocal = sgGoal;
			goalCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			goalLocal = nSubgoals + 2; // uninitialized
			goalCost = INFINITE_COST;

			for (unsigned int i = 0; i < goalSafeHReachableSubgoals.size(); i++)
			{
				if (IsPruned(goalSafeHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, goalSafeHReachableSubgoals[i]) + HCost(goalSafeHReachableSubgoals[i], sgGoal);
					if (costThrough < goalCost)
					{
						goalLocal = goalSafeHReachableSubgoals[i];
						goalCost = costThrough;
					}
				}
			}
		}

		if (goalLocal == nSubgoals+2)
			return INFINITE_COST;

		if (startLocal == goalLocal)	// then the same path will also be found by the actual subgoal graph search
			return INFINITE_COST;

		if (IsHReachable(location[startLocal], location[goalLocal]))
		//if (IsLookaheadHReachable(location[startLocal], location[goalLocal]))
		{
			cost localCost = HCost(startLocal, goalLocal);
			if (startLocal != sgStart)
			{
				localPath.push_back(sgStart);
				localCost += HCost(sgStart, startLocal);
			}

			localPath.push_back(startLocal);
			localPath.push_back(goalLocal);

			if (goalLocal != sgGoal)
			{
				localPath.push_back(sgGoal);
				localCost += HCost(goalLocal, sgGoal);
			}

			return localCost;
		}
	}

	return INFINITE_COST;
}
void SubgoalGraph::AppendOptimalPathRec(subgoalId sg1, subgoalId sg2, std::vector<subgoalId> & path)
{
	if (sg1 != sg2)
	{
		subgoalId mid = (sg1>sg2)?(next[sg1][sg2]):(next[sg2][sg1]);
		if (mid != sg2)
			AppendOptimalPathRec(sg1,mid,path);
		path.push_back(mid);
		if (mid != sg1)
			AppendOptimalPathRec(mid,sg2,path);
	}
}
void SubgoalGraph::AppendOptimalPath(subgoalId sg1, subgoalId sg2, std::vector<subgoalId> & path)
{
#ifdef PAIRWISE_MIDPOINTS
	path.push_back(sg1);
	AppendOptimalPathRec(sg1,sg2,path);
	path.push_back(sg2);
	return;
#endif
	while (sg1 != sg2)
	{
		path.push_back(sg1);
		cost c = (sg1>sg2)?dist[sg1][sg2]:dist[sg2][sg1];
		
		for (int i = 0; i < nNeighbors[sg1]; i++)
		{
			int sg = neighbors[sg1][i];
			cost c1 = (sg>sg1)?dist[sg][sg1]:dist[sg1][sg];
			cost c2 = (sg>sg2)?dist[sg][sg2]:dist[sg2][sg];
				
			if (c1 + c2 == c)
			{
				sg1 = sg;
				break;
			}
		}
	}
	path.push_back(sg1);
}
void SubgoalGraph::GetGlobalConnections(xyLoc & loc, subgoalId & locSg, std::vector<subgoalId> & safeConnections, std::vector<subgoalId> & globalSubgoals, std::vector<subgoalId> & linkToLocal, std::vector<cost> & distToOrigin)
{
	safeConnections.clear();
	globalSubgoals.clear();
	distToOrigin.clear();
	linkToLocal.clear();
	
	mapLoc mLoc = ToMapLoc(loc);

	if (IsSubgoal(mLoc))
	{
		locSg = cellInfo[mLoc];
		
		if (locSg < nGlobalSubgoals)	// It is a global subgoal
		{
			globalSubgoals.push_back(locSg);
			distToOrigin.push_back(0);	// has a trivial 0-cost path to itself
			linkToLocal.push_back(locSg);
		}
		else	// It is a local subgoal, push all the global connections
		{
			for (unsigned int i = 0; i < nNeighbors[locSg]; i++)	// Since we don't have local to local edges, all the edges are global connections
			{
				globalSubgoals.push_back(neighbors[locSg][i]);
				distToOrigin.push_back(HCost(locSg, neighbors[locSg][i]));
				linkToLocal.push_back(locSg);
			}
		}
	}
	else	// not a subgoal
	{
		// use locSg's default value for the subgoal position
		location[locSg] = loc;
		GetSafeHReachableSubgoals(loc, safeConnections);
		
		// First use the hasExtraEdge, gCost, and the extra edge slots to connect 'loc' to all the necessary global subgoals
		// We don't actually add or remove edges, we just use them for book keeping
		
		// Go over all the safe connections
		for (unsigned int i = 0; i < safeConnections.size(); i++)
		{
			subgoalId sg = safeConnections[i];
			if (!IsPruned(sg))	// If it is a global subgoal, say that its directly connected to the start
			{	
				neighbors[sg][nNeighbors[sg]] = locSg;	// meanint it is not a subgoal
				SetExtraEdgeFlag(sg);
				globalSubgoals.push_back(sg);
			}
			gCost[sg] = HCost(sg, locSg);	// Also use the g-value of that state to store its h-value (needed for the next step)
		}
		
		for (unsigned int i = 0; i < safeConnections.size(); i++)	// Go over all the safe connections again
		{
			subgoalId sg = safeConnections[i];

			if (IsPruned(sg))	// If it is a local subgoal, check its global connections
			{
				for (unsigned int j = 0; j < nNeighbors[sg]; j++)
				{
					subgoalId sg2 = neighbors[sg][j];

					if(!HasExtraEdge(sg2))	// If it is not yet connected to the 'loc' (either directly or through a local subgoal)
					{
						neighbors[sg2][nNeighbors[sg2]] = sg;	// Make sg the best known connection
						SetExtraEdgeFlag(sg2);	// Mark it as connected
						gCost[sg2] = HCost(sg, sg2) + gCost[sg];	// Store the cost of the best known path to the 'loc'
						globalSubgoals.push_back(sg2);
					}

					else	// Already has a connection, check if a connection through sg is better
					{
						cost newCost = HCost(sg, sg2) + gCost[sg];
						if (newCost < gCost[sg2])	// Update the connection if necessary
						{
							neighbors[sg2][nNeighbors[sg2]] = sg;
							gCost[sg2] = newCost;
						}
					}
				}
			}
		}
		
		// Now, go over all the found global connections and collect the information (and clear the extra edge flags)
		for (int i = 0; i < globalSubgoals.size(); i++)
		{
			subgoalId sg = globalSubgoals[i];
			RemoveExtraEdgeFlag(sg);
			distToOrigin.push_back(gCost[sg]);
			linkToLocal.push_back(neighbors[sg][nNeighbors[sg]]);
		}
	}
}
cost SubgoalGraph::LookupOptimalPath(xyLoc & startLoc, xyLoc & goalLoc, subgoalId & start, subgoalId & goal, std::vector<subgoalId> & startSafeConnections, std::vector<subgoalId> & goalSafeConnections, std::vector<subgoalId> & path)
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif

	path.clear();

	// First step: Find the global subgoals connected to the start and goal (and their connection costs) 
	static std::vector<subgoalId> startSubgoals;
	static std::vector<subgoalId> goalSubgoals;
	static std::vector<cost> startGlobalSubgoalCosts;
	static std::vector<cost> goalGlobalSubgoalCosts;
	static std::vector<subgoalId> startLinkToLocal;
	static std::vector<subgoalId> goalLinkToLocal;

	start = nSubgoals;	// First, assume these will be the subgoal locations
	goal = nSubgoals + 1;
	
	GetGlobalConnections(startLoc, start, startSafeConnections, startSubgoals, startLinkToLocal, startGlobalSubgoalCosts);
	GetGlobalConnections(goalLoc, goal, goalSafeConnections, goalSubgoals, goalLinkToLocal, goalGlobalSubgoalCosts);

#ifdef SG_STATISTICS	
	double elapsed = t.EndTimer();
	initializeTime += elapsed;
	searchTime -= elapsed;
#endif
	
	if (start < nGlobalSubgoals && goal < nGlobalSubgoals)	// If both start and goal are already global subgoals
	{
		AppendOptimalPath(start,goal,path);
		return (start > goal)?dist[start][goal]:dist[goal][start];
	}

	if (start < nGlobalSubgoals)	// If start is already a global subgoal but goal is not
	{
		cost minCost = INFINITE_COST;
		int connectingSg = 0;

		for (unsigned int i = 0; i < goalSubgoals.size(); i++)
		{
			subgoalId sg = goalSubgoals[i];
			cost currCost = ((start > sg)?dist[start][sg]:dist[sg][start]) + goalGlobalSubgoalCosts[i];
			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg = i;
			}
		}

		if (minCost != INFINITE_COST)
		{
			AppendOptimalPath(start,goalSubgoals[connectingSg],path);
			if (goalLinkToLocal[connectingSg] != goal)
				path.push_back(goalLinkToLocal[connectingSg]);
			path.push_back(goal);
		}

		return minCost;
	}

	if (goal < nGlobalSubgoals)	// If goal is already in the graph
	{
		cost minCost = INFINITE_COST;
		int connectingSg = 0;

		for (unsigned int i = 0; i < startSubgoals.size(); i++)
		{
			subgoalId sg = startSubgoals[i];
			cost currCost = ((goal > sg)?dist[goal][sg]:dist[sg][goal]) + startGlobalSubgoalCosts[i];
			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg = i;
			}
		}

		if (minCost != INFINITE_COST)
		{
			path.push_back(start);
			if (startLinkToLocal[connectingSg] != start)
				path.push_back(startLinkToLocal[connectingSg]);
			AppendOptimalPath(startSubgoals[connectingSg],goal,path);
		}

		return minCost;
	}

	// Both start and goal are not global subgoals
	cost minCost = INFINITE_COST;
	int connectingSg1 = 0;
	int connectingSg2 = 0;

	for (unsigned int i = 0; i < startSubgoals.size(); i++)
	{
		subgoalId sg1 = startSubgoals[i];
		cost hStart = startGlobalSubgoalCosts[i];

		for (unsigned int j = 0; j < goalSubgoals.size(); j++)
		{
			subgoalId sg2 = goalSubgoals[j];
			cost currCost = hStart + ((sg1 > sg2)?dist[sg1][sg2]:dist[sg2][sg1]) + goalGlobalSubgoalCosts[j];

			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg1 = i;
				connectingSg2 = j;
			}
		}
	}

	if (minCost != INFINITE_COST)
	{
		path.push_back(start);
		if (startLinkToLocal[connectingSg1] != start)	path.push_back(startLinkToLocal[connectingSg1]);
			
		AppendOptimalPath(startSubgoals[connectingSg1],goalSubgoals[connectingSg2],path);
		
		if (goalLinkToLocal[connectingSg2] != goal)		path.push_back(goalLinkToLocal[connectingSg2]);
			
		path.push_back(goal);
	}

	return minCost;
}
cost SubgoalGraph::SubgoalAStarSearch(xyLoc & start, xyLoc & goal, cost searchLimit, std::vector<xyLoc> & thePath)
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif

	static std::vector<subgoalId> abstractPath;
	static std::vector<subgoalId> localPath;
	static std::vector<mapLoc> mapLocPath;

	// First, check if there is a direct path between start and goal
	if (IsQuickHReachable(start, goal, mapLocPath))
	{
		ToXYLocPath(mapLocPath, thePath);
		#ifdef SG_STATISTICS
			initializeTime += t.EndTimer();	
			//std::cout<<"Quick path found with cost: "<<HCost(start,goal)/(double)CARD_COST<<std::endl;
		#endif
		return HCost(start,goal);
	}
	else
		mapLocPath.clear();
	
	// If not, do a search
	subgoalId sgStart, sgGoal;
	static std::vector<subgoalId> startSafeHReachableSubgoals;
	static std::vector<subgoalId> goalSafeHReachableSubgoals;
	static std::vector<subgoalId> extraEdgeList;
	cost abstractCost, localCost;
#ifdef PAIRWISE_DISTANCES
	// If we have the global pairwise distances, use it
	if (pairwiseUsage > 0)
		{
		abstractCost = LookupOptimalPath(start, goal, sgStart, sgGoal, startSafeHReachableSubgoals, goalSafeHReachableSubgoals, abstractPath);
		localCost = CheckCommonLocal(sgStart, sgGoal, startSafeHReachableSubgoals, goalSafeHReachableSubgoals, localPath);
		if (localCost < abstractCost)
		{
			#ifdef SG_STATISTICS
				std::cout<<"Shorter path found through common local: "<<localCost/(double)CARD_COST<<" < "<<abstractCost/(double)CARD_COST<<std::endl;
			#endif
			abstractCost = localCost;
			abstractPath = localPath;
		}
	}
	else
#endif
	{
		// Prepare for a search by adding edges
		ConnectStartAndGoalToGraph(start, goal, sgStart, sgGoal, startSafeHReachableSubgoals, goalSafeHReachableSubgoals, extraEdgeList);

		#ifdef SG_STATISTICS
			initializeTime += t.EndTimer();
			t.StartTimer();
		#endif
		
		// Do an A* search over the modified subgoal graph
		abstractCost = SubgoalAStarSearch(sgStart, sgGoal, INFINITE_COST, abstractPath);
	}

	// Now we should have a path, either from a direct lookup or an A* search
	
	// Check if there is a shorter path through two local subgoals (which might be missed on pruned graphs)
	localCost = TryLocalPair(sgStart, sgGoal, startSafeHReachableSubgoals, goalSafeHReachableSubgoals, localPath);
	if (localCost < abstractCost)
	{
		#ifdef SG_STATISTICS
			std::cout<<"Shorter local path found: "<<localCost/(double)CARD_COST<<" < "<<abstractCost/(double)CARD_COST<<std::endl;
		#endif
		abstractCost = localCost;
		abstractPath = localPath;
	}
	#ifdef SG_STATISTICS
		searchTime += t.EndTimer();
		t.StartTimer();
	#endif
		
	if(abstractCost == INFINITE_COST)	//Still haven't found a path
	{
		// Try to find any local-local connection
		CheckAllLocalPairs(sgStart, sgGoal, startSafeHReachableSubgoals, goalSafeHReachableSubgoals, abstractPath);
	}

	// Now we should definitely have an abstract path

	// Get the low level path (path of xyLoc's) from the abstract path (path of subgoalId's)
	if(abstractCost != INFINITE_COST)
	{
		xyLoc from, to;

		if (abstractPath.size() > 0)
			to = location[abstractPath[0]];

		mapLocPath.clear();
		mapLocPath.push_back(ToMapLoc(to));

		for (unsigned int i = 1; i < abstractPath.size(); i++)
		{
			from = to;
			to = location[abstractPath[i]];
			if(!IsHReachable(from, to, mapLocPath, true))	// Append
			{
				#ifdef SG_STATISTICS
					std::cout<<"Hmmm! ";
					std::cout<<"From: ("<<from.x<<" "<<from.y<<") ";
					std::cout<<"to: ("<<to.x<<" "<<to.y<<") "<<std::endl;
				#endif
			}
		}
		ToXYLocPath(mapLocPath, thePath);
	}

#ifdef PAIRWISE_DISTANCES
	if (pairwiseUsage == 0)
#endif
	{
		// CLEAN UP
		for (unsigned int i = 0; i < extraEdgeList.size(); i++)	// Remove all the extra edges
		{
			subgoalId sg = extraEdgeList[i];
			if (HasExtraEdge(sg))
			{
				RemoveExtraEdgeFlag(sg);
				nNeighbors[sg]--;	// Don't include the extra edge in the range
			}
		}
		startSafeHReachableSubgoals.clear();
		goalSafeHReachableSubgoals.clear();
		extraEdgeList.clear();
	}
	
	#ifdef SG_STATISTICS
	finalizeTime += t.EndTimer();

	#ifdef SG_STATISTICS_PER_SEARCH
		std::cout<<"OPTIMIZ Found a path of length "<<abstractCost/(double)CARD_COST<<" in "<<(initializeTime + searchTime + finalizeTime)*1000<<"ms."<<std::endl;
		std::cout<<"Time spent initializing:          "<<initializeTime*1000<<"ms."<<std::endl;
		std::cout<<"Time spent finding abstract path: "<<searchTime*1000<<"ms."<<std::endl;
		std::cout<<"Time spent finalizing:            "<<finalizeTime*1000<<"ms."<<std::endl;
		initializeTime = 0;		searchTime = 0;		finalizeTime = 0;	// reset for the next search. comment out for cumulative statistics
	#endif
	#endif
	return abstractCost;
}
cost SubgoalGraph::GetPath(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath)
{
	return SubgoalAStarSearch(start, goal, INFINITE_COST, thePath);
}

#ifdef SG_RUNNING_IN_HOG
void SubgoalGraph::OpenGLDraw(const MapEnvironment *env)
{
//*
	for (subgoalId s = 0; s < nSubgoals; s++)
	{
		if(IsPruned(s))
			env->SetColor(1,0,0);
		else
			env->SetColor(0,0,1);

		env->OpenGLDraw(location[s]);

		/*
		std::vector<subgoalId> neighbors = edgeVector[s];
		for (unsigned int i = 0; i < neighbors.size(); i++)
		{
			if (neighbors[i] > s)
			{
				env->SetColor(0,0,1);
				//env->GLDrawColoredLine(location[s], location[neighbors[i]]);
			}
		}
		*/
	}

	for (unsigned int i = 0; i < defaultXYPath.size(); i++)
	{
		env->SetColor(1,1,0);
		env->OpenGLDraw(defaultXYPath[i]);
		//env->GLDrawColoredLine(location[defaultAbstractPath[i]], location[defaultAbstractPath[i+1]]);
	}
/*/
	for (mapLoc loc = 0; loc < mapSize; loc++)
	{
		if (IsSubgoal(loc))
		{
			env->SetColor(0,0,1);
			env->OpenGLDraw(ToXYLoc(loc));
		}
	}
//*/
}
#endif
void SubgoalGraph::PrintGraphStatistics()
{
	std::cout<<"--------------------"<<std::endl;

	int nGlobalSubgoals = 0;
	int nLocalSubgoals = 0;

	int nTotalGlobalToGlobalEdge = 0;
	int nTotalLocalToGlobalEdge = 0;
	int nTotalGlobalToLocalEdge = 0;
	int nTotalLocalToLocalEdge = 0;

	int nLocalNotConnectedToGlobal = 0;

	double sumGlobalToGlobalEdgeCost = 0.0;
	double sumLocalToGlobalEdgeCost = 0.0;

	double maxGlobalToGlobalEdgeCost = 0.0;
	double maxLocalToGlobalEdgeCost = 0.0;

	for (subgoalId sg1 = 0; sg1 < nSubgoals; sg1++)
	{
		int nGlobalToGlobalEdge = 0;
		int nLocalToGlobalEdge = 0;
		int nGlobalToLocalEdge = 0;
		int nLocalToLocalEdge = 0;

		if (IsPruned(sg1))
			nLocalSubgoals++;

		else
			nGlobalSubgoals++;

		std::vector<subgoalId> neighbors = edgeVector[sg1];
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			subgoalId sg2 = neighbors[j];
			cost hCost = HCost(sg1,sg2);

			if (!IsPruned(sg1) && !IsPruned(sg2))
			{
				nGlobalToGlobalEdge++;
				sumGlobalToGlobalEdgeCost += hCost;
				if (maxGlobalToGlobalEdgeCost < hCost)
					maxGlobalToGlobalEdgeCost = hCost;
			}

			if (IsPruned(sg1) && !IsPruned(sg2))
			{
				nLocalToGlobalEdge++;
				sumLocalToGlobalEdgeCost += hCost;
				if (maxLocalToGlobalEdgeCost < hCost)
					maxLocalToGlobalEdgeCost = hCost;
			}

			if (!IsPruned(sg1) && IsPruned(sg2))
				nGlobalToLocalEdge++;

			if (IsPruned(sg1) && IsPruned(sg2))
				nLocalToLocalEdge++;
		}

		if (nLocalToGlobalEdge == 0 && IsPruned(sg1))
			nLocalNotConnectedToGlobal++;

		nTotalGlobalToGlobalEdge += nGlobalToGlobalEdge;
		nTotalLocalToGlobalEdge += nLocalToGlobalEdge;
		nTotalGlobalToLocalEdge += nGlobalToLocalEdge;
		nTotalLocalToLocalEdge += nLocalToLocalEdge;
	}
/*
	std::cout<<"Total number of subgoals: "<<nGlobalSubgoals+nLocalSubgoals<<std::endl;
	std::cout<<"Total number of global subgoals: "<<nGlobalSubgoals<<std::endl;
	std::cout<<"Total number of local subgoals: "<<nLocalSubgoals<<std::endl;
	std::cout<<std::endl;
	std::cout<<"Total number of edges: "<<nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge + nTotalGlobalToLocalEdge + nTotalLocalToLocalEdge<<std::endl;
	std::cout<<"Total number of global to global edges: "<<nTotalGlobalToGlobalEdge<<std::endl;
	std::cout<<"Total number of local to global edges: "<<nTotalLocalToGlobalEdge<<std::endl;
	std::cout<<"Total number of global to local edges: "<<nTotalGlobalToLocalEdge<<std::endl;
	std::cout<<"Total number of local to local edges: "<<nTotalLocalToLocalEdge<<std::endl;
	std::cout<<std::endl;
*/
	std::cout<<"Subgoals: "<<nGlobalSubgoals+nLocalSubgoals<<"\t("<<nGlobalSubgoals<<" G + "<<nLocalSubgoals<<" L)\n";
	//std::cout<<"Edges: "<<nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge + nTotalGlobalToLocalEdge + nTotalLocalToLocalEdge<<"\t(";
	std::cout<<"Edges: "<<nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge<<"\t(";
	std::cout<<nTotalGlobalToGlobalEdge<<" GG + ";
	std::cout<<nTotalLocalToGlobalEdge<<" LG + ";
//	std::cout<<nTotalGlobalToLocalEdge<<" GL + ";
	std::cout<<nTotalLocalToLocalEdge<<" LL";
	std::cout<<")\n";
	std::cout<<"Number of local subgoals not connected to a global subgoal: "<<nLocalNotConnectedToGlobal<<std::endl;
	std::cout<<"Branching factor of the global subgraph: "<<1.0*nTotalGlobalToGlobalEdge/nGlobalSubgoals<<std::endl;
	std::cout<<"Maximum edge cost: "<<maxGlobalToGlobalEdgeCost/(double)CARD_COST<<" GG, "<<maxLocalToGlobalEdgeCost/(double)CARD_COST<<" LG"<<std::endl;
	std::cout<<"Average edge cost: "<<(1.0*sumGlobalToGlobalEdgeCost/nTotalGlobalToGlobalEdge)/(double)CARD_COST<<" GG, "<<(1.0*sumLocalToGlobalEdgeCost/nTotalLocalToGlobalEdge)/(double)CARD_COST<<" LG"<<std::endl;
//	std::cout<<"Size: "<<sizeof graph<<std::endl;
//	std::cout<<"Size: "<<sizeof(std::vector<char>)<<std::endl;
	std::cout<<"--------------------"<<std::endl;


	return;

	unsigned int bitsPerMapCell = 8 + 8 + 32; //1 + 1 + 32;
	//traversable;
	//subgoal;
	//cellInfo;

	unsigned int bitsPerSubgoal = 32 + 32 + 16 + 32 + 32 + 32 + 16;
	// location + neighborPtr + nNeighbor + extraEdge + gCost + parent + search counter (+ flags)
			//32 + 1 + 32 + 16 + 1 + 32 + 16 + 32 + 32 + 1;
	//std::vector<xyLoc> location;
	//std::vector<bool> pruned;
	//std::vector<subgoalId*> neighbors;
	//std::vector<uint16_t> nNeighbors;
	//std::vector<bool> hasExtraEdge;
		// + 32 for the space allocated for extra edge
	//std::vector<uint16_t> generated;
	//std::vector<cost> gCost;
	//std::vector<subgoalId> parent;
	//std::vector<bool> open;

	unsigned int bitsPerSubgoalNoSearch = 32 + 32 + 16 + 32;
	//location + neighborPtr + nNeighbor + extraEdge (pruned + hasExtraEdge merged into nNeighbor)

	unsigned int bitsPerEdge = 32;
	double mapMemory = (bitsPerMapCell*height*width)/8192.0;
	double subgoalMemory = (bitsPerSubgoal*(nGlobalSubgoals+nLocalSubgoals))/8192.0;
	double edgeMemory = (bitsPerEdge*(nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge))/8192.0;
	double totalMemory = mapMemory + subgoalMemory + edgeMemory;

	double subgoalNoSearchMemory = (bitsPerSubgoalNoSearch*(nGlobalSubgoals+nLocalSubgoals))/8192.0;
	double totalNoSearchMemory = mapMemory + subgoalNoSearchMemory + edgeMemory;

	double inflation = (2048*2048)/(double(height - 1) * double(height - 1));

	std::cout<<"Estimated memory usage:\n";
	std::cout<<"Map info: "<<mapMemory<<"kb."<<std::endl;
	std::cout<<"Subgoals: "<<subgoalMemory<<"kb."<<std::endl;
	std::cout<<"Edges: "<<edgeMemory<<"kb."<<std::endl;
	std::cout<<"Total: "<<totalMemory<<"kb."<<std::endl;
	std::cout<<"Estimated memory for 2048x2048 map: "<<totalMemory*inflation/1024<<"mb."<<std::endl;

	std::cout<<std::endl;
	std::cout<<"Subgoals (no search): "<<subgoalNoSearchMemory<<"kb."<<std::endl;
	std::cout<<"Total (no search): "<<totalNoSearchMemory<<"kb."<<std::endl;
	std::cout<<"Estimated memory for 2048x2048 map (no search): "<<totalNoSearchMemory*inflation/1024<<"mb."<<std::endl;

	std::cout<<"--------------------"<<std::endl;

}
