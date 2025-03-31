#include "GreedyMoving.h"

#include <algorithm>
#include <limits>
#include <list>
#include <vector>

#include "utils.h"

// Function to initialize the moving table
std::vector<std::vector<int>> GreedyMoving::initializeMovingTable(const std::vector<int>& vertices,
                                                                  const std::vector<std::vector<int>>& weights,
                                                                  const std::vector<std::list<int>>& currentPartition) {
    size_t numVertices = vertices.size();

    int lastNonEmptyClique = std::distance(std::find_if(currentPartition.rbegin(),
                                                        currentPartition.rend(),
                                                        [](const std::list<int>& clique) { return not clique.empty(); }),
                                           currentPartition.rend());

    std::vector<std::vector<int>> benefitOfMovingVertex(numVertices, std::vector<int>(lastNonEmptyClique + 2, 0));

    for (size_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex) {
        int vertex = vertices[vertexIndex];
        int originalCliqueIndex = utils::cliqueIndexForVertex(vertex, currentPartition);

        int decreaseInScore = 0;
        for (int otherVertex : currentPartition[originalCliqueIndex]) {
            if (vertex == otherVertex)
                continue;
            decreaseInScore += weights[vertex][otherVertex];
        }

        for (size_t cliqueIndexToMoveTo = 0; cliqueIndexToMoveTo < lastNonEmptyClique + 2; ++cliqueIndexToMoveTo) {
            if (cliqueIndexToMoveTo == originalCliqueIndex)
                continue;

            for (int otherVertex : currentPartition[cliqueIndexToMoveTo]) {
                benefitOfMovingVertex[vertexIndex][cliqueIndexToMoveTo] += weights[vertex][otherVertex];
            }

            benefitOfMovingVertex[vertexIndex][cliqueIndexToMoveTo] -= decreaseInScore;
        }
    }

    return benefitOfMovingVertex;
}

// Function to get the best move (vertex and clique index)
std::pair<size_t, size_t> GreedyMoving::getBestMove(const std::vector<std::vector<int>>& benefitOfMovingVertex) {
    size_t bestRow = 0, bestCol = 0;
    int maxBenefit = std::numeric_limits<int>::min();

    int numberOfVertices = benefitOfMovingVertex.size();
    int maxLength = benefitOfMovingVertex[0].size();

    for (size_t i = 0; i < numberOfVertices; ++i) {
        for (size_t j = 0; j < maxLength; ++j) {
            if (benefitOfMovingVertex[i][j] > maxBenefit) {
                maxBenefit = benefitOfMovingVertex[i][j];
                bestRow = i;
                bestCol = j;
            }
        }
    }

    return {bestRow, bestCol};
}

// Function to move a vertex from one clique to another
void GreedyMoving::moveVertexFromTo(std::vector<std::list<int>>& currentPartition,
                                    int vertex,
                                    int oldCliqueIndex,
                                    int newCliqueIndex) {
    currentPartition[oldCliqueIndex].remove(vertex);
    currentPartition[newCliqueIndex].push_back(vertex);
}

// Function to update the moving table after moving a vertex
void GreedyMoving::updateMovingTable(const std::vector<std::vector<int>>& weights,
                                     const std::vector<std::list<int>>& currentPartition,
                                     std::vector<std::vector<int>>& benefitOfMovingVertex,
                                     int vertexLastMoved,
                                     int oldCliqueIndex,
                                     int newCliqueIndex) {
    int numberOfCliques = benefitOfMovingVertex[0].size();
    int numberOfVertices = benefitOfMovingVertex.size();

    // Add a new column if the vertex was moved to a new clique
    if (newCliqueIndex == numberOfCliques - 1) {
        for (int i = 0; i < numberOfVertices; ++i) {
            benefitOfMovingVertex[i].push_back(benefitOfMovingVertex[i].back());
        }
    }

    for (int vertexPreviouslyConnected : currentPartition[oldCliqueIndex]) {
        for (auto& benefit : benefitOfMovingVertex[vertexPreviouslyConnected]) {
            benefit += weights[vertexPreviouslyConnected][vertexLastMoved];
        }
    }

    for (int vertexNewlyConnected : currentPartition[newCliqueIndex]) {
        for (auto& benefit : benefitOfMovingVertex[vertexNewlyConnected]) {
            benefit -= weights[vertexNewlyConnected][vertexLastMoved];
        }
    }

    // Update column for old and new cliques
    for (size_t i = 0; i < benefitOfMovingVertex.size(); ++i) {
        benefitOfMovingVertex[i][oldCliqueIndex] -= weights[i][vertexLastMoved];
        benefitOfMovingVertex[i][newCliqueIndex] += weights[i][vertexLastMoved];
    }

    // Update row for the last moved vertex
    int benefitOfLastMove = benefitOfMovingVertex[vertexLastMoved][newCliqueIndex];
    for (size_t j = 0; j < benefitOfMovingVertex[vertexLastMoved].size(); ++j) {
        benefitOfMovingVertex[vertexLastMoved][j] -= benefitOfLastMove;
    }
}

// Function to find the optimal partition
std::vector<std::list<int>> GreedyMoving::run(const std::vector<int>& vertices,
                                              const std::vector<std::vector<int>>& weights,
                                              std::vector<std::list<int>> initialPartition) {
    auto currentPartition = initialPartition;
    auto benefitOfMovingVertex = initializeMovingTable(vertices, weights, currentPartition);

    // Continue until no beneficial moves exist
    while (std::any_of(benefitOfMovingVertex.begin(), benefitOfMovingVertex.end(),
                       [](const std::vector<int>& row) { return *std::max_element(row.begin(), row.end()) > 0; })) {
        auto [vertexIndex, newCliqueIndex] = getBestMove(benefitOfMovingVertex);
        int vertex = vertices[vertexIndex];
        int oldCliqueIndex = utils::cliqueIndexForVertex(vertex, currentPartition);

        // Move the vertex and update the partition
        moveVertexFromTo(currentPartition, vertex, oldCliqueIndex, newCliqueIndex);

        // Update the benefit of moving table
        updateMovingTable(weights, currentPartition, benefitOfMovingVertex, vertexIndex, oldCliqueIndex, newCliqueIndex);
    }

    return currentPartition;
}
