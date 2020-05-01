//
// Created by panc on 14/03/20.
//

#include "graphics.h"
#include <stdint.h>
#include <string.h>

#define PIXEL_WHITE 0xFFFFU
#define PIXEL_BLACK 0x000FU


static SDL_Window *window = NULL;
static SDL_Surface *screenSurface = NULL;
static SDL_Rect pixel;

int
graphics_init(void)
{
    int res = 0;
    printf("Initializing graphics...\n\r");
    res |= SDL_Init(SDL_INIT_VIDEO);
    pixel.x = 0;
    pixel.y = 0;
    pixel.h = PIXEL_H;
    pixel.w = PIXEL_W;
    if(!res)
    {
       window = SDL_CreateWindow("CHIP-8",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH * 10,SCREEN_HEIGHT*10,SDL_WINDOW_SHOWN);
       if(window)
       {
            screenSurface = SDL_GetWindowSurface(window);
            SDL_FillRect(screenSurface,NULL,PIXEL_BLACK);
            SDL_UpdateWindowSurface(window);
            SDL_Delay(2000);
       } else
       {
           res = 1;
           printf("Cannot show window. SDL Error: %s\n",SDL_GetError());
       }
    }
    else
    {
        res = 1;
        printf("Cannot initialize SDL. SDL_Error: %s\n",SDL_GetError());
    }

    printf("Initializing graphics done.\n\r");

    return res;
}

/* \fn graphics_draw
 *
 * This function is used to draw the screenSurface based on the content of \ref gfx array, which contains the pixel map
 * updated by chip-8 instructions at runtime.
 * This function is called when the following opcodes are executed:
 *  1) 0xDXYN -> write starting from pixel at <x,y> N sprites
 *  2) 0x00E0 -> clear screen (set black)
 * \return o if
 */
int graphics_draw(void)
{
    uint32_t color = PIXEL_BLACK;
        for(int r = 0;r<SCREEN_WIDTH;r++)
        {
            for(int col = 0;col<SCREEN_HEIGHT;col++)
            {
                pixel.x = r * 10;
                pixel.y = col * 10;
                (gfx[r][col] == 1) ?  (color = PIXEL_WHITE) : (color = PIXEL_BLACK);
                SDL_FillRect(screenSurface,&pixel,color);
            }
        }
    SDL_UpdateWindowSurface(window);
}

void
graphics_clear(void)
{
    memset(gfx,0x0U,sizeof(gfx));
}
