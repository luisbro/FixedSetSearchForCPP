#include <list>
#include <string>
#include <vector>

#include "PartitionDistances.h"
#include "partition-comparison.hxx"

std::vector<std::vector<float>> computeUpperDistanceMatrix(const std::vector<std::vector<int>>& solutions,
                                                           Metric metric) {
    int numberOfSolutions = solutions.size();

    std::vector<std::vector<float>> distances(numberOfSolutions, std::vector<float>(numberOfSolutions));

    for (int i = 0; i < numberOfSolutions; ++i) {
        for (int j = i + 1; j < numberOfSolutions; ++j) {
            double distance = 0.0;
            switch (metric) {
                case Metric::RAND_ERROR:
                    distance = andres::RandError(solutions[i].begin(),
                                                 solutions[i].end(),
                                                 solutions[j].begin())
                                   .error();
                    break;
                case Metric::VI:
                    distance = andres::VariationOfInformation(solutions[i].begin(),
                                                              solutions[i].end(),
                                                              solutions[j].begin())
                                   .value();
                    break;
            }
            distances[i][j] = static_cast<float>(distance);
        }
    }

    return distances;
}