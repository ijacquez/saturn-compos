#include <yaul.h>

#include <stdint.h>

#include "missile.h"
#include "explosion.h"

#include "../collision.h"
#include "../graphics_refs.h"
#include "../sprite.h"
#include "../player.h"
#include "../level.h"

/* Max # of degrees missile can rotate in a frame */
#define MISSILE_ROTATE_SPEED    (FIX16(3))
#define MISSILE_SPEED           (FIX16(2))

extern loaded_level_t *loaded_level;

static const loaded_spr_t *_loaded_spr;

void
missile_load(void)
{
        _loaded_spr = sprite_load(OBJECT_ID_MISSILE);
        assert(_loaded_spr != NULL);
}

sprite_t *
missile_make(fix16_t x, fix16_t y)
{
        sprite_t *missile = sprite_list_next();
        sprite_make(x, y, missile);
        missile->loaded_spr = _loaded_spr;
        missile->iterate = &missile_move;
        missile->render_width = fix16_int32_from(missile->loaded_spr->sprites[0].width);
        missile->render_height = fix16_int32_from(missile->loaded_spr->sprites[0].height);

        /* Start missile pointed at the player */
        fix16_t x_diff = (loaded_level->player_pos.x + (loaded_level->player_width >> 1)) - (missile->x_pos + (missile->render_width >> 1));
        fix16_t y_diff = (loaded_level->player_pos.y + (loaded_level->player_height >> 1)) - (missile->y_pos + (missile->render_height >> 1));

        missile->angle = fix16_atan2(y_diff, x_diff);

        return missile;
}

void
missile_move(sprite_t *missile)
{
        fix16_t x_diff = (loaded_level->player_pos.x + (loaded_level->player_width >> 1)) - (missile->x_pos + (missile->render_width >> 1));
        fix16_t y_diff = (loaded_level->player_pos.y + (loaded_level->player_height >> 1)) - (missile->y_pos + (missile->render_height >> 1));
        fix16_t target_angle = fix16_atan2(y_diff, x_diff);

        /* Handle when target angle goes over the x axis */
        if (missile->angle > FIX16(90) && target_angle < FIX16(-90)) {
                missile->angle += MISSILE_ROTATE_SPEED;
        } else if (missile->angle < FIX16(-90) && target_angle > FIX16(90)) {
                missile->angle -= MISSILE_ROTATE_SPEED;
        }
        /* Make maximum speed the angle changes MISSILE_ROTATE_SPEED */
        else if (target_angle - missile->angle < MISSILE_ROTATE_SPEED && target_angle - missile->angle > -MISSILE_ROTATE_SPEED) {
                missile->angle = target_angle;
        } else if (target_angle > missile->angle) {
                missile->angle += MISSILE_ROTATE_SPEED;
        } else {
                missile->angle -= MISSILE_ROTATE_SPEED;
        }

        /* Handle when angle wraps around (SBL only goes from -180 to 180) */
        if (missile->angle > FIX16(180)) {
                missile->angle -= FIX16(360);
        } else if (missile->angle < FIX16(-180)) {
                missile->angle += FIX16(360);
        }

        missile->x_pos += fix16_mul(fix16_cos(missile->angle), MISSILE_SPEED);
        missile->y_pos += fix16_mul(fix16_sin(missile->angle), MISSILE_SPEED);

        /* If middle of missile touches a wall, destroy the missile */
        if (collision_point_check(missile->x_pos + FIX16(8), missile->y_pos + FIX16(8))) {
                sprite_list_delete(missile);
                explosion_make(missile->x_pos - FIX16(8), missile->y_pos - FIX16(8));
        }

        /* If missile hits the player, kill him */
        if (collision_player(missile)) {
                player_die();
        }
}
