#include "GRASP.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

#include "GreedyAdding.h"
#include "GreedyMoving.h"
#include "SimulatedAnnealing.h"
#include "SolutionClass.h"
#include "utils.h"

std::pair<
    SolutionWithValueAndIndexLookup,
    std::vector<SolutionWithValueAndIndexLookup>>
GRASP::run(int numberOfDesiredSolutions,
           const std::vector<int>& vertices,
           const std::vector<std::vector<int>>& weights,
           int lengthOfRandomCandidateList,
           double initialTemperature,
           double batchSizeScaleFactor,
           double cooldownFactor,
           double minimalTransitionRatio) {
    std::vector<SolutionWithValueAndIndexLookup> solutionsWithValues;

    for (int i = 0; i < numberOfDesiredSolutions; ++i) {
        std::vector<std::list<int>> partition;
        int value;
        double duration;

        partition = GreedyAdding::runForEmptyPartition(vertices, weights, lengthOfRandomCandidateList);
        partition = GreedyMoving::run(vertices, weights, partition);
        partition = SimulatedAnnealing::run(partition, vertices, weights, initialTemperature, batchSizeScaleFactor, cooldownFactor, minimalTransitionRatio);

        auto newSolution = SolutionWithValueAndIndexLookup(partition, weights);

        // add the solution, if it is not already in the solutionsWithValues vector
        if (std::find(solutionsWithValues.begin(), solutionsWithValues.end(), newSolution) == solutionsWithValues.end()) {
            solutionsWithValues.push_back(newSolution);
        }
    }

    SolutionWithValueAndIndexLookup bestSolutionWithValue = *std::max_element(solutionsWithValues.begin(), solutionsWithValues.end());

    return {bestSolutionWithValue, solutionsWithValues};
}
