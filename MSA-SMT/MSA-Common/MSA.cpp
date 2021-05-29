#include "MSA.h"
#include "z3++.h"

void encodeInput(Input& input)
{
    // Compute the number of rows
    input.m = (int)input.rawInput.size();

    // Compute the length of each row
    for (auto it : input.rawInput) {
        const int length = (int)it.size();
        input.n.push_back(length);
        input.blanks.push_back(input.k - length);
    }

    // Compute unique chararcters
    for (auto it : input.rawInput) {
        for (char c : it) {
            input.uniqueChars.insert(c);
        }
    }
}

void MSAMethod(const Input& input, Output& output)
{
    z3::context c;
    z3::set_param("parallel.enable", true);

    // Build one symbol for each input location
    std::vector<std::vector<z3::expr>> symbols;
    for (int row = 0; row < input.m; ++row) {
        std::vector<z3::expr> newRow;
        for (int col = 0; col < input.n[row]; ++col) {
            char name[24] = {};
            snprintf(name, 24, "B_%d_%d", row, col);
            const z3::expr x = c.int_const(name);
            newRow.push_back(x);
        }
        symbols.push_back(newRow);
    }

    z3::solver s(c);

    // 1. Domain constraints
    for (int row = 0; row < input.m; ++row) {
        for (z3::expr& it : symbols[row]) {
            s.add(it >= 0 && it <= input.blanks[row]);
        }
    }

    // 2. Increasing - Each symbol needs to be larger or equal to the previous
    for (int row = 0; row < input.m; ++row) {
        for (int col = 1; col < input.n[row]; ++col) {
            const z3::expr& thisSymbol = symbols[row][col];
            const z3::expr& lastSymbol = symbols[row][col - 1];
            s.add(thisSymbol >= lastSymbol);
        }
    }

    // 3. Alignment - Each symbol in each row needs to be matched with a symobl in each other row (or a blank)
    for (int row = 0; row < input.m; ++row) {
        for (int col = 0; col < input.n[row]; ++col) {
            const z3::expr& thisSymbol = symbols[row][col];
            const char thisSymbolInput = input.rawInput[row][col];

            // TODO: Find some way to eliminate these bogus constraints
            z3::expr thisSymbolsConstraints = (thisSymbol == thisSymbol); // Something true

            // In each row, this symbol needs to match at least 1 other 
            for (int row2 = 0; row2 < input.m; ++row2) {
                if (row == row2) {
                    continue;
                }

                // TODO: Find some way to eliminate these bogus constraints
                z3::expr matchingConstraints = (thisSymbol != thisSymbol); // Something false
                z3::expr noMatchConstraints = (thisSymbol == thisSymbol); // Something true

                for (int col2 = 0; col2 < input.n[row2]; ++col2) {
                    const z3::expr& otherSymbol = symbols[row2][col2];
                    const char otherSymbolInput = input.rawInput[row2][col2];

                    // Tight constraints - Check if these 2 columns are even close enough to be aligned,
                    // if not, continue.
                    if (input.tightConstraints) {
                        if (abs(col - col2) > abs(std::max(input.blanks[row], input.blanks[row2]))) {
                            continue;
                        }
                    }

                    // Either these symbols align (and match)
                    if (thisSymbolInput == otherSymbolInput) {

                        // Symbols are aligned by columns
                        z3::expr aligned = (thisSymbol + col == otherSymbol + col2);
                        matchingConstraints = (matchingConstraints || aligned);
                    }

                    // Or it doesn't match any (i.e. blank)
                    z3::expr noMatch = (thisSymbol + col != otherSymbol + col2);
                    noMatchConstraints = (noMatchConstraints && noMatch);
                }


                thisSymbolsConstraints = (thisSymbolsConstraints && (matchingConstraints || noMatchConstraints));

            }
            s.add(thisSymbolsConstraints);
        }
    }


    // 4. Postprocessing and results
    const z3::check_result result = s.check();
    if (result == z3::sat)
    {
        output.isSAT = true;

        // Retrieve the output characters
        z3::model m = s.get_model();

        for (int row = 0; row < input.m; ++row) {
            std::vector<int> encodedOutputRow = {};
            std::vector<char> decodedOutputRow = {};

            int prevBlanks = 0;

            for (int col = 0; col < input.k; ++col) {
                char name[24] = {};
                snprintf(name, 24, "B_%d_%d", row, col);

                for (uint32_t i = 0; i < m.size(); ++i) {
                    z3::func_decl v = m[i];
                    if (v.name().str().compare(name) == 0) {

                        // Get the raw output
                        const int blanks = m.get_const_interp(v).get_numeral_int();
                        encodedOutputRow.push_back(blanks);

                        // (Post-processing) Add blanks to the output
                        for (int x = 0; x < blanks - prevBlanks; ++x) {
                            decodedOutputRow.push_back('-');
                        }

                        // Add the symbol for this location
                        decodedOutputRow.push_back(input.rawInput[row][col]);

                        prevBlanks = blanks;
                    }
                }
            }

            // Fill the rest with blanks
            while (decodedOutputRow.size() < input.k) {
                decodedOutputRow.push_back('-');
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


Output computeMSA(Input& input)
{
    Output output;

    encodeInput(input);
    MSAMethod(input, output);

    return output;
}