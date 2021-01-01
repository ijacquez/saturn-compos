#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <math.h>

#include "graphics/objects.h"
#include "vdp1_hw.h"

#define SPRITE_MIRROR_HORZ      (1 << 4)
#define SPRITE_MIRROR_VERT      (1 << 5)

struct sprite;

typedef void (*sprite_iterate_fn)(struct sprite *);

/// If the sprite shouldn't be displayed
#define OPTION_NODISP   (1 << 0)
/// If we're on a slope
#define OPTION_SLOPE    (1 << 1)

#define SPRITE_GRAVITY          (FIX16(0.5f))
#define SPRITE_MAXFALLSPEED     (FIX16(14.0f))

typedef struct loaded_spr_sprite {
        vdp1_vram_t texture_base;
        vdp1_clut_t *clut_base;
        uint16_t width;
        uint16_t height;
} loaded_spr_sprite_t;

typedef struct loaded_spr {
        object_id_t object_id;
        loaded_spr_sprite_t *sprites;
        uint16_t sprite_count;
} loaded_spr_t;

typedef struct sprite {
        const loaded_spr_t *loaded_spr;
        uint16_t spr_index; // Selected sprite

        uint16_t index; // Where the sprite is in the sprites array

        uint16_t options;
        void *data;
        uint16_t collision;
        int16_t sort;

        fix16_t x_pos;
        fix16_t y_pos;
        fix16_t render_width;
        fix16_t render_height;

        fix16_t bb_x;
        fix16_t bb_y;
        fix16_t bb_width;
        fix16_t bb_height;

        fix16_t dx;
        fix16_t dy;
        fix16_t scale;
        fix16_t angle;

        uint16_t mirror;

        bool anim_playing;
        uint16_t anim_timer; // Timer for animations
        uint16_t anim_cursor; // Where we are in animation array

        sprite_iterate_fn iterate;
} sprite_t;

/* XXX: Move to a global config header */
#define SPRITE_LIST_COUNT (64)

/// Sets up initial sprite display
void sprite_init(void);

/// Inits the sprite pointer given
void sprite_make(fix16_t x, fix16_t y, sprite_t *sprite);
/// Automatically picks the simplest SBL function for drawing the sprite
/// depending on required features needs command to be opened before calling
void sprite_draw(sprite_t *sprite);

//draws all sprites in the sprite list
void sprite_list_draw_all(void);
//gets a pointer to the next sprite in the list
sprite_t *sprite_list_next(void);
//deletes the given sprite from the sprite list
void sprite_list_delete(sprite_t *sprite);
//deletes all sprites from the list
void sprite_list_delete_all(void);

//utility function that moves a sprite based on its state
//collision: 1 to do collision, 0 to not
//returns the tile the sprite is over
uint16_t sprite_move(sprite_t *sprite, int collision);

const loaded_spr_t *sprite_load(object_id_t object_id);
void sprite_load_reset_all(void);

#endif /* !SPRITE_H */
