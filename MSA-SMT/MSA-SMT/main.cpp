#include "MSA.h"

int main()
{
    Input input;
    Output output;

    getInput(input);
    printInput(input);
    MSAMethod2(input, output);
    printOutput(output);

    return 0;
}
