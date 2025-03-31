#include "DiversePoolSearch.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "GRASP.h"
#include "GreedyAdding.h"
#include "GreedyMoving.h"
#include "PartitionDistances.h"
#include "SimulatedAnnealing.h"
#include "SolutionClass.h"
#include "SolutionManager.h"
#include "partition-comparison.hxx"
#include "utils.h"

std::vector<std::list<int>> DiversePoolSearch::run(
    const std::vector<int>& vertices,
    const std::vector<std::vector<int>>& weights,
    int numberOfTotalIterations,
    int timeLimit,
    double initialTemperature,
    double batchSizeScaleFactor,
    const std::string& resultLogFileName,
    int desiredSize,
    int improvementFactor,
    double cooldownFactor,
    double minimalTransitionRatio,
    int lengthOfRandomCandidateList,
    int numberOfGraspIterations) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Initialize best solution and solutions list
    SolutionWithValueAndIndexLookup bestSolutionWithValues;
    std::vector<SolutionWithValueAndIndexLookup> solutionsWithValues;

    // Adjust GRASP iterations
    if (numberOfGraspIterations == -1) {
        numberOfGraspIterations = desiredSize * 3;
    }

    // Run GRASP algorithm to get initial solutions
    std::tie(bestSolutionWithValues, solutionsWithValues) = GRASP::run(
        numberOfGraspIterations,
        vertices,
        weights,
        lengthOfRandomCandidateList,
        initialTemperature,
        batchSizeScaleFactor,
        cooldownFactor,
        minimalTransitionRatio);

    // Filter out similar solutions
    solutionsWithValues = DiversePoolSearch::filterSimilarSolutions(solutionsWithValues, 0.01f);

    // Create a solution manager
    SolutionManager solutionManager(0.02f, desiredSize);

    // Initialize the solution manager with filtered solutions
    solutionManager.initialize(solutionsWithValues);

    // Main iteration loop
    for (int i = 0; i < numberOfTotalIterations; i++) {
        for (int j = 0; j < desiredSize && j < solutionManager.count(); j++) {
            // Check time limit
            auto currentTime = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            if (elapsedTime >= timeLimit) {
                break;
            }

            auto solution = solutionManager.getSolution(j);

            // Get partition
            auto partition = solution.partition;

            // Run simulated annealing
            std::vector<std::list<int>> simulatedAnnealingPartition = SimulatedAnnealing::run(
                partition,
                vertices,
                weights,
                initialTemperature,
                batchSizeScaleFactor,
                cooldownFactor,
                minimalTransitionRatio);

            SolutionWithValueAndIndexLookup newSolution(simulatedAnnealingPartition, weights);

            if (newSolution.value <= solutionManager.getWorstSolution().value) {
                continue;
            }

            // Skip if similar solution with higher value exists.
            // This is especially also the case, if the solution returned by SA
            // is the same as the solution from the pool it was based on.
            if (solutionManager.existsSimilarSolutionWithHigherValue(newSolution)) {
                continue;
            }

            // std::cout << "Try to improve solution: " << newSolution.value << std::endl;

            // Try to improve the solution
            newSolution = DiversePoolSearch::tryImproveSolution(
                newSolution,
                vertices,
                weights,
                initialTemperature,
                batchSizeScaleFactor,
                cooldownFactor,
                minimalTransitionRatio,
                improvementFactor);

            // std::cout << newSolution.value << std::endl;

            // Try to add the solution
            elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
            solutionManager.tryAddSolution(newSolution, elapsedTime);
        }
    }

    // Log results
    logResults(resultLogFileName, solutionManager.getAllSolutions());

    // Return the best solution
    return solutionManager.getBestSolution().partition;
}

// Helper function to try to improve a solution
SolutionWithValueAndIndexLookup DiversePoolSearch::tryImproveSolution(
    const SolutionWithValueAndIndexLookup& solution,
    const std::vector<int>& vertices,
    const std::vector<std::vector<int>>& weights,
    double initialTemperature,
    double batchSizeScaleFactor,
    double cooldownFactor,
    double minimalTransitionRatio,
    int improvementFactor) {
    SolutionWithValueAndIndexLookup improvedSolution = solution;
    bool improving = true;

    while (improving) {
        improving = false;
        std::vector<float> multipliers;

        // Add multipliers based on improvement factor
        multipliers.insert(multipliers.end(), 2 * improvementFactor, 48.0f);
        multipliers.insert(multipliers.end(), improvementFactor, 36.0f);
        multipliers.insert(multipliers.end(), improvementFactor, 24.0f);

        for (auto multiplier : multipliers) {
            double improveTemp = initialTemperature * std::pow(cooldownFactor, multiplier);
            std::vector<std::list<int>> tempPartition = SimulatedAnnealing::run(
                improvedSolution.partition,
                vertices,
                weights,
                improveTemp,
                batchSizeScaleFactor,
                cooldownFactor,
                minimalTransitionRatio);

            int value = utils::valueForPartition(tempPartition, weights);

            if (value > improvedSolution.value) {
                improvedSolution = SolutionWithValueAndIndexLookup(tempPartition, weights);
                improving = true;
                // std::cout << "Improved solution to: " << value << " at multiplier " << multiplier << std::endl;
                break;
            }
        }
    }

    return improvedSolution;
}

// Helper function to filter similar solutions
std::vector<SolutionWithValueAndIndexLookup> DiversePoolSearch::filterSimilarSolutions(
    const std::vector<SolutionWithValueAndIndexLookup>& solutions,
    float similarityThreshold) {
    // Extract clique index for each vertex from solutions
    std::vector<std::vector<int>> cliqueIndexForVertexList;
    for (const auto& solution : solutions) {
        cliqueIndexForVertexList.push_back(solution.cliqueIndexForVertex);
    }

    // Compute distances between solutions
    std::vector<std::vector<float>> distances = computeUpperDistanceMatrix(cliqueIndexForVertexList, Metric::RAND_ERROR);

    // Make distance matrix symmetric
    for (int i = 0; i < distances.size(); ++i) {
        for (int j = i + 1; j < distances.size(); ++j) {
            distances[j][i] = distances[i][j];
        }
    }

    // Filter solutions by removing similar ones with lower values
    std::vector<SolutionWithValueAndIndexLookup> filteredSolutions;
    for (int i = 0; i < solutions.size(); ++i) {
        bool keep = true;
        for (int j = 0; j < solutions.size(); ++j) {
            if (i != j && distances[i][j] < similarityThreshold && solutions[i].value < solutions[j].value) {
                keep = false;
                break;
            }
        }
        if (keep) {
            filteredSolutions.push_back(solutions[i]);
        }
    }

    return filteredSolutions;
}

// Helper function to log results
void DiversePoolSearch::logResults(
    const std::string& resultLogFileName,
    const std::vector<SolutionWithValueAndIndexLookup>& solutions) {
    // if no log file name is provided, return
    if (resultLogFileName.empty()) {
        return;
    }

    std::ofstream outFile(resultLogFileName);
    if (outFile.is_open()) {
        for (const auto& solution : solutions) {
            outFile << solution.value << std::endl;
        }
        outFile.close();
    } else {
        std::cerr << "Unable to open file: " << resultLogFileName << std::endl;
    }
}
