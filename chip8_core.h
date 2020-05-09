//
// Created by panc on 14/03/20.
//

#ifndef UNTITLED_CHIP8_CORE_H
#define UNTITLED_CHIP8_CORE_H
#include <stdint.h>

void chip8_init(void);
int chip8_load_rom(void);
void chip8_emulate_cycle(void);
uint8_t chip8_is_draw_flag_set(void);
void chip8_draw_flag_reset(void);
uint8_t chip8_terminate(void);
#endif //UNTITLED_CHIP8_CORE_H
