#include "SolutionClass.h"

#include <list>
#include <vector>

#include "utils.h"

SolutionWithValueAndIndexLookup::SolutionWithValueAndIndexLookup(const std::vector<std::list<int>>& partition,
                                                                 const std::vector<std::vector<int>>& weights) : partition(partition) {
    value = utils::valueForPartition(partition, weights);

    int numberOfVertices = weights.size();
    int numberOfCliques = partition.size();
    cliqueIndexForVertex.resize(numberOfVertices);

    for (size_t i = 0; i < numberOfCliques; ++i) {
        for (int vertex : partition[i]) {
            cliqueIndexForVertex[vertex] = i;
        }
    }
}

bool SolutionWithValueAndIndexLookup::operator==(const SolutionWithValueAndIndexLookup& other) const {
    if (value != other.value) {
        return false;
    }

    if (partition.size() != other.partition.size()) {
        return false;
    }

    // check for each vertex if it is in the same clique in both solutions
    for (auto& clique : partition) {
        if (clique.empty()) {
            continue;
        }

        int someVertexInClique = clique.front();
        int cliqueIndexInOtherSolution = other.cliqueIndexForVertex[someVertexInClique];

        for (int vertex : clique) {
            if (other.cliqueIndexForVertex[vertex] != cliqueIndexInOtherSolution) {
                return false;
            }
        }
    }

    return true;
}

bool SolutionWithValueAndIndexLookup::operator!=(const SolutionWithValueAndIndexLookup& other) const {
    return !(*this == other);
}

bool SolutionWithValueAndIndexLookup::operator<(const SolutionWithValueAndIndexLookup& other) const {
    return value < other.value;
}

bool SolutionWithValueAndIndexLookup::operator>(const SolutionWithValueAndIndexLookup& other) const {
    return value > other.value;
}