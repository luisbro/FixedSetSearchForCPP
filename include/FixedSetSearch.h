#ifndef FIXED_SET_SEARCH_H
#define FIXED_SET_SEARCH_H

#include <list>
#include <vector>

#include "SolutionClass.h"

class FixedSetSearch {
   public:
    static std::vector<std::list<int>> run(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        int numberOfTotalIterations,
        int timeLimit,
        double initialTemperature,
        double batchSizeScaleFactor,  // sigma
        int m = 10,
        int n = 50,
        int k = 10,
        int maximumStagnationCountPerPortion = 20,
        int numberOfGRASPIterations = 10,
        int lengthOfRandomCandidateList = 2,  // alpha
        double cooldownFactor = 0.96,         // Theta
        double minimalTransitionRatio = 0.01);

    static std::vector<std::list<int>> findFixedPartialSolution(
        const std::vector<std::list<int>>& baseSolution,
        const std::vector<SolutionWithValueAndIndexLookup>& solutionsForFixedSet,
        double portionOfFixedVertices);

   private:
    static void shuffleVector(
        std::vector<SolutionWithValueAndIndexLookup>& vec);

    static std::vector<double> initializeFixedSetSizePortions(
        int numberOfVertices);
};

#endif  // FIXED_SET_SEARCH_H
