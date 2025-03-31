#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include <cstdint>
#include <list>
#include <vector>

class SimulatedAnnealing {
   public:
    enum MoveType {
        MOVING,
        EDGING,
        PUSHING
    };

    static bool allowSingletonMoves;

    static std::vector<std::list<int>> run(
        const std::vector<std::list<int>>& partition,
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        double initialTemperature,
        double batchSizeScaleFactor,
        double cooldownFactor,
        double minimalTransitionRatio);

    static std::tuple<bool, double, int> step(
        std::vector<int>& numberOfVerticesInCliques,
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        std::vector<std::vector<int>>& benefitOfMoving,
        std::vector<int>& cliqueIndexForVertex,
        int vertexMovePreviously,
        double currentTemperature);

    static void moveVertex(
        int vertexToMove,
        int cliqueToMoveFrom,
        int cliqueToMoveTo,
        std::vector<int>& numberOfVerticesInCliques,
        std::vector<int>& cliqueIndexForVertex,
        std::vector<std::vector<int>>& benefitOfMoving,
        const std::vector<std::vector<int>>& weights);

    static std::tuple<int, int> bestMoveForVertex(
        int vertexToMove,
        int cliqueToMoveFrom,
        const std::vector<int>& numberOfVerticesInCliques,
        const std::vector<std::vector<int>>& benefitOfMoving);

    static std::tuple<int, int, MoveType> bestTwoPartMoveForVertex(
        int vertexToMove,
        int vertexMovePreviously,
        int cliqueToMoveFrom,
        int cliqueToMovePreviousVertexFrom,
        const std::vector<int>& numberOfVerticesInCliques,
        const std::vector<std::vector<int>>& weights,
        const std::vector<std::vector<int>>& benefitOfMoving);

    static double rewardForMove(
        int vertexToMove,
        int bestCliqueToMoveTo,
        const std::vector<std::vector<int>>& benefitOfMoving);

    static bool makeWorseningMove(
        double rewardForBestMove,
        double currentTemperature);

    static std::vector<int> initializeNumberOfVerticesInCliques(
        const std::vector<std::list<int>>& currentPartition,
        int numberOfVertices);

    static std::vector<int> initializeCliqueIndexForVertexLookup(
        const std::vector<std::list<int>>& currentPartition,
        int numberOfVertices);

    static std::vector<std::vector<int>> initializeEdgeWeightSumsBetweenVertexAndClique(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        const std::vector<std::list<int>>& currentPartition,
        int numberOfNonEmptyCliques);

    static void updateEdgeWeightSumsBetweenVertexAndClique(
        const std::vector<std::vector<int>>& weights,
        std::vector<std::vector<int>>& benefitOfMovingVertex,
        int vertexLastMoved,
        int oldCliqueIndex,
        int newCliqueIndex);

    static double CalculateSimulatedAnnealingTemperature(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        double batchSizeScaleFactor);

   private:
    static std::tuple<int, int> bestClassicalMoveForVertex(
        int vertexToMove,
        int cliqueToMoveFrom,
        int numberOfCliques,
        const std::vector<int>& numberOfVerticesInCliques,
        const std::vector<std::vector<int>>& benefitOfMoving);

    static std::tuple<int, int> bestEdgingForVertex(
        int vertexToMove,
        int cliqueToMoveFrom,
        int vertexMovedPreviously,
        int cliqueToMovePreviousVertexFrom,
        int numberOfCliques,
        const std::vector<std::vector<int>>& benefitOfMoving,
        const std::vector<std::vector<int>>& weights);

    static std::tuple<int, int> bestPushingForVertex(
        int vertexToMove,
        int cliqueToMoveFrom,
        int vertexMovedPreviously,
        int cliqueToMovePreviousVertexFrom,
        int numberOfCliques,
        const std::vector<std::vector<int>>& benefitOfMoving,
        const std::vector<std::vector<int>>& weights);

    static std::tuple<int, int> valueForMovingToEmptyClique(
        int vertexToMove,
        int cliqueToMoveFrom,
        const std::vector<int>& numberOfVerticesInCliques,
        const std::vector<std::vector<int>>& benefitOfMoving);

    static std::vector<std::list<int>> cliqueIndexVectorToPartition(
        const std::vector<int>& cliqueIndexForVertex,
        int numberOfVertices);
};

#endif  // SIMULATED_ANNEALING_H
