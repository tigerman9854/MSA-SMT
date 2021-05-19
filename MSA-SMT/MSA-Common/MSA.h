#pragma once
#include <vector>
#include <map>
#include <set>

struct Input {
    // Input sequences
    std::vector<std::vector<char>> rawInput = {};

    // Encoded input
    std::vector<std::vector<int>> encodedInput = {};

    // Length of each sequence
    std::vector<int> n = {};

    // Number of sequences
    int m = 0;

    // Length we're testing
    int k = 0;

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

    // Output in terms of numbers (2D array)
    std::vector<std::vector<int>> encodedOutput = {};

    // Output in terms of characters (2D array)
    std::vector<std::vector<char>> decodedOutput = {};
};


Output computeMSA(Input& input, bool tightConstraints = false, int method = 2);

void getInput(Input& input);
void printInput(const Input& input);

// Method 2
void encodeInput2(Input& input);
void MSAMethod2(const Input& input, Output& output);

// Method 3
void encodeInput3(Input& input);
void MSAMethod3(const Input& input, Output& output);

void printOutput(const Output& output);