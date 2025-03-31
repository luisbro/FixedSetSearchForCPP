#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <list>
#include <vector>

namespace utils {
inline int valueForPartition(const std::vector<std::list<int>>& partition,
                             const std::vector<std::vector<int>>& weights) {
    int score = 0;

    for (const auto& clique : partition) {
        // Convert list to vector for easier indexing
        std::vector<int> verticesInClique(clique.begin(), clique.end());

        // Iterate over all unique pairs of vertices in the clique
        for (size_t i = 0; i < clique.size(); ++i) {
            for (size_t j = i + 1; j < clique.size(); ++j) {
                int v1 = verticesInClique[i];
                int v2 = verticesInClique[j];
                score += weights[v1][v2];  // Add the weight of the edge between v1 and v2
            }
        }
    }

    return score;
}

inline int cliqueIndexForVertex(int vertex,
                                const std::vector<std::list<int>>& partition) {
    for (size_t i = 0; i < partition.size(); ++i) {
        if (std::find(partition[i].begin(), partition[i].end(), vertex) != partition[i].end()) {
            return static_cast<int>(i);  // Found the vertex in this clique
        }
    }
    return -1;  // Vertex not found in any clique
}
}  // namespace utils

#endif  // UTILS_H
