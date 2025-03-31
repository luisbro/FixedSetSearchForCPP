#include <filesystem>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "DiversePoolSearch.h"
#include "FileReading.h"
#include "FixedSetSearch.h"
#include "SimulatedAnnealing.h"
#include "Timer.h"
#include "utils.h"

int main() {
    // --- Input Data Configuration ---
    // IMPORTANT: Change this path to your test instance
    std::string filepath = "/path/to/instance/folder/instance_name.txt";

    // --- Read Problem Data ---
    std::cout << "Reading problem data from: " << filepath << std::endl;
    auto [vertices, weights] = FileReading::readProblemFromFile(filepath);

    // --- Simulated Annealing Parameters ---
    std::cout << "Calculating initial temperature..." << std::endl;
    double batchSizeScaleFactor = 8;
    double initialTemperature = SimulatedAnnealing::CalculateSimulatedAnnealingTemperature(vertices, weights, batchSizeScaleFactor);

    // --- Search Parameters ---
    int numberOfTotalIterations = 10'000;
    int timeLimitInSeconds = 20 * 60;  // 20 minutes

    // --- Variables to Store Results ---
    std::vector<std::list<int>> bestPartition;
    double duration = 0;

    // --- Choose Search Method ---
    bool useDiversePoolSearch = false;
    if (useDiversePoolSearch) {
        // --- Run Diverse Pool Search ---
        std::string resultLogFileName = "";  // Change to store results in a file
        std::cout << "Running Diverse Pool Search..." << std::endl;
        duration = measureExecutionTime([&]() {
            bestPartition = DiversePoolSearch::run(
                vertices,
                weights,
                numberOfTotalIterations,
                timeLimitInSeconds,
                initialTemperature,
                batchSizeScaleFactor,
                resultLogFileName);
        });
    } else {
        // --- Run Fixed Set Search ---
        std::cout << "Running Fixed Set Search..." << std::endl;
        duration = measureExecutionTime([&]() {
            bestPartition = FixedSetSearch::run(
                vertices,
                weights,
                numberOfTotalIterations,
                timeLimitInSeconds,
                initialTemperature,
                batchSizeScaleFactor);
        });
    }

    // --- Output Results ---
    std::cout << "Duration: " << duration << " s" << std::endl;
    std::cout << "Value for Best Partition: " << utils::valueForPartition(bestPartition, weights) << std::endl;

    return 0;
}
