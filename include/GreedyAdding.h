#ifndef GREEDY_ADDING_H
#define GREEDY_ADDING_H

#include <list>
#include <vector>

class GreedyAdding {
   public:
    static std::vector<std::list<int>> runForEmptyPartition(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        int lengthOfRandomCandidateList);

    static std::vector<std::list<int>> run(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        std::vector<std::list<int>>& initialPartition,
        int lengthOfRandomCandidateList);

   private:
    static void addingVertexToPartitionStep(
        std::vector<int>& candidateVertices,
        std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
        std::vector<std::list<int>>& currentPartition,
        const std::vector<std::vector<int>>& weights,
        int lengthOfRandomCandidateList);

    static std::pair<int, int> getIndicesOfRandomAddingMoveFromBestOptions(
        const std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
        int lengthOfRandomCandidateList,
        const std::vector<int>& candidateVertices,
        int firstEmptyCliqueIndex);

    static std::vector<std::vector<int>> initializeBenefitOfAddingVertexToClique(
        const std::vector<int>& candidateVertices,
        const std::vector<std::list<int>>& currentPartition,
        const std::vector<std::vector<int>>& weights);

    static void updateBenefitOfAddingVertexToClique(
        std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
        const std::vector<int>& candidateVertices,
        const std::vector<std::list<int>>& currentPartition,
        const std::vector<std::vector<int>>& weights,
        int vertexIndex,
        int cliqueIndex);
};

#endif  // GREEDY_ADDING_H
