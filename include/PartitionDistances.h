#ifndef PARTITION_DISTANCES_H
#define PARTITION_DISTANCES_H

#include <list>
#include <string>
#include <vector>

enum Metric {
    RAND_ERROR,
    VI,
};

std::vector<std::vector<float>> computeUpperDistanceMatrix(
    const std::vector<std::vector<int>>& solutions,
    Metric metric = Metric::RAND_ERROR);

#endif  // PARTITION_DISTANCES_H