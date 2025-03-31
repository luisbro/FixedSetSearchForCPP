#ifndef GREEDY_MOVING_H
#define GREEDY_MOVING_H

#include <list>
#include <vector>

class GreedyMoving {
   public:
    static std::vector<std::list<int>> run(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        std::vector<std::list<int>> partition);

    static void updateMovingTable(
        const std::vector<std::vector<int>>& weights,
        const std::vector<std::list<int>>& currentPartition,
        std::vector<std::vector<int>>& benefitOfMovingVertex,
        int vertexLastMoved,
        int oldCliqueIndex,
        int newCliqueIndex);

    static void moveVertexFromTo(
        std::vector<std::list<int>>& currentPartition,
        int vertex,
        int oldCliqueIndex,
        int newCliqueIndex);

    static std::vector<std::vector<int>> initializeMovingTable(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        const std::vector<std::list<int>>& currentPartition);

   private:
    static std::pair<size_t, size_t> getBestMove(
        const std::vector<std::vector<int>>& benefitOfMovingVertex);
};

#endif  // GREEDY_MOVING_H
