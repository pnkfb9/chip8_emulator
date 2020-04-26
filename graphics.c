//
// Created by panc on 14/03/20.
//

#include "graphics.h"
#include <stdint.h>
#include <string.h>
static uint8_t gfx[SCREEN_WIDTH * SCREEN_HEIGHT];
SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;

int
graphics_init(void)
{
    int res = 0;
    printf("Initializing graphics...\n\r");
    memset(gfx,0x0U,sizeof(gfx));
    res |= SDL_Init(SDL_INIT_VIDEO);
    if(!res)
    {
       window = SDL_CreateWindow("CHIP-8",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH * 10,SCREEN_HEIGHT*10,SDL_WINDOW_SHOWN);
       if(window)
       {
            screenSurface = SDL_GetWindowSurface(window);
            SDL_FillRect(screenSurface,NULL,SDL_MapRGB(screenSurface->format,0x00,0x00,0x00));
            SDL_UpdateWindowSurface(window);
            SDL_Delay(2000);
       } else
       {
           printf("Cannot show window. SDL Error: %s\n",SDL_GetError());
       }
    } else{
        printf("Cannot initialize SDL. SDL_Error: %s\n",SDL_GetError());
    }
    printf("Initializing graphics done.\n\r");
}