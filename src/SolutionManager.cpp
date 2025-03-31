#include "SolutionManager.h"

#include <algorithm>
#include <iostream>

#include "partition-comparison.hxx"

bool SolutionManager::isSimilar(const SolutionWithValueAndIndexLookup& solution1,
                                const SolutionWithValueAndIndexLookup& solution2) const {
    double distance = andres::RandError(
                          solution1.cliqueIndexForVertex.begin(),
                          solution1.cliqueIndexForVertex.end(),
                          solution2.cliqueIndexForVertex.begin())
                          .error();

    return distance < similarityThreshold;
}

int SolutionManager::findInsertPosition(const SolutionWithValueAndIndexLookup& candidateSolution) const {
    // Using std::find_if to find the first solution with lower value
    auto it = std::find_if(solutions.begin(), solutions.end(),
                           [&candidateSolution](const SolutionWithValueAndIndexLookup& solution) {
                               return candidateSolution.value > solution.value;
                           });

    return std::distance(solutions.begin(), it);
}

void SolutionManager::removeSimilarSolutionsWithLowerValue(const SolutionWithValueAndIndexLookup& solution,
                                                           int startIndex) {
    for (int i = startIndex + 1; i < solutions.size(); i++) {
        if (isSimilar(solution, solutions[i])) {
            solutions.erase(solutions.begin() + i);
            i--;  // Adjust index after removal
        }
    }
}

SolutionManager::SolutionManager(float similarityThreshold, int maxCapacity)
    : similarityThreshold(similarityThreshold), maxCapacity(maxCapacity) {
}

void SolutionManager::initialize(const std::vector<SolutionWithValueAndIndexLookup>& initialSolutions) {
    solutions.clear();

    // Sort solutions by value and take up to maxCapacity
    solutions = initialSolutions;
    std::sort(solutions.begin(), solutions.end(),
              [](const SolutionWithValueAndIndexLookup& a, const SolutionWithValueAndIndexLookup& b) {
                  return a.value > b.value;
              });

    if (solutions.size() > maxCapacity) {
        solutions.resize(maxCapacity);
    }
}

size_t SolutionManager::count() const {
    return solutions.size();
}

SolutionWithValueAndIndexLookup SolutionManager::getSolution(int index) const {
    return solutions[index];
}

bool SolutionManager::similarSolutionExists(const SolutionWithValueAndIndexLookup& candidateSolution) const {
    for (const auto& existingSolution : solutions) {
        if (isSimilar(candidateSolution, existingSolution)) {
            return true;
        }
    }

    return false;
}

bool SolutionManager::existsSimilarSolutionWithHigherValue(const SolutionWithValueAndIndexLookup& candidateSolution) const {
    int insertPosition = findInsertPosition(candidateSolution);

    for (int i = 0; i < insertPosition; i++) {
        if (isSimilar(candidateSolution, solutions[i])) {
            return true;
        }
    }

    return false;
}

void SolutionManager::tryAddSolution(const SolutionWithValueAndIndexLookup& candidateSolution,
                                     double elapsedTime) {
    // Find insertion position
    int insertPosition = findInsertPosition(candidateSolution);

    // Check if the solution is better than the worst solution
    if (insertPosition >= maxCapacity) {
        // std::cout << "Solution is worse than the worst solution" << std::endl;
        return;
    }

    // Check if similar solutions with higher values exist
    if (existsSimilarSolutionWithHigherValue(candidateSolution)) {
        // std::cout << "Similar solution with higher value exists" << std::endl;
        return;
    }

    // Insert the solution at the right position
    solutions.insert(solutions.begin() + insertPosition, candidateSolution);

    // Print value, if new best solution
    if (insertPosition == 0) {
        std::cout << "New best: " << candidateSolution.value << "    Time: " << elapsedTime << " seconds." << std::endl;
    }

    // Remove similar solutions with lower values
    removeSimilarSolutionsWithLowerValue(candidateSolution, insertPosition);

    // Truncate to max capacity if needed
    if (solutions.size() > maxCapacity) {
        solutions.resize(maxCapacity);
    }
}

SolutionWithValueAndIndexLookup SolutionManager::getBestSolution() const {
    return solutions[0];
}

SolutionWithValueAndIndexLookup SolutionManager::getWorstSolution() const {
    return solutions.back();
}

const std::vector<SolutionWithValueAndIndexLookup>& SolutionManager::getAllSolutions() const {
    return solutions;
}
