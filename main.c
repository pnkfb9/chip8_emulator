#include <stdio.h>
#include <time.h>
#include "chip8_core.h"
#include "graphics.h"


int main() {
    clock_t start, end;
    printf("---- CHIP8 INTERPRETER ----\n");
    printf("Written by Fabio Pancot, Coronavirus quarantine member group, March 2020.\n\n\n\r");
    graphics_init();
    chip8_init();
    start = clock();
    while(!chip8_terminate())
    {

        if((long double)(end - start)/((float)CLOCKS_PER_SEC/1000) >= 2.0f) {
            start = clock();

            chip8_emulate_cycle();
            if (chip8_is_draw_flag_set()) {
                graphics_draw();
                chip8_draw_flag_reset();
            }
        }
        end = clock();
       
    }

    return 0;
}
