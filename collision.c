#include "collision.h"
#include "blocks.h"
#include "player.h"
#include "sprite.h"
#include "scroll.h"
#include "level.h"

#define TO_TILE(fixed) (fix16_int32_to(fixed) >> 4)

#define FLAG_IS_SET(var, flags) (((var) & (flags)) != MAP_VALUE_FLAG_NONE)

extern loaded_level_t *loaded_level;

bool
collision_point_check(fix16_t x, fix16_t y)
{
        const map_value_flags_t flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD, TO_TILE(x), TO_TILE(y));

        return FLAG_IS_SET(flags, MAP_VALUE_FLAG_COLLISION);
}

trigger_id_t
collision_trigger_get(const sprite_t *sprite)
{
        const map_value_flags_t flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos),
                TO_TILE(sprite->y_pos));

        if (!FLAG_IS_SET(flags, MAP_VALUE_FLAG_TRIGGER)) {
                return TRIGGER_ID_INVALID;
        }

        return scroll_trigger_index_get(SCROLL_BG_PLAYFIELD,
            TO_TILE(sprite->x_pos),
            TO_TILE(sprite->y_pos));
}

bool
collision_up_check(const sprite_t *sprite)
{
        // Top: top left or top right pixel hits something
        const map_value_flags_t tr_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos),
                TO_TILE(sprite->y_pos));

        const map_value_flags_t tl_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_width - FIX16(1)),
                TO_TILE(sprite->y_pos));

        return (FLAG_IS_SET(tr_flags, MAP_VALUE_FLAG_COLLISION) ||
                FLAG_IS_SET(tl_flags, MAP_VALUE_FLAG_COLLISION));
}

bool
collision_down_check(const sprite_t *sprite)
{
        // Bottom: bottom left or bottom right pixel hits something
        const map_value_flags_t bl_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x),
                TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height - FIX16(1)));
        const map_value_flags_t br_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x + sprite->bb_width - FIX16(1)),
                TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height - FIX16(1)));

        if (FLAG_IS_SET(bl_flags, MAP_VALUE_FLAG_COLLISION) ||
            FLAG_IS_SET(br_flags, MAP_VALUE_FLAG_COLLISION)) {
                return 1;
        }

        return 0;
}

bool
collision_below_offset_check(const sprite_t *sprite, fix16_t offset_x,
    fix16_t offset_y)
{
        const map_value_flags_t bl_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x + offset_x),
                TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height + offset_y + FIX16(1)));
        const map_value_flags_t br_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x + sprite->bb_width + offset_x - FIX16(1)),
                TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height + offset_y + FIX16(1)));

        return (FLAG_IS_SET(bl_flags, MAP_VALUE_FLAG_COLLISION) ||
            FLAG_IS_SET(br_flags, MAP_VALUE_FLAG_COLLISION));
}

bool
collision_below_check(const sprite_t *sprite)
{
        return collision_below_offset_check(sprite, FIX16(0.0f), FIX16(0.0f));
}

bool
collision_left_check(const sprite_t *sprite)
{
        // Left: top left + 8px and bottom left - 8px hits something
        const map_value_flags_t tl_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x),
                TO_TILE(sprite->y_pos + sprite->bb_y + FIX16(4.0f)));

        map_value_flags_t bl_flags;
        bl_flags = MAP_VALUE_FLAG_NONE;

        // If we're on a slope, don't check bottom left to avoid colliding with
        // the solid wall next to it
        if ((sprite->options & OPTION_SLOPE) == OPTION_SLOPE) {
                /* bottom_left = 0; */
        } else {
                bl_flags = scroll_flags_get(SCROLL_BG_PLAYFIELD,
                    TO_TILE(sprite->x_pos + sprite->bb_x),
                    TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height - FIX16(1)) - FIX16(8.0f));
        }

        if (FLAG_IS_SET(tl_flags, MAP_VALUE_FLAG_COLLISION) ||
            FLAG_IS_SET(bl_flags, MAP_VALUE_FLAG_COLLISION)) {
                return 1;
        }

        return 0;
}

bool
collision_right_check(const sprite_t *sprite)
{
        // Right: top right + 8px and bottom right - 8px hits something
        const map_value_flags_t tr_flags =
            scroll_flags_get(SCROLL_BG_PLAYFIELD,
                TO_TILE(sprite->x_pos + sprite->bb_x + sprite->bb_width - FIX16(1)),
                TO_TILE(sprite->y_pos + sprite->bb_y + FIX16(4)));

        map_value_flags_t br_flags;
        br_flags = MAP_VALUE_FLAG_NONE;

        /* br_flags = 0; */

        if ((sprite->options & OPTION_SLOPE) == OPTION_SLOPE) {
                /* br_flags = 0; */
        } else {
                br_flags = scroll_flags_get(SCROLL_BG_PLAYFIELD,
                    TO_TILE(sprite->x_pos + sprite->bb_x + sprite->bb_width - FIX16(1)),
                    TO_TILE(sprite->y_pos + sprite->bb_y + sprite->bb_height - FIX16(1)) - FIX16(8.0f));
        }

        if (FLAG_IS_SET(tr_flags, MAP_VALUE_FLAG_COLLISION) ||
            FLAG_IS_SET(br_flags, MAP_VALUE_FLAG_COLLISION)) {
                return 1;
        }

        return 0;
}

void
collision_mask_update(sprite_t *sprite)
{
        uint16_t collision = 0;

        if (collision_up_check(sprite)) {
                collision |= COLLISION_UP;
        }

        if (collision_down_check(sprite)) {
                collision |= COLLISION_DOWN;
        }

        if (collision_left_check(sprite)) {
                collision |= COLLISION_LEFT;
        }

        if (collision_right_check(sprite)) {
                collision |= COLLISION_RIGHT;
        }

        sprite->collision = collision;
}

void
collision_vert_eject(sprite_t *sprite)
{
        // Reset slope bit
        sprite->options &= ~OPTION_SLOPE;

        if (sprite->dy < 0) {
                while (collision_up_check(sprite)) {
                        sprite->dy = 0;
                        sprite->y_pos += FIX16(1);
                }
        }

        if (sprite->dy > 0) {
                while (collision_down_check(sprite)) {
                        sprite->dy = 0;
                        sprite->y_pos -= FIX16(1);
                }

                /* const map_value_flags_t bottom_flags = */
                /*     scroll_flags_get(SCROLL_BG_PLAYFIELD, */
                /*         TO_TILE(sprite->x_pos + (sprite->x_size >> 1)), */
                /*         TO_TILE(sprite->y_pos + sprite->y_size - FIX16(1))); */

                /* if (block_check(bottom)) { */
                /*         // If we're on a tile boundary, don't need to do anything to the position. */

                /*         // If we're not (have been moved vertically by the slope */
                /*         // tile), set the position to the bottom of the tile */
                /*         // before modifying it with the heightmap */
                /*         if (sprite->y_pos & 0x000F0000) { */
                /*                 sprite->y_pos &= 0xFFF00000; */
                /*                 sprite->y_pos += FIX16(16); */
                /*         } */

                /*         int32_t block_index = fix16_int32_to(sprite->x_pos + (sprite->x_size >> 1)) & 0xF; */

                /*         sprite->y_pos -= fix16_int32_from(block_get(bottom, block_index)); */

                /*         sprite->options |= OPTION_SLOPE; */
                /* } */
        }
}

void
collision_horiz_eject(sprite_t *sprite)
{
        if (sprite->dx < 0) {
                while (collision_left_check(sprite)) {
                        sprite->dx = 0;
                        sprite->x_pos &= 0xFFFF0000;
                        sprite->x_pos += FIX16(1);
                }
        }

        if (sprite->dx > 0) {
                while (collision_right_check(sprite)) {
                        sprite->dx = 0;
                        sprite->x_pos &= 0xFFFF0000;
                        sprite->x_pos -= FIX16(1);
                }
        }
}

bool
collision_player(const sprite_t *sprite)
{
        return ((loaded_level->player_pos.x <= sprite->x_pos + sprite->bb_width) &&
                (loaded_level->player_pos.x + loaded_level->player_width >= sprite->x_pos) &&
                (loaded_level->player_pos.y <= sprite->y_pos + sprite->bb_height) &&
                (loaded_level->player_pos.y + loaded_level->player_height >= sprite->y_pos));
}
