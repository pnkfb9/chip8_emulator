#include <stdio.h>
#include "chip8_core.h"
#include "graphics.h"
int main() {
    printf("---- CHIP8 INTERPRETER ----\n");
    printf("Written by Fabio Pancot, Coronavirus quarantine member group, March 2020.\n\n\n\r");
    graphics_init();
    chip8_init();

    while(1)
    {
        chip8_emulate_cycle();
        //graphics_draw();
        if(chip8_is_draw_flag_set())
        {
            graphics_draw();
            chip8_draw_flag_reset();
        }

    }

    return 0;
}
