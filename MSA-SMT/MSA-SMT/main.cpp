#include "MSA.h"

// Change this value to switch between methods
const int method = 3;

int main()
{
    Input input;
    Output output;

    getInput(input);

    if (method == 2) {
        encodeInput2(input);
    }
    else if (method == 3) {
        encodeInput3(input);
    }

    printInput(input);

    if (method == 2) {
        MSAMethod2(input, output);
    }
    else if (method == 3) {
        MSAMethod3(input, output);
    }
   

    printOutput(output);

    return 0;
}
