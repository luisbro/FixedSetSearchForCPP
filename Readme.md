# Fixed Set Search for Clique Partitioning

## Academic Context
This project was developed as part of a university assignment and is shared for reference and learning purposes. While effort has been made to create a functional implementation, it represents student work and may contain imperfections. Consider it an educational exploration of the Fixed Set Search algorithm rather than a fully optimized production implementation.

## Project Overview
This project implements the **Fixed Set Search** algorithm for solving the **Clique Partitioning Problem** (CPP) in C++. It's based on the algorithm described in:

Jovanovic, R., Sanfilippo, A. P., & Voß, S. (2023). [Fixed set search applied to the clique partitioning problem](https://doi.org/10.1016/j.ejor.2023.01.044). *European Journal of Operational Research, 309(1)*, 65-81.

While the original implementation was in C#, this C++ version introduces:
- Complete reimplementation of the core algorithm
- A new experimental variation called **Diverse Pool Search** (though it doesn't appear to improve performance)
- Various minor bug fixes and optimizations

**Note:** Due to differences in handling randomness and certain implementation details, results may differ between this version and the original. While individual components have been tested separately, comprehensive verification against the original implementation has not been performed.

## Installation

### Dependencies
This project includes the following header-only library:
- [bjoern-andres/partition-comparison](https://github.com/bjoern-andres/partition-comparison) for evaluating partition distances.

This library is **already included** in the `include` directory, so no additional setup is needed.

### Input Instances
The program can process any clique partitioning problem instances, provided they follow the expected format. For example instances, the [CP-Lib repository](https://github.com/MMSorensen/CP-Lib) is recommended as a source of benchmark instances.

### Configuration (Before Building)
Before building the project, you need to make adjustments to `main.cpp`:

1. Open `main.cpp` and locate the following section:
   ```cpp
   // --- Input Data Configuration ---
   // IMPORTANT: Change this path to your test instance
   std::string filepath = "/path/to/instance/folder/instance_name.txt";
   ```

2. Replace `/path/to/instance/folder/instance_name.txt` with the actual path to your problem instance file.

   For example, if you've downloaded the CP-Lib instances to `/home/user/CP-Lib/`, your configuration might look like:
   ```cpp
   std::string filepath = "/home/user/CP-Lib/Random/p1000-1.txt";
   ```

3. (Optional) Set any algorithm parameters as needed.

### Building the Project
After configuring `main.cpp`, you can now proceed to build the project. The project was built using **Ubuntu** on **WSL** (Windows Subsystem for Linux) with the following tools and libraries:
- **CMake** (for configuring the build system)
- **Make** (for compiling the project)
- **GCC** (with C++17 support)

To build the project, follow these steps:

1. From project root, create a build directory and navigate into it:
   ```sh
   mkdir build
   cd build
   ```

2. Run CMake to configure the project:
   ```sh
   cmake ..
   ```

3. Compile the project:
   ```sh
   make
   ```

This will generate the executable you can run.

## Usage

### Running the Program

After configuring the path to your problem instance and building, run the executable within the build folder:

```sh
./fixed_set_search_exe
```

The program will read the adjacency matrix from the file path you specified in `main.cpp` and begin processing.

## Customization

To change the input data or algorithm parameters, modify the `main` function in `main.cpp`. Some customizations include:

- Setting a different input data file path
- Adjusting algorithm parameters
- Selecting between Fixed Set Search and Diverse Pool Search variants

## License
This project is released under the GNU General Public License (GPL) v3.0. See the [LICENSE](LICENSE) file for details.

### Third-Party Libraries
This project includes the following third-party libraries:
1. **PartitionComparison** - MIT License
   - Copyright (c) 2016 by Bjoern Andres
   - [GitHub Repository](https://github.com/bjoern-andres/partition-comparison)
   - When using this code, please cite the repository

The full license texts for included libraries are in their respective directories within the `include` folder.

## References
- Jovanovic, R., Sanfilippo, A. P., & Voß, S. (2023). Fixed set search applied to the clique partitioning problem. *European Journal of Operational Research, 309(1)*, 65-81. https://doi.org/10.1016/j.ejor.2023.01.044
- [Original C# implementation](https://github.com/rakabog/CPPConsole)
- Sørensen, M. M., & Letchford, A. N. (2024). CP-Lib: Benchmark Instances of the Clique Partitioning Problem. *Mathematical Programming Computation, 16(1)*, 93-111. https://doi.org/10.1007/s12532-023-00249-1
- [CP-Lib repository](https://github.com/MMSorensen/CP-Lib)
- [Partition Comparison repository](https://github.com/bjoern-andres/partition-comparison)