#ifndef SOLUTION_MANAGER_H
#define SOLUTION_MANAGER_H

#include <algorithm>
#include <list>
#include <vector>

#include "SolutionClass.h"
#include "partition-comparison.hxx"

class SolutionManager {
   public:
    SolutionManager(
        float similarityThreshold = 0.02f,
        int maxCapacity = 10);

    void initialize(
        const std::vector<SolutionWithValueAndIndexLookup>& initialSolutions);

    size_t count() const;

    SolutionWithValueAndIndexLookup getSolution(
        int index) const;

    bool similarSolutionExists(
        const SolutionWithValueAndIndexLookup& candidateSolution) const;

    bool existsSimilarSolutionWithHigherValue(
        const SolutionWithValueAndIndexLookup& candidateSolution) const;

    void tryAddSolution(
        const SolutionWithValueAndIndexLookup& candidateSolution, double elapsedTime);

    SolutionWithValueAndIndexLookup getBestSolution() const;

    SolutionWithValueAndIndexLookup getWorstSolution() const;

    const std::vector<SolutionWithValueAndIndexLookup>& getAllSolutions() const;

   private:
    std::vector<SolutionWithValueAndIndexLookup> solutions;
    std::vector<int> attemptCounts;
    float similarityThreshold;
    int maxCapacity;

    bool isSimilar(
        const SolutionWithValueAndIndexLookup& solution1,
        const SolutionWithValueAndIndexLookup& solution2) const;

    int findInsertPosition(
        const SolutionWithValueAndIndexLookup& candidateSolution) const;

    void removeSimilarSolutionsWithLowerValue(
        const SolutionWithValueAndIndexLookup& solution,
        int startIndex);
};

#endif  // SOLUTION_MANAGER_H