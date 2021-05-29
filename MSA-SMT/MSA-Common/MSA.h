#pragma once
#include <vector>
#include <map>
#include <set>

struct Input {
    // Length we're testing
    int k = 0;

    // Input sequences
    std::vector<std::vector<char>> rawInput = {};

    // Tight constraints
    bool tightConstraints = true;



    // Number of sequences
    int m = 0;

    // Length of each sequence
    std::vector<int> n = {};

    // Blanks in each sequence
    std::vector<int> blanks = {};
};

struct Output {
    // Whether the input was satisfiable or not
    bool isSAT = false;

    // Time it took to compute
    int64_t nsec = 0;

    // Output in terms of numbers (2D array)
    std::vector<std::vector<int>> encodedOutput = {};

    // Output in terms of characters (2D array)
    std::vector<std::vector<char>> decodedOutput = {};
};


// Expects input to have at least rawInput and k populated
Output computeMSA(Input& input);
