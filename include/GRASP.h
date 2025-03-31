#ifndef GRASP_H
#define GRASP_H

#include <list>
#include <vector>

#include "SolutionClass.h"

class GRASP {
   public:
    static std::pair<
        SolutionWithValueAndIndexLookup,
        std::vector<SolutionWithValueAndIndexLookup>>
    run(
        int numberOfDesiredSolutions,
        const std::vector<int>& vertices,
        const std::vector<std::vector<int>>& weights,
        int lengthOfRandomCandidateList,
        double initialTemperature,
        double batchSizeScaleFactor,
        double cooldownFactor,
        double minimalTransitionRatio);
};

#endif  // GRASP_H
