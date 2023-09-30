#ifndef COLLISION_H
#define COLLISION_H

#include <stdint.h>

#include "scroll.h"
#include "sprite.h"
#include "graphics/triggers.h"

#define COLLISION_UP    (1 << 0)
#define COLLISION_DOWN  (1 << 1)
#define COLLISION_LEFT  (1 << 2)
#define COLLISION_RIGHT (1 << 3)

void collision_mask_update(sprite_t *sprite);
// Returns 1 if the given point is a wall tile
bool collision_point_check(fix16_t x, fix16_t y);
trigger_id_t collision_trigger_get(const sprite_t *sprite);

bool collision_up_check(const sprite_t *sprite);
bool collision_down_check(const sprite_t *sprite);
bool collision_below_offset_check(const sprite_t *sprite, fix16_t offset_x,
    fix16_t offset_y);
bool collision_below_check(const sprite_t *sprite);
bool collision_left_check(const sprite_t *sprite);
bool collision_right_check(const sprite_t *sprite);

// Ejects the sprite_t vertically
void collision_vert_eject(sprite_t *sprite);
// Ejects the sprite_t horizontally
void collision_horiz_eject(sprite_t *sprite);

// Returns 1 if the sprite_t is touching a player, zero otherwise
bool collision_player(const sprite_t *sprite);

#endif /* !COLLISION_H */
