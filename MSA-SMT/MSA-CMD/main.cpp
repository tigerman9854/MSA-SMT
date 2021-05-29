#include "MSA.h"
#include "stdio.h"

#include <iostream>
#include <string>

void getInput(Input& input)
{
    // Ask the user for sequences
    while (true) {
        printf("Enter a sequence. Leave blank if done: ");

        std::string seq;
        std::getline(std::cin, seq);

        if (!seq.length()) {
            break;
        }

        std::vector<char> row;
        for (auto c : seq) {
            row.push_back(c);
        }
        input.rawInput.push_back(row);
    }

    // Store the max length
    printf("Enter the max alignment length: ");
    std::cin >> input.k;
}


void printInput(const Input& input)
{
    printf("\nInput:\n");
    for (const std::vector<char>& it : input.rawInput) {
        for (const char c : it) {
            printf("%c", c);
        }
        printf("\n");
    }
}

void printOutput(const Output& output)
{
    if (output.isSAT) {
        printf("\nEncoded Output:\n");
        for (const std::vector<int>& it : output.encodedOutput) {
            for (auto val : it) {
                printf("%d ", val);
            }
            printf("\n");
        }

        printf("\nDecoded Output:\n");
        for (const std::vector<char>& it : output.decodedOutput) {
            for (auto val : it) {
                printf("%c", val);
            }
            printf("\n");
        }
    }
    else {
        printf("\nNo solution.\n");
    }
}

int main()
{
    Input input;

    getInput(input);
    Output output = computeMSA(input);
    printInput(input);
    printOutput(output);

    return 0;
}
