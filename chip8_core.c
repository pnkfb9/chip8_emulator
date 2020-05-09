//
// Created by panc on 14/03/20.
//

#include "graphics.h"
#include "chip8_core.h"
#include <string.h>
#include <bits/types/FILE.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
 *  system memory map:
 *
 *  0x000-0x1FF -> interpreter
 *  0x04F-0x0A0 -> built-in font set (0-F)
 *  0x1FF-0xFFF -> program ROM and work RAM
 */
#define FONTSET_START_ADDR  (0x050U)
#define FONTSET_END_ADDR    (0x0A0U)
#define FONT_SIZE           (5U)
#define FONTSET_SIZE    (FONTSET_END_ADDR - FONTSET_START_ADDR)
#define ROM_OFFSET          (0x200U)

static uint8_t  draw_flag = 0;
static uint16_t opcode;
static uint8_t memory[4096];
static uint8_t V[16];                                                   ///< 16 registers, where last register carry the flag

static uint16_t I;                                                      ///< Index register
static uint16_t pc;                                                     ///< Program counter

static uint16_t stack[16];                                              ///< 16 levels os stack max
static uint16_t sp;                                                     ///< Stack Pointer

static uint8_t delay_timer;                                             ///< Counter used to emulate the delay timer
static uint8_t sound_timer;                                             ///< Counter used to emulate the sound timer
static time_t t;

static uint8_t keys[16] = {0};                                          ///< byte array to store the different keys state(pressed or released)
static int cycle = 0;

static uint8_t chip8_fontset[80] =
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };


typedef enum
{
    CLS = 0,
    RET,
    SYS_ADDR,
    JP_ADDR,
    CALL_ADDR,
    SE_VX_BYTE,
    SNE_VX_BYTE,
    SE_VX_VY,
    LD_VX_BYTE,
    ADD_VX_BYTE,
    LD_VX_VY,
    OR_VX_VY,
    AND_VX_VY,
    XOR_VX_VY,
    ADD_VX_VY,
    SUB_VX_VY,
    SHR_VX__VY_,              //denote Vx {, Vy }
    SUBN_VX_VY,
    SHL_VX__VY_,              //denote Vx {, Vy }
    SNE_VX_VY,
    LD_I_ADDR,
    JP_V0_ADDR,
    RND_VX_BYTE,
    DRW_VX_VY_NIBBLE,
    SKP_VX,
    SKNP_VX,
    LD_VX_DT,
    LD_VX_K,
    LD_DT_VX,
    LD_ST_VX,
    ADD_I_VX,
    LD_F_VX,
    LD_B_VX,
    LD_I_VX,
    LD_VX_I
}CHIP8_opcodes_t;


static uint16_t instr_set[35] = {
        0x00E0U,    // CLS
        0x00EEU,    // RET
        0x0000U,    // SYS Addr
        0x1000U,    // JP addr
        0x2000U,    // CALL addr
        0x3000U,    // SE Vx, byte
        0x4000U,    // SNE Vx, byte
        0x5000U,    // SE Vx, Vy
        0x6000U,    // LD Vx, byte
        0x7000U,    // ADD Vx, byte
        0x8000U,    // LD Vx, Vy
        0x8001U,    // OR Vx, Vy
        0x8002U,    // AND Vx, Vy
        0x8003U,    // XOR Vx, Vy
        0x8004U,    // ADD Vx, Vy
        0x8005U,    // SUB Vx, Vy
        0x8006U,    // SHR Vx { , Vy }
        0x8007U,    // SUBN Vx, Vy
        0x800EU,    // SHL Vx { , Vy }
        0x9000U,    // SNE Vx, Vy
        0xA000U,    // LD I, addr
        0xB000U,    // JP V0, addr
        0xC000U,    // RND Vx, byte
        0xD000U,    // DRW Vx, Vy, nibble
        0xE09EU,    // SKP Vx
        0xE0A1U,    // SKNP Vx
        0xF007U,    // LD Vx, DT
        0xF00AU,    // LD Vx, K
        0xF015U,    // LD DT, Vx
        0xF018U,    // LD ST, Vx
        0xF01EU,    // ADD I, Vx
        0xF029U,    // LD F, Vx
        0xF033U,    // LD B, Vx
        0xF055U,    // LD[I], Vx
        0xF065U     // LD Vx, [I]

};

static void chip8_decode(const uint16_t *opc);
static void chip8_handle_keyboard(void);
static void chip8_print_status(void);

void chip8_init(void)
{
    srand(time(NULL));
    pc = 0x200U;
    opcode = 0x0U;
    I = 0x00U;
    sp = 0x00U;

    //clear regs, stack and memory
    for(uint8_t i = 0; i < 16;i++)
    {
        stack[i] = 0x00U;
        V[i] = 0x0U;
    }
    memset(&memory, 0x0U,sizeof(memory));

    for(uint8_t i = 0;i<FONTSET_SIZE;i++)
    {
         memory[i + FONTSET_START_ADDR] = chip8_fontset[i];
    }
    chip8_load_rom();
}

int
chip8_load_rom(void)
{
    FILE *file;
    long lSize;
    int retval = -1;
    file = fopen("./roms/INVADERS","rb");
endif //UNTITLED_GRAPHICS_H
    if(file)
    {
        fseek(file,0,SEEK_END);
        lSize = ftell(file);
        rewind(file);
        uint8_t byte = 0x0;
        size_t  res = 0;
        for(int pos = 0;pos<lSize;pos++)
        {
            res = fread(&byte,1,1,file);
            //printf(" %X at %X\n",byte,pos+ROM_OFFSET);
            if(res)memory[pos+ROM_OFFSET] = byte;
        }
        retval = 0;
    }

    //for(int i =0;i<lSize;i++)printf("memory[%X] = %X\n",ROM_OFFSET+i,memory[ROM_OFFSET+i]);
    return retval;
}
void
chip8_emulate_cycle(void)
{
    //fetch
    opcode = (memory[pc] << 8U) | memory[pc + 1];

    //decode
    chip8_decode(&opcode);

    //execute
    //chip8_handle_keyboard();
    //update timers
    //chip8_print_status();
    if(delay_timer)delay_timer--;
    if(sound_timer)sound_timer--;
    cycle++;
}

void
chip8_decode(const uint16_t *opc)
{
    if(opc) {


        switch ((*opc & 0xF000U))
        {
            case 0x0000U:
            {
                switch((*opc & 0x000FU))
                {

                    case 0x0000U: //CLS
                    {
                        graphics_clear();
                        draw_flag = 1;
                        pc+=2;
                    }
                    break;
                    case 0x000EU:   //RET
                    {
                        sp--;
                        pc = stack[sp];
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
            case 0x1000U:
            {
                pc = (*opc & 0x0FFFU);
            }
            break;
            case 0x2000U:
            {
                stack[sp] = pc;
                ++sp;
                pc = (*opc & 0x0FFFU);
            }
            break;
            case 0x3000U:
            {
                if(V[((*opc & 0x0F00U) >> 8U)] == (*opc & 0x00FFU))
                {
                    pc+=4;
                }
                else
                {
                    pc+=2;
                }
            }
            break;
            case 0x4000U:
            {
               if(V[((*opc & 0x0F00U) >> 8U)] != (*opc & 0x00FFU))
               {
                   pc+=4;
               }
               else
               {
                   pc+=2;
               }
            }
            break;
            case 0x5000U:
            {
                if(V[((*opc & 0x0F00U)>>8U)] == V[((*opc & 0x00F0U) >> 4U)])
                {
                    pc+=4;
                }
                else
                {
                    pc+=2;
                }
            }
            break;
            case 0x6000U:
            {
                V[((*opc & 0x0F00U)>>8U)] = (*opc & 0x00FFU);
                pc+=2;
            }
            break;
            case 0x7000U:
            {
                V[((*opc & 0x0F00U)>>8U)] = V[((*opc & 0x0F00U)>>8U)] + (*opc & 0x00FFU);
                pc+=2;
            }
            break;
            case 0x8000U: {
                switch ((*opc & 0x000FU)) {
                    case 0x0000U: {
                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0001U: {
                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] | V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0002U: {
                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] & V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0003U: {
                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] ^ V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0004U: {
                        //uint8_t res = 0;
                        //V[((*opc & 0x0F00U)>>8U)]

                        if (V[((*opc & 0x00F0U) >> 4U)] > (0xFFU - V[((*opc & 0x0F00U) >> 8U)])) {
                            V[0xF] = 1U;
                        } else {
                            V[0xF] = 0U;
                        }

                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] + V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0005U:   //SUB Vx,Vy
                    {
                        if (V[((*opc & 0x00F0U) >> 4U)] < (V[((*opc & 0x0F00U) >> 8U)])) {
                            V[0xF] = 1U;
                        } else {
                            V[0xF] = 0U;
                        }

                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] - V[((*opc & 0x00F0U) >> 4U)];
                        pc += 2;
                    }
                        break;
                    case 0x0006U: //Vx SHR 1
                    {
                        if (0x01U & (V[((*opc & 0x0F00U) >> 8U)])) {
                            V[0xF] = 1U;
                        } else {
                            V[0xF] = 0U;
                        }

                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] >> 1U;
                        pc += 2;
                    }
                        break;
                    case 0x0007U: {
                        if (V[((*opc & 0x00F0U) >> 4U)] > (V[((*opc & 0x0F00U) >> 8U)])) {
                            V[0xF] = 1U;
                        } else {
                            V[0xF] = 0U;
                        }

                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x00F0U) >> 4U)] - V[((*opc & 0x0F00U) >> 8U)];
                        pc += 2;
                    }
                        break;
                    case 0x000EU: {
                        if (0x80U & (V[((*opc & 0x0F00U) >> 8U)])) {
                            V[0xF] = 1U;
                        } else {
                            V[0xF] = 0U;
                        }

                        V[((*opc & 0x0F00U) >> 8U)] = V[((*opc & 0x0F00U) >> 8U)] << 1U;
                        pc += 2;
                    }
                        break;
                    default:
                        break;
                }

            }
            break;
            case 0x9000U:
            {
                if(V[((*opc & 0x0F00U)>>8U)] != V[((*opc & 0x00F0U)>>4U)])
                {
                    pc+=2;
                }
                pc+=2;
            }
            break;
            case 0xA000U:
            {
                I =(*opc & 0x0FFFU);
                pc+=2;
            }
            break;
            case 0xB000U:
            {
                pc = V[0x0] + (*opc & 0x0FFFU);
            }
            break;
            case 0xC000U:
            {
                //srand((unsigned)time(&t));
                uint8_t random = rand() % 256;
                V[((*opc & 0x0F00U)>>8U)] = random & (*opc & 0x00FFU);
                pc+=2;
            }
            break;
            case 0xD000U:
            {
                ///< draw sprite
                uint8_t x,y,lines, line,x_set,y_set;
                x = V[(*opc & 0x0F00U) >> 8U];
                y = V[(*opc & 0x00F0U) >> 4U];
                lines = (*opc & 0x000FU);

                for(int y_l = 0; y_l < lines; y_l++) //rows
                {   line = memory[I + y_l];
                    for(int x_l = 0; x_l < 8U;x_l++) // columns
                    {
                        if((line & (0x80U >> x_l)) != 0)
                        {
                            x_set = x + x_l;
                            y_set = y + y_l;
                            if( x_set >(SCREEN_WIDTH - 1))
                            {
                                x_set -= SCREEN_WIDTH;
                            }
                            if( y_set > (SCREEN_HEIGHT - 1))
                            {
                                y_set -=SCREEN_HEIGHT;
                            }
                            if(gfx[y_set][x_set] == 1) V[0xF] = 1U;

                            gfx[y_set][x_set] ^= 1U;
                        }
                    }
                }
                pc+=2;
                draw_flag  = 1U;
            }
            break;
            case 0xE000U:
            {
                switch((*opc & 0x0FFU))
                {
                    case 0x009EU:
                    {
                        if(keys[V[(*opc & 0x0F00U)>>8U]] == 1U)
                        {
                            pc+=4;
                        } else
                        {
                            pc+=2;
                        }
                    }
                    break;
                    case 0x00A1U:
                    {
                        if(keys[V[(*opc & 0x0F00U)>>8U]] == 0U)
                        {
                            pc+=4;
                        } else
                        {
                            pc+=2;
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
            case 0xF000U:
            {
                switch((*opc & 0x0FFU))
                {
                    case 0x0007U:
                    {
                            V[((*opc & 0x0F00U)>>8U)] = delay_timer;
                            pc+=2;
                    }
                    break;
                    case 0x000AU:
                    {

                    }
                    break;
                    case 0x0015U:
                    {
                            delay_timer = V[((*opc & 0x0F00U)>>8U)];
                            pc+=2;
                    }
                    break;
                    case 0x0018U:
                    {
                        sound_timer = V[((*opc & 0x0F00U)>>8U)];
                        pc+=2;
                    }
                    break;
                    case 0x001EU:
                    {
                            I = I + V[((*opc &0x0F00U)>>8U)];
                            pc+=2;
                    }
                    break;
                    case 0x0029U:
                    {
                            uint16_t location = FONTSET_START_ADDR;
                            location += (FONT_SIZE * V[(*opc & 0x0F00U)>>8U]);
                            //printf("location: %X\n",location);
                            I = location;
                            pc+=2;
                    }
                    break;
                    case 0x0033U:
                    {
                            uint16_t value = V[(*opc & 0x0F00U)>>8U];
                            memory[I] = (value/100)%10;
                            memory[I + 1] = (value/10)%10;
                            memory[I + 2] = value%10;
                            pc+=2;
                    }
                    break;
                    case 0x0055U:
                    {
                        for(uint8_t i = 0;i <= ((*opc & 0x0F00U)>>8U);i++)
                        {
                            memory[I+i] = V[i];
                        }
                        pc+=2;
                    }
                    break;
                    case 0x0065U:
                    {
                        for(uint8_t i = 0;i <= ((*opc &0x0F00U)>>8U);i++)
                        {
                             V[i] = memory[I + i];
                        }
                        pc+=2;
                    }
                    break;
                }
            }
            break;

        }
    }

}

uint8_t chip8_is_draw_flag_set(void)
{
    return draw_flag;
}

void chip8_draw_flag_reset(void)
{
    draw_flag = 0;

}

void chip8_handle_keyboard(void)
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_3:
                {
                    keys[0] = 1U;
                }
                break;
                case SDLK_4:
                {
                    keys[1] = 1U;
                }
                break;
                case SDLK_5:
                {
                    keys[2] = 1U;
                }
                break;
                case SDLK_6:
                {
                    keys[3] = 1U;
                }
                break;
                case SDLK_e:
                {
                    keys[4] = 1U;
                }
                break;
                case SDLK_r:
                {
                    keys[5] = 1U;
                }
                break;
                case SDLK_t:
                {
                    keys[6] = 1U;
                }
                break;
                case SDLK_y:
                {
                    keys[7] = 1U;
                }
                break;
                case SDLK_f:
                {
                    keys[8] = 1U;
                }
                break;
                case SDLK_g:
                {
                    keys[9] = 1U;
                }
                break;
                case SDLK_h:
                {
                    keys[10] = 1U;
                }
                break;
                case SDLK_j:
                {
                    keys[11] = 1U;
                }
                break;
                case SDLK_v:
                {
                    keys[12] = 1U;
                }
                break;
                case SDLK_b:
                {
                    keys[13] = 1U;
                }
                break;
                case SDLK_n:
                {
                    keys[14] = 1U;
                }
                break;
                case SDLK_m:
                {
                    keys[15] = 1U;
                }
                break;
                default:
                    break;
            }
        }
        if(event.type == SDL_KEYUP)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_3:
                {
                    keys[0] = 0U;
                }
                    break;
                case SDLK_4:
                {
                    keys[1] = 0U;
                }
                    break;
                case SDLK_5:
                {
                    keys[2] = 0U;
                }
                    break;
                case SDLK_6:
                {
                    keys[3] = 0U;
                }
                    break;
                case SDLK_e:
                {
                    keys[4] = 0U;
                }
                    break;
                case SDLK_r:
                {
                    keys[5] = 0U;
                }
                    break;
                case SDLK_t:
                {
                    keys[6] = 0U;
                }
                    break;
                case SDLK_y:
                {
                    keys[7] = 0U;
                }
                    break;
                case SDLK_f:
                {
                    keys[8] = 0U;
                }
                    break;
                case SDLK_g:
                {
                    keys[9] = 0U;
                }
                    break;
                case SDLK_h:
                {
                    keys[10] = 0U;
                }
                    break;
                case SDLK_j:
                {
                    keys[11] = 0U;
                }
                    break;
                case SDLK_v:
                {
                    keys[12] = 0U;
                }
                    break;
                case SDLK_b:
                {
                    keys[13] = 0U;
                }
                    break;
                case SDLK_n:
                {
                    keys[14] = 0U;
                }
                    break;
                case SDLK_m:
                {
                    keys[15] = 0U;
                }
                    break;
                default:
                    break;
            }
        }
    }

}

void
chip8_print_status(void)
{
    printf("\n***\ncycle %d\n", cycle);
    printf("PC: %d\n",pc);
    printf("opcode: %X\n",opcode);

    /*for(int i = 0;i<16;i++)
    {
        printf("V[%d] = %X\n",i,V[i]);
    }*/
    printf("***\n");
}