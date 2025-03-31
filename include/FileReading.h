#ifndef FILEREADING_H
#define FILEREADING_H

#include <string>
#include <vector>

class FileReading {
   public:
    static std::pair<std::vector<int>, std::vector<std::vector<int>>> readProblemFromFile(const std::string& filename);
};

#endif  // FILEREADING_H
