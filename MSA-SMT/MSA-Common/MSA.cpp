#include "MSA.h"
#include "z3++.h"

#include "stdio.h"
#include <algorithm> // std::find


namespace 
{
    // Hardcoded input for now
    //const char* tempInput[3] = { "CGTCGCCACCGCCGGCTACGACAAC", "CGTCGCCACCGCCGGCTACGATAAC", "CGTCGTCACCGCCGGCTACGACAAC" };
    //const int k = 27;
    const char* tempInput[3] = { "AAAGT", "AAGT", "GAAGT" };
    const int k = 6;

    // Helper function to retrieve a symbol from an array of symbols
    z3::expr GetSymbol(const std::vector<z3::expr>& symbols, const Input& input, const int r, const int c) {
        return symbols.at(r * input.k + c);
    }
}

Output computeMSA(Input& input, bool tightConstraints, int method)
{
    input.m = (int)input.rawInput.size();

    // Pre-process
    for (auto it : input.rawInput) {  
        input.n.push_back((int)it.size());
    } 

    // Compute unique chararcters
    for (auto it : input.rawInput) {
        for (char c : it) {
            input.uniqueChars.insert(c);
        }
    }
    input.base = (int)input.uniqueChars.size() + 1;

    Output output;

    if (method == 2) {
        encodeInput2(input);
        MSAMethod2(input, output);
    }
    else if (method == 3) {
        encodeInput3(input);
        MSAMethod3(input, output);
    }

    return output;
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






#pragma region Method2

void encodeInput2(Input& input)
{
    // Build the encoding and decoding maps
    input.encoding.insert(std::pair<char, int>('-', 0));
    input.decoding.insert(std::pair<int, char>(0, '-'));
    int code = 1;
    for (auto it : input.uniqueChars) {
        input.encoding.insert(std::pair<char, int>(it, code));
        input.decoding.insert(std::pair<int, char>(code, it));
        code++;
    }

    // Build an encoded input vector
    for (auto it : input.rawInput) {
        std::vector<int> row = {};
        for (int i = 0; i < it.size(); ++i) {
            const int encodedValue = i * input.base + input.encoding.at(it[i]);
            row.push_back(encodedValue);
        }
        input.encodedInput.push_back(row);
    }
}

void MSAMethod2(const Input& input, Output& output)
{
    z3::context c;
    z3::set_param("parallel.enable", true);

    // Build a 2D array of symbols
    std::vector<z3::expr> symbols;
    for (int row = 0; row < input.m; ++row) {
        for (int col = 0; col < input.k; ++col) {
            char name[24] = {};
            snprintf(name, 24, "S_%d_%d", row, col);
            const z3::expr x = c.int_const(name);
            symbols.push_back(x);
        }
    }

    z3::solver s(c);

    // 1. Domain constraints (tight domain) (sometimes faster)
    for (int row = 0; row < input.m; ++row) {
        const int blanks = input.k - input.n[row];
        for (int col = 0; col < input.k; ++col) {
            const z3::expr& thisSymbol = GetSymbol(symbols, input, row, col);
            z3::expr thisDomain = (thisSymbol != thisSymbol); // Something false

            for (int inputCol = std::max(col - blanks, 0); inputCol <= std::min(input.n[row] - 1, col); ++inputCol) {
                const int inputVal = input.encodedInput.at(row).at(inputCol);

                thisDomain = (thisDomain || thisSymbol == inputVal);
            }

            for (int blankCol = std::max(col - blanks + 1, 0); blankCol <= col; ++blankCol) {
                const int blankVal = blankCol * input.base;
                thisDomain = (thisDomain || thisSymbol == blankVal);
            }

            s.add(thisDomain);
        }
    }

    // Loose domain (sometimes slower)
    /*for (z3::expr& it : symbols) {
        s.add(it >= 0 && it <= input.base * input.k);
    }*/

    // 2. Alignment constraints
    for (int col = 0; col < input.k; ++col) {
        for (int row = 0; row < input.m; ++row) {
            const z3::expr& thisSymbol = GetSymbol(symbols, input, row, col);

            z3::expr thisSymbolsConstraints = (thisSymbol == thisSymbol); // Something true

            for (int row2 = 0; row2 < input.m; ++row2) {
                if (row == row2) {
                    continue;
                }
                else {
                    const z3::expr& otherSymbol = GetSymbol(symbols, input, row2, col);
                    const z3::expr conjecture = ((thisSymbol % input.base) == (otherSymbol % input.base) || (otherSymbol % input.base) == 0);
                    thisSymbolsConstraints = (thisSymbolsConstraints && conjecture);
                }
            }
            thisSymbolsConstraints = (thisSymbolsConstraints || thisSymbol % input.base == 0);
            s.add(thisSymbolsConstraints);
        }
    }

    // 3. Increasing
    for (int row = 0; row < input.m; ++row) {
        for (int col = 1; col < input.k; ++col) {
            const z3::expr& thisSymbol = GetSymbol(symbols, input, row, col);
            const z3::expr& lastSymbol = GetSymbol(symbols, input, row, col - 1);

            // Increasing not required for 2 blanks in a row
            // This constraint makes it much slower, but is necessary :(
            const z3::expr multiBlanks = (thisSymbol % input.base == 0 && thisSymbol == lastSymbol);
            s.add(thisSymbol > lastSymbol || multiBlanks);
        }
    }

    // 4. One of each input per row
    for (int row = 0; row < input.m; ++row) {
        for (int it : input.encodedInput[row])
        {
            z3::expr thisInputCharsConstraints = (GetSymbol(symbols, input, row, 0) < 0); // Something false

            // More constraints (sometimes faster)
            for (int col = 0; col < input.k; ++col) {
                const z3::expr& thisSymbol = GetSymbol(symbols, input, row, col);

                z3::expr thisPermuationsConstraints = (thisSymbol == it);

                for (int col2 = 0; col2 < input.k; ++col2) {
                    if (col == col2) {
                        continue;
                    }

                    const z3::expr& otherSymbol = GetSymbol(symbols, input, row, col2);

                    thisPermuationsConstraints = (thisPermuationsConstraints && otherSymbol != it);
                }

                thisInputCharsConstraints = (thisInputCharsConstraints || thisPermuationsConstraints);
            }

            // Less constraints (sometimes faster)
            /*for (int col = 0; col < input.k; ++col) {
                const z3::expr& thisSymbol = GetSymbol(symbols, input, row, col);

                z3::expr thisPermuationsConstraints = (thisSymbol == it);
                thisInputCharsConstraints = (thisInputCharsConstraints || thisPermuationsConstraints);
            }*/

            s.add(thisInputCharsConstraints);
        }
    }


    // 5. Postprocessing and results
    const z3::check_result result = s.check();
    if (result == z3::sat)
    {
        output.isSAT = true;

        // Retrieve the output characters
        z3::model m = s.get_model();

        for (int row = 0; row < input.m; ++row) {
            std::vector<int> encodedOutputRow = {};
            std::vector<char> decodedOutputRow = {};

            for (int col = 0; col < input.k; ++col) {
                char name[24] = {};
                snprintf(name, 24, "S_%d_%d", row, col);

                for (uint32_t i = 0; i < m.size(); ++i) {
                    z3::func_decl v = m[i];
                    if (v.name().str().compare(name) == 0) {

                        // Get the raw output
                        int val = m.get_const_interp(v).get_numeral_int();
                        encodedOutputRow.push_back(val);

                        // (Post-processing) Check if this char even exists in the input, if not, replace with a "-"
                        const std::vector<int>& inputRow = input.encodedInput[row];
                        if (std::find(inputRow.begin(), inputRow.end(), val) == inputRow.end()) {
                            val = 0;
                        }

                        // Decode the output value
                        const int outputVal = val % input.base;
                        const char outputChar = input.decoding.at(outputVal);
                        decodedOutputRow.push_back(outputChar);
                    }
                }
            }

            // Push this row onto the output
            output.encodedOutput.push_back(encodedOutputRow);
            output.decodedOutput.push_back(decodedOutputRow);
        }
    }
    else
    {
        // No solution :(
        output.isSAT = false;
    }
}

#pragma endregion Kyles Optimized method





#pragma region Method3

void encodeInput3(Input& input)
{
    // Build the encoding and decoding maps
    // Map each unique character to a unique value
    input.encoding.insert(std::pair<char, int>('-', 0));
    input.decoding.insert(std::pair<int, char>(0, '-'));
    int code = 1;
    for (auto it : input.uniqueChars) {
        input.encoding.insert(std::pair<char, int>(it, code));
        input.decoding.insert(std::pair<int, char>(code, it));
        code++;
    }

    // Build an encoded input vector
    for (auto it : input.rawInput) {
        std::vector<int> row = {};
        for (int i = 0; i < it.size(); ++i) {
            const int encodedValue = input.encoding.at(it[i]);
            row.push_back(encodedValue);
        }
        input.encodedInput.push_back(row);
    }
}

struct Edge {
    Edge(const z3::expr& sym) : symbol(sym) 
    { }

    z3::expr symbol;
    std::pair<int, int> start;
    std::pair<int, int> end;
    int startChar = 0;
    int endChar = 0;
};

void MSAMethod3(const Input& input, Output& output)
{
    z3::context c;
    z3::set_param("parallel.enable", true);

    // Define symbols
    std::vector<Edge> edges;
    for (int row = 0; row < input.m; ++row) {
        for (int col = 0; col < input.n[row]; ++col) {
            // For each input symbol
            const int thisSymbol = input.encodedInput[row][col];
            
            // Loop through all symbols in all later rows
            for (int row2 = row + 1; row2 < input.m; ++row2) {
                for (int col2 = 0; col2 < input.n[row2]; ++col2) {
                    // Create an edge if the symbols match
                    const int otherSymbol = input.encodedInput[row2][col2];

                    if (thisSymbol == otherSymbol) {
                        char name[32] = {};
                        snprintf(name, 32, "S_%d_%d__%d_%d", row, col, row2, col2);

                        Edge e(c.bool_const(name));
                        e.start = std::make_pair(row, col);
                        e.end = std::make_pair(row2, col2);
                        e.startChar = thisSymbol;
                        e.endChar = otherSymbol;

                        edges.push_back(e);
                    }
                }
            }
        }
    }

    // Declare solver
    z3::solver s(c);

    // Define constraints

    // 0. No critical cycles



    // 1. Pick only 1 edge from each char to each row

    // 2. Edges cannot cross

    // 3. Objective
    
    // Run the solver
    const z3::check_result result = s.check();
    if (result == z3::sat)
    {
        // There is a solution
        output.isSAT = true;

        // View the model
        z3::model m = s.get_model();

       // Enumerate and print the variables
        for (uint32_t i = 0; i < m.size(); ++i) {
            z3::func_decl v = m[i];
            std::cout << "\n" << v.name() << ": " << m.get_const_interp(v);
        }
        std::cout << "\n";

        // TODO: Decode the variables and add them to the output
    }
    else
    {
        // No solution :(
        output.isSAT = false;
    }
}

#pragma endregion Linear Programming method






void printInput(const Input& input)
{
    printf("Input:\n");
    for (auto it : input.rawInput) {
        for (auto c : it) {
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