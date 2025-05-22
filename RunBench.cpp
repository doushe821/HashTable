#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char PerfData[4][20] = 
    {
        {"Native"},
        {"oFirst"},
        {"Second"},
        {"oThird"},
    };

    char PerfCallString[50] = {"./Hash --opt "};
    for(int i = 0; i < 4; i++)
    {
        PerfCallString[strlen(PerfCallString)] = i + '0';
        for(int j = 0; j < 10; j++)
        {
            system(PerfCallString);
        }
    }
}