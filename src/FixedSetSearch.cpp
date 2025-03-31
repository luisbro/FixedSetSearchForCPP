#include "FixedSetSearch.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <random>
#include <tuple>
#include <vector>

#include "GRASP.h"
#include "GreedyAdding.h"
#include "GreedyMoving.h"
#include "PartitionDistances.h"
#include "SimulatedAnnealing.h"
#include "SolutionClass.h"

std::vector<std::list<int>> FixedSetSearch::run(const std::vector<int>& vertices,
                                                const std::vector<std::vector<int>>& weights,
                                                int numberOfTotalIterations,
                                                int timeLimit,
                                                double initialTemperature,
                                                double batchSizeScaleFactor,
                                                int m,
                                                int n,
                                                int k,
                                                int maximumStagnationCountPerPortion,
                                                int numberOfGRASPIterations,
                                                int lengthOfRandomCandidateList,
                                                double cooldownFactor,
                                                double minimalTransitionRatio) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<double> fixedSetSizePortions = initializeFixedSetSizePortions(vertices.size());
    int currentPortionIndex = 0;
    double currentPortion = fixedSetSizePortions[currentPortionIndex];

    // store solutions along with their values
    SolutionWithValueAndIndexLookup bestSolutionWithValues;
    std::vector<SolutionWithValueAndIndexLookup> solutionsWithValues;
    int numberOfSolutionsToStore = std::max(n, m);

    std::tie(bestSolutionWithValues, solutionsWithValues) = GRASP::run(numberOfGRASPIterations,
                                                                       vertices,
                                                                       weights,
                                                                       lengthOfRandomCandidateList,
                                                                       initialTemperature,
                                                                       batchSizeScaleFactor,
                                                                       cooldownFactor,
                                                                       minimalTransitionRatio);

    int stagnationCounter = 0;

    // boolean to track, whether we need to sort the list of solutions again
    bool addedPartitionToSolutions = true;

    for (int iteration = numberOfGRASPIterations; iteration < numberOfTotalIterations; ++iteration) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime >= timeLimit) {
            break;
        }

        if (addedPartitionToSolutions) {
            // Sort solutions by their values
            std::sort(solutionsWithValues.begin(), solutionsWithValues.end(),
                      [](const SolutionWithValueAndIndexLookup& a, const SolutionWithValueAndIndexLookup& b) {
                          return a.value > b.value;
                      });
        }

        int currentNumberOfSolutions = static_cast<int>(solutionsWithValues.size());
        int m_restricted = std::min(m, currentNumberOfSolutions);  // to choose base solution from
        int n_restricted = std::min(n, currentNumberOfSolutions);  // to choose k solutions from
        int k_restricted = std::min(k, currentNumberOfSolutions);  // to choose solutions for building the fixed partial solution

        // copy the solutions to a new vector, choose the n best and shuffle them to get k random solutions for the fixed partial solution
        std::vector<SolutionWithValueAndIndexLookup> solutionsForFixedSet(solutionsWithValues.begin(), solutionsWithValues.begin() + n_restricted);
        shuffleVector(solutionsForFixedSet);
        solutionsForFixedSet.resize(k_restricted);

        // Select the m best solutions for base solution
        std::vector<SolutionWithValueAndIndexLookup> solutionsForBaseSolution(solutionsWithValues.begin(), solutionsWithValues.begin() + m_restricted);
        int index = rand() % solutionsForBaseSolution.size();
        const std::vector<std::list<int>>& baseSolution = solutionsForBaseSolution[index].partition;

        double duration;

        // Find fixed partial solution
        std::vector<std::list<int>> fixedPartialSolution;
        fixedPartialSolution = findFixedPartialSolution(baseSolution, solutionsForFixedSet, currentPortion);

        // Perform GRASP and local search on the new partition
        std::vector<std::list<int>> partition;
        partition = GreedyAdding::run(vertices, weights, fixedPartialSolution, lengthOfRandomCandidateList);
        partition = GreedyMoving::run(vertices, weights, partition);
        partition = SimulatedAnnealing::run(partition, vertices, weights, initialTemperature, batchSizeScaleFactor, cooldownFactor, minimalTransitionRatio);

        auto newSolution = SolutionWithValueAndIndexLookup(partition, weights);

        // add the solution, if it is not already in the solutionsWithValues vector
        bool newUniqueSolution = std::none_of(solutionsWithValues.begin(), solutionsWithValues.end(),
                                              [&](const SolutionWithValueAndIndexLookup& sol) { return sol == newSolution; });

        int currentWorstValue = solutionsWithValues.back().value;
        bool improvedValue = newSolution.value > currentWorstValue;

        bool solutionsFull = solutionsWithValues.size() >= numberOfSolutionsToStore;

        if (newUniqueSolution && !solutionsFull) {
            solutionsWithValues.push_back(newSolution);
            addedPartitionToSolutions = true;
        } else if (newUniqueSolution && improvedValue) {
            solutionsWithValues.pop_back();
            solutionsWithValues.push_back(newSolution);
            addedPartitionToSolutions = true;
        } else {
            addedPartitionToSolutions = false;
        }

        // Update the best solution if a better one is found
        if (newSolution.value > bestSolutionWithValues.value) {
            elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            std::cout << "New best: " << newSolution.value << "    Iteration: " << iteration << "    Time: " << elapsedTime << " seconds." << std::endl;
            bestSolutionWithValues = newSolution;
            stagnationCounter = 0;
        } else {
            stagnationCounter++;
        }

        // Adjust portion and reset stagnation counter if needed
        if (stagnationCounter >= maximumStagnationCountPerPortion) {
            currentPortionIndex = (currentPortionIndex + 1) % fixedSetSizePortions.size();
            currentPortion = fixedSetSizePortions[currentPortionIndex];
            stagnationCounter = 0;
        }
    }

    return bestSolutionWithValues.partition;
}

void FixedSetSearch::shuffleVector(std::vector<SolutionWithValueAndIndexLookup>& vec) {
    static std::random_device rd;
    static std::mt19937 g(rd());
    std::shuffle(vec.begin(), vec.end(), g);
}

std::vector<double> FixedSetSearch::initializeFixedSetSizePortions(int numberOfVertices) {
    // The portions of free vertices are negative powers of 2
    // The maximal portion of fixed vertices should be such, that the number of free vertices is just below 10 (see C# implementation)
    // (or one could also say just above 5 as the number of free vertices halfs with each portion)
    // This could be achieved by the following formula:
    // maxPortionNumber = ceil(log2(numberOfVertices / 10))
    // or
    // maxPortionNumber = floor(log2(numberOfVertices / 5))
    // where the maxPortionNumber is the maximal negative power of 2
    // which can be derived from the condition:
    // numberOfVertices / 2^(maxPortionNumber) > 5

    int maxPortionNumber = static_cast<int>(floor(log2(static_cast<double>(numberOfVertices) / 5.0)));

    std::vector<double> fixedSetSizePortions;
    for (int i = 1; i <= maxPortionNumber; ++i) {
        fixedSetSizePortions.push_back(1.0 - pow(2.0, -i));
    }

    return fixedSetSizePortions;
}

std::vector<std::list<int>> FixedSetSearch::findFixedPartialSolution(const std::vector<std::list<int>>& baseSolution,
                                                                     const std::vector<SolutionWithValueAndIndexLookup>& solutionsForFixedSet,
                                                                     double portionOfFixedVertices) {
    std::vector<std::pair<int, double>> vertexSimilarityScores;

    // Compute similarity scores
    for (const auto& cliqueOfBaseSolution : baseSolution) {
        for (int vertex : cliqueOfBaseSolution) {
            double similarityScoreForVertex = 0;
            for (const auto& solution : solutionsForFixedSet) {
                int cliqueIndexInCompareSolution = solution.cliqueIndexForVertex[vertex];
                int numberOfSameVertices = 0;

                for (int v : cliqueOfBaseSolution) {
                    if (solution.cliqueIndexForVertex[v] == cliqueIndexInCompareSolution) {
                        ++numberOfSameVertices;
                    }
                }

                similarityScoreForVertex += static_cast<double>(numberOfSameVertices);
            }
            similarityScoreForVertex /= static_cast<double>(cliqueOfBaseSolution.size());
            vertexSimilarityScores.push_back({vertex, similarityScoreForVertex});
        }
    }

    // Sort vertices by similarity score
    std::sort(vertexSimilarityScores.begin(), vertexSimilarityScores.end(),
              [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                  return a.second > b.second;
              });

    // Select the top portion of fixed vertices
    int sizeOfFixedPartialSolution = static_cast<int>(round(portionOfFixedVertices * vertexSimilarityScores.size()));
    std::vector<int> verticesToKeep;
    for (int i = 0; i < sizeOfFixedPartialSolution; ++i) {
        verticesToKeep.push_back(vertexSimilarityScores[i].first);
    }

    // Create the fixed partial solution
    std::vector<std::list<int>> fixedPartialSolution;
    for (const auto& clique : baseSolution) {
        std::list<int> newClique;
        for (int vertex : clique) {
            if (std::find(verticesToKeep.begin(), verticesToKeep.end(), vertex) != verticesToKeep.end()) {
                newClique.push_back(vertex);
            }
        }
        fixedPartialSolution.push_back(newClique);
    }

    return fixedPartialSolution;
}
