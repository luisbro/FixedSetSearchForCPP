#include "SimulatedAnnealing.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <random>
#include <tuple>
#include <vector>

#include "GreedyAdding.h"
#include "RandomNumberGenerator.h"
#include "utils.h"

/**
 * In the C# implementation, it was allowed for vertices to move from a singleton partition
 * into a new, empty partition. This can be toggled here,
 * but it could still be the case that even setting this to false will not prohibit singleton moves,
 * as there might be some places that escaped me.
 */
bool SimulatedAnnealing::allowSingletonMoves = false;

std::vector<std::list<int>> SimulatedAnnealing::run(const std::vector<std::list<int>>& initialPartition,
                                                    const std::vector<int>& vertices,
                                                    const std::vector<std::vector<int>>& weights,
                                                    double initialTemperature,
                                                    double batchSizeScaleFactor,
                                                    double cooldownFactor,
                                                    double minimalTransitionRatio) {
    double currentTemperature = initialTemperature;

    // move all non-empty partitions to the front
    std::vector<std::list<int>> sortedPartition = initialPartition;
    auto indexOfFirstEmptyClique = std::partition(
        sortedPartition.begin(), sortedPartition.end(),
        [](const std::list<int>& clique) { return !clique.empty(); });

    int numberOfCliques = std::distance(sortedPartition.begin(), indexOfFirstEmptyClique);
    int numberOfVertices = vertices.size();
    int batchSize = static_cast<int>(std::round(batchSizeScaleFactor * numberOfCliques * numberOfVertices));

    std::vector<std::vector<int>> edgeWeightSumsBetweenVertexAndClique = initializeEdgeWeightSumsBetweenVertexAndClique(vertices, weights, sortedPartition, numberOfCliques);
    std::vector<int> numberOfVerticesInCliques = initializeNumberOfVerticesInCliques(sortedPartition, vertices.size());
    std::vector<int> cliqueIndexForVertex = initializeCliqueIndexForVertexLookup(sortedPartition, numberOfVertices);

    double currentPartitionValue = utils::valueForPartition(sortedPartition, weights);
    std::vector<int> bestPartitionAsCliqueIndexVector = cliqueIndexForVertex;
    double bestPartitionValue = currentPartitionValue;
    int stagnationCounter = 0;

    int vertexMovedPreviously = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    while (stagnationCounter < 5) {
        int numberOfTransitions = 0;

        for (int i = 0; i < batchSize; ++i) {
            bool performedTransition;
            double rewardForBestMove;
            std::tie(performedTransition, rewardForBestMove, vertexMovedPreviously) = step(numberOfVerticesInCliques,
                                                                                           vertices,
                                                                                           weights,
                                                                                           edgeWeightSumsBetweenVertexAndClique,
                                                                                           cliqueIndexForVertex,
                                                                                           vertexMovedPreviously,
                                                                                           currentTemperature);

            if (performedTransition) {
                currentPartitionValue += rewardForBestMove;
                numberOfTransitions++;
            }

            if (currentPartitionValue > bestPartitionValue) {
                bestPartitionAsCliqueIndexVector = cliqueIndexForVertex;
                bestPartitionValue = currentPartitionValue;
            }
        }

        currentTemperature *= cooldownFactor;
        double transitionRatio = static_cast<double>(numberOfTransitions) / batchSize;

        if (transitionRatio < minimalTransitionRatio) {
            stagnationCounter++;
        } else {
            stagnationCounter = 0;
        }

        if (currentTemperature < 0.0005) {
            // The algorith breaking due to the temperature being to low in an indication
            // that there are some vertices that are moved, even with extremely low probablility,
            // for example because they have valid moves with gain 0.
            // It can also be an indication for bugs, which is why this statement was introduced.
            std::cout << "Temperature too low" << std::endl;
            break;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    if (duration > std::chrono::seconds(10)) {
        std::cout << "Simulated Annealing took " << duration.count() << " seconds" << std::endl;
        std::cout << "Temperature: " << currentTemperature << std::endl;
    }

    std::vector<std::list<int>> bestPartition = cliqueIndexVectorToPartition(bestPartitionAsCliqueIndexVector, numberOfVertices);

    return bestPartition;
}

std::vector<std::list<int>> SimulatedAnnealing::cliqueIndexVectorToPartition(const std::vector<int>& cliqueIndexForVertex, int numberOfVertices) {
    std::vector<std::list<int>> partition(numberOfVertices);

    for (int vertexIndex = 0; vertexIndex < cliqueIndexForVertex.size(); ++vertexIndex) {
        partition[cliqueIndexForVertex[vertexIndex]].push_back(vertexIndex);
    }

    return partition;
}

std::tuple<bool, double, int> SimulatedAnnealing::step(std::vector<int>& numberOfVerticesInCliques,
                                                       const std::vector<int>& vertices,
                                                       const std::vector<std::vector<int>>& weights,
                                                       std::vector<std::vector<int>>& benefitOfMoving,
                                                       std::vector<int>& cliqueIndexForVertex,
                                                       int vertexMovedPreviously,
                                                       double currentTemperature) {
    int vertexToMove = RandomNumberGenerator::getRandomNumberBelow(vertices.size());
    int cliqueToMoveFrom = cliqueIndexForVertex[vertexToMove];
    int cliqueToMovePreviousVertexFrom = cliqueIndexForVertex[vertexMovedPreviously];

    // auto [bestCliqueToMoveTo, rewardForBestMove] = bestMoveForVertex(vertexToMove, cliqueToMoveFrom, numberOfVerticesInCliques, benefitOfMoving);
    auto [bestCliqueToMoveTo, rewardForBestMove, moveType] = bestTwoPartMoveForVertex(vertexToMove,
                                                                                      vertexMovedPreviously,
                                                                                      cliqueToMoveFrom,
                                                                                      cliqueToMovePreviousVertexFrom,
                                                                                      numberOfVerticesInCliques,
                                                                                      weights,
                                                                                      benefitOfMoving);

    bool performedTransition = false;

    if (rewardForBestMove >= 0 || makeWorseningMove(rewardForBestMove, currentTemperature)) {
        switch (moveType) {
            case MoveType::MOVING:
                moveVertex(vertexToMove, cliqueToMoveFrom, bestCliqueToMoveTo, numberOfVerticesInCliques, cliqueIndexForVertex, benefitOfMoving, weights);
                break;
            case MoveType::EDGING:
                moveVertex(vertexToMove, cliqueToMoveFrom, bestCliqueToMoveTo, numberOfVerticesInCliques, cliqueIndexForVertex, benefitOfMoving, weights);
                moveVertex(vertexMovedPreviously, cliqueToMovePreviousVertexFrom, bestCliqueToMoveTo, numberOfVerticesInCliques, cliqueIndexForVertex, benefitOfMoving, weights);
                break;
            case MoveType::PUSHING:
                moveVertex(vertexToMove, cliqueToMoveFrom, bestCliqueToMoveTo, numberOfVerticesInCliques, cliqueIndexForVertex, benefitOfMoving, weights);
                moveVertex(vertexMovedPreviously, cliqueToMovePreviousVertexFrom, cliqueToMoveFrom, numberOfVerticesInCliques, cliqueIndexForVertex, benefitOfMoving, weights);
                break;
        }

        performedTransition = true;
    }

    return {performedTransition, rewardForBestMove, vertexToMove};
}

void SimulatedAnnealing::moveVertex(int vertexToMove,
                                    int cliqueToMoveFrom,
                                    int cliqueToMoveTo,
                                    std::vector<int>& numberOfVerticesInCliques,
                                    std::vector<int>& cliqueIndexForVertex,
                                    std::vector<std::vector<int>>& benefitOfMoving,
                                    const std::vector<std::vector<int>>& weights) {
    numberOfVerticesInCliques[cliqueToMoveFrom] -= 1;
    numberOfVerticesInCliques[cliqueToMoveTo] += 1;

    cliqueIndexForVertex[vertexToMove] = cliqueToMoveTo;

    updateEdgeWeightSumsBetweenVertexAndClique(weights, benefitOfMoving, vertexToMove, cliqueToMoveFrom, cliqueToMoveTo);
}

std::tuple<int, int> SimulatedAnnealing::bestMoveForVertex(int vertexToMove,
                                                           int cliqueToMoveFrom,
                                                           const std::vector<int>& numberOfVerticesInCliques,
                                                           const std::vector<std::vector<int>>& benefitOfMoving) {
    size_t numberOfCliques = benefitOfMoving.size();

    auto [bestCliqueToMoveTo, bestValueChangeForMovingVertex] = bestClassicalMoveForVertex(vertexToMove,
                                                                                           cliqueToMoveFrom,
                                                                                           numberOfCliques,
                                                                                           numberOfVerticesInCliques,
                                                                                           benefitOfMoving);

    auto [emptyCliqueToMoveTo, changeForRemoval] = valueForMovingToEmptyClique(vertexToMove,
                                                                               cliqueToMoveFrom,
                                                                               numberOfVerticesInCliques,
                                                                               benefitOfMoving);

    if (bestValueChangeForMovingVertex > changeForRemoval) {
        return {bestCliqueToMoveTo, bestValueChangeForMovingVertex};
    } else {
        return {emptyCliqueToMoveTo, changeForRemoval};
    }
}

std::tuple<int, int, SimulatedAnnealing::MoveType> SimulatedAnnealing::bestTwoPartMoveForVertex(int vertexToMove,
                                                                                                int vertexMovedPreviously,
                                                                                                int cliqueToMoveFrom,
                                                                                                int cliqueToMovePreviousVertexFrom,
                                                                                                const std::vector<int>& numberOfVerticesInCliques,
                                                                                                const std::vector<std::vector<int>>& weights,
                                                                                                const std::vector<std::vector<int>>& benefitOfMoving) {
    // If the vertex was moved in the previous step, there is no way to make a move with of type PUSHING or EDGING
    if (vertexToMove == vertexMovedPreviously) {
        auto [bestCliqueToMoveTo, bestValueChangeForMovingVertex] = bestMoveForVertex(vertexToMove, cliqueToMoveFrom, numberOfVerticesInCliques, benefitOfMoving);
        return {bestCliqueToMoveTo, bestValueChangeForMovingVertex, MoveType::MOVING};
    }

    size_t numberOfCliques = benefitOfMoving.size();

    auto [bestCliqueToMoveTo, bestValueChangeForMovingVertex] = bestClassicalMoveForVertex(vertexToMove,
                                                                                           cliqueToMoveFrom,
                                                                                           numberOfCliques,
                                                                                           numberOfVerticesInCliques,
                                                                                           benefitOfMoving);

    auto [bestCliqueToEdgeTo, bestValueChangeForEdging] = bestEdgingForVertex(vertexToMove,
                                                                              cliqueToMoveFrom,
                                                                              vertexMovedPreviously,
                                                                              cliqueToMovePreviousVertexFrom,
                                                                              numberOfCliques,
                                                                              benefitOfMoving,
                                                                              weights);

    auto [bestCliqueToPushTo, bestValueChangeForPushing] = bestPushingForVertex(vertexToMove,
                                                                                cliqueToMoveFrom,
                                                                                vertexMovedPreviously,
                                                                                cliqueToMovePreviousVertexFrom,
                                                                                numberOfCliques,
                                                                                benefitOfMoving,
                                                                                weights);

    auto [emptyCliqueToMoveTo, changeForRemoval] = valueForMovingToEmptyClique(vertexToMove,
                                                                               cliqueToMoveFrom,
                                                                               numberOfVerticesInCliques,
                                                                               benefitOfMoving);

    int bestValueOfAnyMove = std::max({bestValueChangeForMovingVertex, bestValueChangeForEdging, bestValueChangeForPushing, changeForRemoval});

    // Choose the best move

    if (bestValueOfAnyMove == bestValueChangeForMovingVertex) {
        return {bestCliqueToMoveTo, bestValueOfAnyMove, MoveType::MOVING};
    }

    if (bestValueOfAnyMove == bestValueChangeForEdging) {
        return {bestCliqueToEdgeTo, bestValueOfAnyMove, MoveType::EDGING};
    }

    if (bestValueOfAnyMove == bestValueChangeForPushing) {
        return {bestCliqueToPushTo, bestValueOfAnyMove, MoveType::PUSHING};
    }

    if (bestValueOfAnyMove == changeForRemoval) {
        return {emptyCliqueToMoveTo, bestValueOfAnyMove, MoveType::MOVING};
    }

    // Unused default
    return {bestCliqueToMoveTo, bestValueChangeForMovingVertex, MoveType::MOVING};
}

std::tuple<int, int> SimulatedAnnealing::bestClassicalMoveForVertex(int vertexToMove,
                                                                    int cliqueToMoveFrom,
                                                                    int numberOfCliques,
                                                                    const std::vector<int>& numberOfVerticesInCliques,
                                                                    const std::vector<std::vector<int>>& benefitOfMoving) {
    int bestCliqueToMoveTo = -1;
    int highestChangeForAddingVertexToClique = std::numeric_limits<int>::min();

    int changeForRemoval = -benefitOfMoving[cliqueToMoveFrom][vertexToMove];

    for (int candidateCliqueIndex = 0; candidateCliqueIndex < numberOfCliques; ++candidateCliqueIndex) {
        if (not allowSingletonMoves && numberOfVerticesInCliques[candidateCliqueIndex] == 0) continue;  // Moving the vertex to an empty clique is not covered by this function
        if (candidateCliqueIndex == cliqueToMoveFrom) continue;                                         // Moving the vertex to the clique it is currently in doesn't make sense
        int changeForAddingVertexToCandidateClique = benefitOfMoving[candidateCliqueIndex][vertexToMove];

        if (changeForAddingVertexToCandidateClique > highestChangeForAddingVertexToClique) {
            highestChangeForAddingVertexToClique = changeForAddingVertexToCandidateClique;
            bestCliqueToMoveTo = candidateCliqueIndex;
        }
    }

    int bestValueChangeForMovingVertex = highestChangeForAddingVertexToClique + changeForRemoval;

    return {bestCliqueToMoveTo, bestValueChangeForMovingVertex};
}

std::tuple<int, int> SimulatedAnnealing::bestEdgingForVertex(int vertexToMove,
                                                             int cliqueToMoveFrom,
                                                             int vertexMovedPreviously,
                                                             int cliqueToMovePreviousVertexFrom,
                                                             int numberOfCliques,
                                                             const std::vector<std::vector<int>>& benefitOfMoving,
                                                             const std::vector<std::vector<int>>& weights) {
    int bestCliqueToEdgeTo = -1;
    int highestValueChangeForEdging = std::numeric_limits<int>::min();

    int changeForRemoval = -benefitOfMoving[cliqueToMoveFrom][vertexToMove];
    int changeForRemovalOfPreviouslyMovedVertex = -benefitOfMoving[cliqueToMovePreviousVertexFrom][vertexMovedPreviously];

    for (int candidateCliqueIndex = 0; candidateCliqueIndex < numberOfCliques; ++candidateCliqueIndex) {
        if (candidateCliqueIndex == cliqueToMoveFrom) continue;                // Moving the vertex to the clique it is currently in doesn't make sense
        if (candidateCliqueIndex == cliqueToMovePreviousVertexFrom) continue;  // Moving the vertex that was previously moved to the clique it is currently in doesn't make sense
        int changeForEdging = benefitOfMoving[candidateCliqueIndex][vertexToMove] + benefitOfMoving[candidateCliqueIndex][vertexMovedPreviously];

        if (changeForEdging > highestValueChangeForEdging) {
            highestValueChangeForEdging = changeForEdging;
            bestCliqueToEdgeTo = candidateCliqueIndex;
        }
    }

    // Adjustment according to explanation in the paper
    int adjustmentForEdging = cliqueToMoveFrom != cliqueToMovePreviousVertexFrom
                                  ? weights[vertexToMove][vertexMovedPreviously]       // Both vertices are moved from different cliques to the same, new clique
                                  : 2 * weights[vertexToMove][vertexMovedPreviously];  // Both vertices are moved from the same clique to the same, new clique

    highestValueChangeForEdging += adjustmentForEdging;

    int bestValueChangeForEdging = highestValueChangeForEdging + changeForRemoval + changeForRemovalOfPreviouslyMovedVertex;

    return {bestCliqueToEdgeTo, bestValueChangeForEdging};
}

std::tuple<int, int> SimulatedAnnealing::bestPushingForVertex(int vertexToMove,
                                                              int cliqueToMoveFrom,
                                                              int vertexMovedPreviously,
                                                              int cliqueToMovePreviousVertexFrom,
                                                              int numberOfCliques,
                                                              const std::vector<std::vector<int>>& benefitOfMoving,
                                                              const std::vector<std::vector<int>>& weights) {
    int bestCliqueToPushTo = -1;
    int highestSumForPushing = std::numeric_limits<int>::min();

    int changeForRemoval = -benefitOfMoving[cliqueToMoveFrom][vertexToMove];
    int changeForRemovalOfPreviouslyMovedVertex = -benefitOfMoving[cliqueToMovePreviousVertexFrom][vertexMovedPreviously];

    // A push can not happen if the vertices are in the same clique
    if (cliqueToMoveFrom == cliqueToMovePreviousVertexFrom) {
        return {bestCliqueToPushTo, highestSumForPushing};
    }

    for (int candidateCliqueIndex = 0; candidateCliqueIndex < numberOfCliques; ++candidateCliqueIndex) {
        if (candidateCliqueIndex == cliqueToMoveFrom) continue;  // Moving the vertex to the clique it is currently in doesn't make sense
        int changeForPushing = benefitOfMoving[candidateCliqueIndex][vertexToMove] + benefitOfMoving[cliqueToMoveFrom][vertexMovedPreviously];

        // Adjustment according to explanation in the paper
        int adjustmentForPushing = candidateCliqueIndex != cliqueToMovePreviousVertexFrom
                                       ? -weights[vertexToMove][vertexMovedPreviously]       // The previously moved vertex pushes the current vertex to a different clique
                                       : -2 * weights[vertexToMove][vertexMovedPreviously];  // The previously moved vertex and the current swap places

        changeForPushing += adjustmentForPushing;

        if (changeForPushing > highestSumForPushing) {
            highestSumForPushing = changeForPushing;
            bestCliqueToPushTo = candidateCliqueIndex;
        }
    }

    int bestValueChangeForPushing = highestSumForPushing + changeForRemoval + changeForRemovalOfPreviouslyMovedVertex;

    return {bestCliqueToPushTo, bestValueChangeForPushing};
}

std::tuple<int, int> SimulatedAnnealing::valueForMovingToEmptyClique(int vertexToMove,
                                                                     int cliqueToMoveFrom,
                                                                     const std::vector<int>& numberOfVerticesInCliques,
                                                                     const std::vector<std::vector<int>>& benefitOfMoving) {
    int changeForRemoval = -benefitOfMoving[cliqueToMoveFrom][vertexToMove];

    int emptyCliqueToMoveTo = -1;

    if (numberOfVerticesInCliques[cliqueToMoveFrom] == 1) {
        // If the clique is already a singleton, don't move the vertex
        emptyCliqueToMoveTo = cliqueToMoveFrom;
        if (not allowSingletonMoves) {
            changeForRemoval = std::numeric_limits<int>::min();
        }
    } else {
        // If the clique is not a singleton, move the vertex to the first empty clique
        auto firstEmptyCliqueIndex = std::find(numberOfVerticesInCliques.begin(), numberOfVerticesInCliques.end(), 0) - numberOfVerticesInCliques.begin();
        emptyCliqueToMoveTo = firstEmptyCliqueIndex;
    }

    return {emptyCliqueToMoveTo, changeForRemoval};
}

double SimulatedAnnealing::rewardForMove(int vertexToMove,
                                         int bestCliqueToMoveTo,
                                         const std::vector<std::vector<int>>& benefitOfMoving) {
    return benefitOfMoving[vertexToMove][bestCliqueToMoveTo];
}

bool SimulatedAnnealing::makeWorseningMove(double rewardForBestMove,
                                           double currentTemperature) {
    double probabilityOfMakingWorseningMove = std::exp(rewardForBestMove / currentTemperature);
    return RandomNumberGenerator::getRandomFloatBetweenZeroAndOne() < probabilityOfMakingWorseningMove;
}

std::vector<int> SimulatedAnnealing::initializeNumberOfVerticesInCliques(const std::vector<std::list<int>>& currentPartition,
                                                                         int numberOfVertices) {
    std::vector<int> numberOfVerticesInCliques(numberOfVertices, 0);
    for (int i = 0; i < currentPartition.size(); ++i) {
        numberOfVerticesInCliques[i] = currentPartition[i].size();
    }
    return numberOfVerticesInCliques;
}

std::vector<int> SimulatedAnnealing::initializeCliqueIndexForVertexLookup(const std::vector<std::list<int>>& currentPartition,
                                                                          int numberOfVertices) {
    std::vector<int> cliqueIndexForVertex(numberOfVertices, -1);
    for (size_t i = 0; i < currentPartition.size(); ++i) {
        for (int vertex : currentPartition[i]) {
            cliqueIndexForVertex[vertex] = i;
        }
    }
    return cliqueIndexForVertex;
}

std::vector<std::vector<int>> SimulatedAnnealing::initializeEdgeWeightSumsBetweenVertexAndClique(const std::vector<int>& vertices,
                                                                                                 const std::vector<std::vector<int>>& weights,
                                                                                                 const std::vector<std::list<int>>& currentPartition,
                                                                                                 int numberOfNonEmptyCliques) {
    size_t numVertices = vertices.size();

    std::vector<std::vector<int>> edgeWeightSumsBetweenVertexAndClique(numberOfNonEmptyCliques, std::vector<int>(numVertices, 0));

    for (size_t cliqueIndex = 0; cliqueIndex < numberOfNonEmptyCliques; ++cliqueIndex) {
        for (size_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex) {
            int vertex = vertices[vertexIndex];
            int connections = 0;

            for (int otherVertex : currentPartition[cliqueIndex]) {
                connections += weights[vertex][otherVertex];
            }

            edgeWeightSumsBetweenVertexAndClique[cliqueIndex][vertexIndex] = connections;
        }
    }

    return edgeWeightSumsBetweenVertexAndClique;
}

void SimulatedAnnealing::updateEdgeWeightSumsBetweenVertexAndClique(const std::vector<std::vector<int>>& weights,
                                                                    std::vector<std::vector<int>>& benefitOfMovingVertex,
                                                                    int vertexLastMoved,
                                                                    int oldCliqueIndex,
                                                                    int newCliqueIndex) {
    if (oldCliqueIndex == newCliqueIndex) return;

    if (benefitOfMovingVertex.size() < newCliqueIndex + 1) {
        benefitOfMovingVertex.resize(newCliqueIndex + 1, std::vector<int>(weights.size(), 0));
    }

    const std::vector<int>& vertexWeights = weights[vertexLastMoved];

    std::transform(benefitOfMovingVertex[newCliqueIndex].begin(),
                   benefitOfMovingVertex[newCliqueIndex].end(),
                   vertexWeights.begin(),
                   benefitOfMovingVertex[newCliqueIndex].begin(),
                   std::plus<int>());

    std::transform(benefitOfMovingVertex[oldCliqueIndex].begin(),
                   benefitOfMovingVertex[oldCliqueIndex].end(),
                   vertexWeights.begin(),
                   benefitOfMovingVertex[oldCliqueIndex].begin(),
                   std::minus<int>());
}

double SimulatedAnnealing::CalculateSimulatedAnnealingTemperature(const std::vector<int>& vertices,
                                                                  const std::vector<std::vector<int>>& weights,
                                                                  double batchSizeScaleFactor) {
    double calibrationTemperature = 1000;
    double lowerTemperature = 1;
    double upperTemperature = 2000;
    double tolerance = 0.05;
    double desiredTransitionRatio = 0.5;

    while (true) {
        int numberOfTransitions = 0;
        std::vector<std::list<int>> partition = GreedyAdding::runForEmptyPartition(vertices, weights, 2);

        // move all non-empty partitions to the front
        auto indexOfFirstEmptyClique = std::partition(
            partition.begin(), partition.end(),
            [](const std::list<int>& clique) { return !clique.empty(); });

        int numberOfCliques = std::distance(partition.begin(), indexOfFirstEmptyClique);
        int numberOfVertices = vertices.size();
        int batchSize = static_cast<int>(std::round(batchSizeScaleFactor * numberOfCliques * numberOfVertices));

        std::vector<int> numberOfVerticesInCliques = initializeNumberOfVerticesInCliques(partition, numberOfVertices);
        std::vector<std::vector<int>> edgeWeightSumsBetweenVertexAndClique = initializeEdgeWeightSumsBetweenVertexAndClique(vertices, weights, partition, numberOfCliques);
        std::vector<int> cliqueIndexForVertex = initializeCliqueIndexForVertexLookup(partition, numberOfVertices);

        int vertexMovedPreviously = 0;

        for (int i = 0; i < batchSize; ++i) {
            bool performedTransition;
            double rewardForBestMove;
            std::tie(performedTransition, rewardForBestMove, vertexMovedPreviously) = step(numberOfVerticesInCliques,
                                                                                           vertices,
                                                                                           weights,
                                                                                           edgeWeightSumsBetweenVertexAndClique,
                                                                                           cliqueIndexForVertex,
                                                                                           vertexMovedPreviously,
                                                                                           calibrationTemperature);

            if (performedTransition) {
                numberOfTransitions++;
            }
        }

        double transitionRatio = static_cast<double>(numberOfTransitions) / batchSize;

        if (transitionRatio > desiredTransitionRatio + tolerance) {
            upperTemperature = calibrationTemperature;
            calibrationTemperature = (calibrationTemperature + lowerTemperature) / 2;
        } else if (transitionRatio < desiredTransitionRatio - tolerance) {
            lowerTemperature = calibrationTemperature;
            calibrationTemperature = (calibrationTemperature + upperTemperature) / 2;
        } else {
            break;
        }
    }

    return calibrationTemperature;
}