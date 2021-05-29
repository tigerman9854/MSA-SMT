#include "MSA.h"
#include "stdio.h"

namespace
{
    // Hardcoded input for now
    const char* tempInput[3] = { "AAAGT", "AAGT", "GAAGT" };
    const int k = 6;

}

void getInput(Input& input)
{
    // TODO: Implement some way to ask the user for input

    // Store the sequences
    input.m = (int)(sizeof(tempInput) / sizeof(tempInput[0]));
    for (int i = 0; i < input.m; ++i) {
        std::vector<char> row;

        const char* sequence = tempInput[i];
        const int length = (int)strlen(sequence);
        for (int j = 0; j < length; ++j) {
            row.push_back(sequence[j]);
        }

        input.rawInput.push_back(row);
        input.n.push_back(length);
    }

    // Store the max length
    input.k = k;

    // Compute unique chararcters
    for (auto it : input.rawInput) {
        for (char c : it) {
            input.uniqueChars.insert(c);
        }
    }
    input.base = (int)input.uniqueChars.size() + 1;
}


void printInput(const Input& input)
{
    printf("Input:\n");
    for (const std::vector<char>& it : input.rawInput) {
        for (const char c : it) {
            printf("%c", c);
        }
        printf("\n");
    }

    printf("\nEncoded Input:\n");
    for (const std::vector<int>& it : input.encodedInput) {
        for (auto val : it) {
            printf("%d ", val);
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
    Output output;

    getInput(input);
    encodeInput5(input);
    printInput(input);
    MSAMethod5(input, output);
    printOutput(output);

    return 0;
}
