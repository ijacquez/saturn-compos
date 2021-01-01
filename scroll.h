#ifndef SCROLL_H
#define SCROLL_H

#include <stdint.h>
#include <math.h>

#include "graphics/triggers.h"

#include "format_lvl.h"

#define TILE_SIZE       16
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   224
// Maximum # tiles shown on screen
#define SCREEN_TILES_X  28
#define SCREEN_TILES_Y  20
#define BG_TILES        32

extern int32_t map_tiles_x[];
extern int32_t map_tiles_y[];

#define COPY_MODE_LCOL 1
#define COPY_MODE_RCOL 2
#define COPY_MODE_TROW 4
#define COPY_MODE_BROW 8

#define SCROLL_HMIRROR (0x400)
#define SCROLL_VMIRROR (0x800)

extern uint32_t copy_modes[]; // What to copy to VRAM from the map
extern fix16_t scrolls_x[];
extern fix16_t scrolls_y[];

typedef enum scroll {
        SCROLL_BG_PLAYFIELD = 2,
        SCROLL_BG_OVERLAP   = 3,
        SCROLL_BG_FAR       = 0,
        SCROLL_BG_NEAR      = 1
} scroll_t;

void scroll_init(void);

void scroll_lvl_process(const lvl_t *lvl);

// Translate scroll by (x,y) amounts.
void scroll_move(scroll_t scroll, fix16_t x, fix16_t y);

// Moves scroll absolutely to coordinates.
void scroll_set(scroll_t scroll, fix16_t x, fix16_t y);

void scroll_backcolor_set(color_rgb1555_t color);

// Gets the value at the given coordinates for a square map.
map_value_flags_t scroll_flags_get(scroll_t scroll, int32_t x, int32_t y);
trigger_id_t scroll_trigger_index_get(scroll_t scroll, int32_t x, int32_t y);

// Copies scroll to VRAM after position has been changed by move/set scroll.
void scroll_update(scroll_t scroll);

// Sets all backgrounds to how they were on initialized.
void scroll_reset(void);

// Sets up a linescroll screen with 4 splits.
// boundaryN: where the screen splits are
void scroll_linescroll4(scroll_t scroll, fix16_t scroll_val, int boundary1, int boundary2, int boundary3);

void scroll_screen_move(uint32_t row);

#endif /* !SCROLL_H */
