#include <stdint.h>
#include <stdio.h>

#include "scroll.h"

#define TILE2BLOCK(tile) (((tile) & 0x01FF) >> 1)

#define SIZEOF_BLOCK_SLOPES (sizeof(_block_slopes) / sizeof(uint8_t *))

static uint8_t tile2_slope[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16};
static uint8_t tile3_slope[] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};
static uint8_t tile4_slope[] = {8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15};

/* static const uint8_t *_block_slopes[] = { */
/*         NULL, */
/*         NULL, */
/*         tile2_slope, */
/*         tile3_slope, */
/*         tile4_slope, */
/*         NULL, */
/*         NULL, */
/*         NULL, */
/*         NULL, */
/*         tile2_slope, */
/*         tile3_slope, */
/*         tile4_slope */
/* }; */

static const uint8_t *_block_slopes[] = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
};

uint8_t
block_get(int tile, int offset)
{
        return 0;

        /* const uint8_t *block_slopes_ptr = _block_slopes[TILE2BLOCK(tile)]; */

        /* if ((TILE2BLOCK(tile) >= SIZEOF_BLOCK_SLOPES) || (block_slopes_ptr == NULL)) { */
        /*         return 0; */
        /* } */

        /* if (tile & SCROLL_HMIRROR) { */
        /*         return block_slopes_ptr[15 - offset]; */
        /* } else { */
        /*         return block_slopes_ptr[offset]; */
        /* } */
}

inline int
block_check(int tile)
{
        /* if ((TILE2BLOCK(tile) < SIZEOF_BLOCK_SLOPES) && */
        /*     (_block_slopes[TILE2BLOCK(tile)] != NULL)) { */
        /*         return 1; */
        /* } */

        return 0;
}
