#include <stdio.h>
#include <stdlib.h>

int main()
{
    char PerfData[4][20] = 
    {
        {"Native"},
        {"oFirst"},
        {"Second"},
        {"oThird"},
    };

    char PerfCallString[50] = {};
    for(int i = 0; i < 4; i++)
    {
        sprintf(PerfCallString,  "perf record --output=%s%d.data ./Hash --opt %d", PerfData[i], 0, i);
        for(int j = 0; j < 10; j++)
        {
            PerfCallString[27] = '0' + j;
            system(PerfCallString);
        }
    }
}