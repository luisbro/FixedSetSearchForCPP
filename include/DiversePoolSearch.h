#ifndef DIVERSE_POOL_SEARCH_H
#define DIVERSE_POOL_SEARCH_H

#include <list>
#include <string>
#include <vector>

#include "SolutionClass.h"

class DiversePoolSearch {
   public:
    static std::vector<std::list<int>> run(
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        int numberOfTotalIterations,
        int timeLimit,
        double initialTemperature,
        double batchSizeScaleFactor,  // sigma
        const std::string& resultLogFileName = "",
        int desiredSize = 10,
        int improvementFactor = 3,
        double cooldownFactor = 0.96,  // Theta
        double minimalTransitionRatio = 0.01,
        int lengthOfRandomCandidateList = 2,  // alpha
        int numberOfGraspIterations = -1);

   private:
    static void logResults(
        const std::string& resultLogFileName,
        const std::vector<SolutionWithValueAndIndexLookup>& solutions);

    static SolutionWithValueAndIndexLookup tryImproveSolution(
        const SolutionWithValueAndIndexLookup& solution,
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        double initialTemperature,
        double batchSizeScaleFactor,
        double cooldownFactor,
        double minimalTransitionRatio,
        int improvementFactor);

    static std::vector<SolutionWithValueAndIndexLookup> filterSimilarSolutions(
        const std::vector<SolutionWithValueAndIndexLookup>& solutions,
        float similarityThreshold);
};

#endif  // DIVERSE_POOL_SEARCH_H