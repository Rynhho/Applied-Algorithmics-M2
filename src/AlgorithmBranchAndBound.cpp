#include "AlgorithmBranchAndBound.hpp"

#include <chrono>
#include <cmath>

Solution AlgorithmBranchAndBound::solveMinCenters(const Graph& graph, int radius)
{
    Graph unweightedGraph = transformToUnitGraph(graph, radius);

    int nbCenters = 0;
    while (true)
    {
        std::vector<int> partialSolution;
        Solution solution = branchAndBound(unweightedGraph, partialSolution, nbCenters);

        if (solution.isValid)
        {
            return solution;
        }

        nbCenters += 1;
    }
}

Solution AlgorithmBranchAndBound::solveMinRadius(const Graph& graph, int nbCenters)
{
    Solution solution;

    // Todo

    return solution;
}

Solution AlgorithmBranchAndBound::branchAndBound(const Graph& graph, std::vector<int>& partialSolution, int nbCenters)
{
    if (nbCenters == 0)
    {
        Solution solution;

        std::vector<bool> markedVertices(graph.getNbVertices(), false);
        for (int vertex : partialSolution)
        {
            markedVertices[vertex] = true;

            for (int neighbor : graph.getNeighbors(vertex))
            {
                markedVertices[neighbor] = true;
            }
        }

        for (bool b : markedVertices)
        {
            if (b == false)
            {
                solution.isValid = false;
                return solution;
            }
        }

        solution.centers = partialSolution;
        solution.isValid = true;
        return solution;
    }
    else
    {
        std::vector<int> nonDominatedVertices;
        nonDominatedVertices.reserve(graph.getNbVertices() / 2);

        std::vector<bool> markedVertices(graph.getNbVertices(), false);
        for (int vertex : partialSolution)
        {
            markedVertices[vertex] = true;

            for (int neighbor : graph.getNeighbors(vertex))
            {
                markedVertices[neighbor] = true;
            }
        }

        for (int vertex = 0; vertex < markedVertices.size(); ++vertex)
        {
            if (markedVertices[vertex] == false)
            {
                nonDominatedVertices.push_back(vertex);
            }
        }

        int minDegVertex = nonDominatedVertices[0];
        int minDeg = graph.getNeighbors(minDegVertex).size();

        for (int vertex : nonDominatedVertices)
        {
            int vertexDeg = graph.getNeighbors(vertex).size();
            if (vertexDeg < minDeg)
            {
                minDegVertex = vertex;
                minDeg = vertexDeg;
            }
        }

        partialSolution.push_back(minDegVertex);
        Solution newSolution = branchAndBound(graph, partialSolution, nbCenters - 1);
        if (newSolution.isValid)
        {
            return newSolution;
        }
        else
        {
            partialSolution.pop_back();
        }

        for (int neighbor : graph.getNeighbors(minDegVertex))
        {
            partialSolution.push_back(neighbor);

            newSolution = branchAndBound(graph, partialSolution, nbCenters - 1);
            if (newSolution.isValid)
            {
                return newSolution;
            }
            else
            {
                partialSolution.pop_back();
            }
        }

        Solution solution;
        solution.isValid = false;
        return solution;
    }
}

Graph AlgorithmBranchAndBound::transformToUnitGraph(const Graph& graph, int radius)
{
    std::vector<std::vector<int>> adjacencyList(graph.getNbVertices(), std::vector<int>());

    for (int vertex1 = 0; vertex1 < graph.getNbVertices(); ++vertex1)
    {
        for (int vertex2 = 0; vertex2 < graph.getNbVertices(); ++vertex2)
        {
            if (graph.getDistance(vertex1, vertex2) <= radius)
            {
                adjacencyList[vertex1].push_back(vertex2);
            }
        }
    }

    return Graph(adjacencyList);
}
