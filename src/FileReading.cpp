#include "FileReading.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::pair<std::vector<int>, std::vector<std::vector<int>>> FileReading::readProblemFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return {{}, {}};
    }

    int numberOfVertices;
    file >> numberOfVertices;

    // Initialize the vertices vector
    std::vector<int> vertices(numberOfVertices);
    for (int i = 0; i < numberOfVertices; ++i) {
        vertices[i] = i;
    }

    // Initialize the weights matrix
    std::vector<std::vector<int>> weights(numberOfVertices, std::vector<int>(numberOfVertices, 0));

    // Read the data in CPn format
    for (int i = 0; i < numberOfVertices; ++i) {
        for (int j = i + 1; j < numberOfVertices; ++j) {
            int weight;
            file >> weight;
            weights[i][j] = weight;
            weights[j][i] = weight;  // Make the matrix symmetrical
        }
    }

    file.close();

    return {vertices, weights};
}
