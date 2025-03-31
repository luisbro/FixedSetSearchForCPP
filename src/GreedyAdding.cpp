#include "GreedyAdding.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <random>
#include <tuple>
#include <vector>

#include "RandomNumberGenerator.h"

// Static function to find the solution for an empty partition
std::vector<std::list<int>> GreedyAdding::runForEmptyPartition(const std::vector<int>& vertices,
                                                               const std::vector<std::vector<int>>& weights,
                                                               int lengthOfRandomCandidateList = 2) {
    std::vector<std::list<int>> initialPartition(vertices.size());
    return run(vertices, weights, initialPartition, lengthOfRandomCandidateList);
}

// Static function to find the solution
std::vector<std::list<int>> GreedyAdding::run(const std::vector<int>& vertices,
                                              const std::vector<std::vector<int>>& weights,
                                              std::vector<std::list<int>>& initialPartition,
                                              int lengthOfRandomCandidateList = 2) {
    std::vector<int> verticesInInitialPartition;
    for (const auto& clique : initialPartition) {
        for (int vertex : clique) {
            verticesInInitialPartition.push_back(vertex);
        }
    }

    std::vector<int> candidateVertices;
    for (int v : vertices) {
        if (std::find(verticesInInitialPartition.begin(), verticesInInitialPartition.end(), v) == verticesInInitialPartition.end()) {
            candidateVertices.push_back(v);
        }
    }

    std::vector<std::list<int>> currentPartition = initialPartition;
    std::vector<std::vector<int>> benefitOfAddingVertexToClique = initializeBenefitOfAddingVertexToClique(candidateVertices, currentPartition, weights);

    size_t required_iterations = candidateVertices.size();
    for (size_t i = 0; i < required_iterations; ++i) {
        addingVertexToPartitionStep(candidateVertices, benefitOfAddingVertexToClique, currentPartition, weights, lengthOfRandomCandidateList);
    }

    currentPartition.resize(vertices.size());

    return currentPartition;
}

// Static function to handle adding vertex to partition
void GreedyAdding::addingVertexToPartitionStep(std::vector<int>& candidateVertices,
                                               std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
                                               std::vector<std::list<int>>& currentPartition,
                                               const std::vector<std::vector<int>>& weights,
                                               int lengthOfRandomCandidateList) {
    int vertexIndex, cliqueIndex;

    int firstEmptyCliqueIndex = std::find_if(currentPartition.begin(),
                                             currentPartition.end(),
                                             [](const std::list<int>& clique) { return clique.empty(); }) -
                                currentPartition.begin();
    std::tie(vertexIndex, cliqueIndex) = getIndicesOfRandomAddingMoveFromBestOptions(benefitOfAddingVertexToClique, lengthOfRandomCandidateList, candidateVertices, firstEmptyCliqueIndex);
    currentPartition[cliqueIndex].push_back(candidateVertices[vertexIndex]);
    updateBenefitOfAddingVertexToClique(benefitOfAddingVertexToClique, candidateVertices, currentPartition, weights, vertexIndex, cliqueIndex);
    candidateVertices.erase(candidateVertices.begin() + vertexIndex);
}

void GreedyAdding::updateBenefitOfAddingVertexToClique(std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
                                                       const std::vector<int>& candidateVertices,
                                                       const std::vector<std::list<int>>& currentPartition,
                                                       const std::vector<std::vector<int>>& weights,
                                                       int vertexIndex,
                                                       int cliqueIndex) {
    int movedVertex = candidateVertices[vertexIndex];

    for (size_t i = 0; i < candidateVertices.size(); ++i) {
        benefitOfAddingVertexToClique[i][cliqueIndex] += weights[candidateVertices[i]][movedVertex];
    }

    benefitOfAddingVertexToClique.erase(benefitOfAddingVertexToClique.begin() + vertexIndex);
}

// Static function to initialize benefit of adding vertex to clique
std::vector<std::vector<int>> GreedyAdding::initializeBenefitOfAddingVertexToClique(const std::vector<int>& candidateVertices,
                                                                                    const std::vector<std::list<int>>& currentPartition,
                                                                                    const std::vector<std::vector<int>>& weights) {
    std::vector<std::vector<int>> benefit(candidateVertices.size(), std::vector<int>(currentPartition.size(), 0));
    // Initialization logic for benefit can be added here based on weights

    for (size_t i = 0; i < candidateVertices.size(); ++i) {
        for (size_t j = 0; j < currentPartition.size(); ++j) {
            for (int vertex : currentPartition[j]) {
                benefit[i][j] += weights[candidateVertices[i]][vertex];
            }
        }
    }

    return benefit;
}

std::pair<int, int> GreedyAdding::getIndicesOfRandomAddingMoveFromBestOptions(const std::vector<std::vector<int>>& benefitOfAddingVertexToClique,
                                                                              int lengthOfRandomCandidateList,
                                                                              const std::vector<int>& candidateVertices,
                                                                              int firstEmptyCliqueIndex) {
    // Tuple for
    // 1. vertexIndex
    // 2. cliqueIndex
    // 3. benefit
    std::vector<std::tuple<int, int, int>> randomCandidateList;

    // Fill the Random Candidate List with the first i additions of the first vertex
    // Assuming that the random candiate list is shorter than the number of vertices.
    int firstVertex = 0;
    for (int i = 0; i < lengthOfRandomCandidateList; i++) {
        randomCandidateList.push_back({firstVertex, i, benefitOfAddingVertexToClique[firstVertex][i]});
        continue;
    }

    int randomCandidateListMinumum = std::get<2>(*std::min_element(randomCandidateList.begin(), randomCandidateList.end(),
                                                                   [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
                                                                       return std::get<2>(a) < std::get<2>(b);
                                                                   }));

    for (const auto& row : benefitOfAddingVertexToClique) {
        // the first empty clique should mark the end of all existing cliques
        // and we don't need to look further, as only other empty cliques should follow the first empty one
        // TODO: This could be handled by looking at the first non-empty clique from the back, to be more stable
        for (int i = 0; i <= firstEmptyCliqueIndex; ++i) {
            int current_benefit = row[i];

            // add to randomCandidateList if the current_benefit is better than the miniumum of the randomCandidateList
            if (current_benefit > randomCandidateListMinumum) {
                auto vertexIndex = &row - &benefitOfAddingVertexToClique[0];
                randomCandidateList.push_back({vertexIndex, i, current_benefit});

                // remove the minimum element from the list if it is too long
                auto minElement = std::min_element(randomCandidateList.begin(), randomCandidateList.end(),
                                                   [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
                                                       return std::get<2>(a) < std::get<2>(b);
                                                   });
                randomCandidateList.erase(minElement);

                // update the minimum of the randomCandidateList
                randomCandidateListMinumum = std::get<2>(*std::min_element(randomCandidateList.begin(), randomCandidateList.end(),
                                                                           [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
                                                                               return std::get<2>(a) < std::get<2>(b);
                                                                           }));
            }
        }
    }

    auto max_element_iter = std::max_element(randomCandidateList.begin(),
                                             randomCandidateList.end(),
                                             [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
                                                 return std::get<2>(a) < std::get<2>(b);
                                             });
    int max_benefit = std::get<2>(*max_element_iter);

    if (max_benefit == 0) {
        int randomVertexIndex = RandomNumberGenerator::getRandomNumberBelow(candidateVertices.size());
        auto& movesForVertex = benefitOfAddingVertexToClique[randomVertexIndex];
        // move into first clique with benefit 0
        int cliqueIndex = std::find(movesForVertex.begin(), movesForVertex.end(), 0) - movesForVertex.begin();
        return {randomVertexIndex, cliqueIndex};
    }

    auto randomCandidate = randomCandidateList[RandomNumberGenerator::getRandomNumberBelow(randomCandidateList.size())];

    return {std::get<0>(randomCandidate), std::get<1>(randomCandidate)};
}