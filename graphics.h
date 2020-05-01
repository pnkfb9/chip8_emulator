/*
* \file graphics.h
* TODO: aggiungere commento
* \author Fabio Pancot
* \date 14/03/20.
*
*/

#ifndef CHIP_8_GRAPHICS_H
#define CHIP_8_GRAPHICS_H
#include <SDL2/SDL.h>


#define SCREEN_WIDTH    64U         ///< Chip-8 original screen width size
#define SCREEN_HEIGHT   32U         ///< Chip-8 original screen height size

/*
 * Pixel's width and height. Because of the original screen size was too small to modern PCs,
 * I decided to scale the original size by a factor of 10, so each pixel in my SDL surface will be
 * 10 times a \"normal\" pixel. In this way I can render a 640px x 320px screen and have a better user experience.
*/
#define PIXEL_W         10U
#define PIXEL_H         10U

uint8_t gfx[SCREEN_HEIGHT][SCREEN_WIDTH];

/*
 * \brief The initialization function.
 * \return 0 if initialization OK, 1 otherwise.
 */
int graphics_init(void);

/*
 * \brief This functions clears the pixels (set all to black).
 */
void graphics_clear(void);

void graphics_draw(void);
#endif //CHIP_8_GRAPHICS_H
