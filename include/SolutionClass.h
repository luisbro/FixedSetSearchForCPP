#ifndef SOLUTION_CLASS_H
#define SOLUTION_CLASS_H

#include <list>
#include <vector>

class SolutionWithValueAndIndexLookup {
   public:
    SolutionWithValueAndIndexLookup(
        const std::vector<std::list<int>>& partition,
        const std::vector<std::vector<int>>& weights);

    // default constructor
    SolutionWithValueAndIndexLookup() = default;

    std::vector<std::list<int>> partition;
    int value = 0;
    std::vector<int> cliqueIndexForVertex;

    bool operator==(const SolutionWithValueAndIndexLookup& other) const;

    bool operator!=(const SolutionWithValueAndIndexLookup& other) const;

    bool operator<(const SolutionWithValueAndIndexLookup& other) const;

    bool operator>(const SolutionWithValueAndIndexLookup& other) const;
};

#endif  // SOLUTION_CLASS_H