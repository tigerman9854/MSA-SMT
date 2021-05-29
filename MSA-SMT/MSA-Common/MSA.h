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
    bool tightConstraints = false;




    // Encoded input
    std::vector<std::vector<int>> encodedInput = {};

    // Length of each sequence
    std::vector<int> n = {};

    // Number of sequences
    int m = 0;

    int maxBlanks = 0;

    // Blanks in each sequence
    std::vector<int> blanks = {};

    // Number of unique characters
    int base = 0;

    // Set of unique characters
    std::set<char> uniqueChars = {};

    // Encoding map
    std::map<char, int> encoding = {};

    // Decoding map
    std::map<int, char> decoding = {};
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
