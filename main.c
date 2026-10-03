#include "main.h"

int main ()
{
    stdio_init_all();
    if(!dac_init())
    {
        printf("Unable to initialize DAC hardware.\r\n");
        printf("Program Exit.\r\n");
        return 1;
    }


    cli_run();

    return 0;
}