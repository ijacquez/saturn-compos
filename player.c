#include <yaul.h>

#include <string.h>

#include "animation.h"
#include "collision.h"
#include "enemies.h"
#include "level.h"
#include "player.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"

#define PLAYER_ACCEL            (FIX16(0.25f))
#define PLAYER_MAX_SPEED        (FIX16(2.5f))
// Furthest right the player sprite can go
#define PLAYER_MAX_X_POS        (FIX16(140.0f))
// Jump stuff
#define PLAYER_MAX_JUMPS        (1)
#define PLAYER_JUMP_SPEED       (-FIX16(5.0f))

typedef enum anim_type {
        PLAYER_ANIM_TYPE_IDLE,
        PLAYER_ANIM_TYPE_WALK,
        PLAYER_ANIM_TYPE_RUN,
        PLAYER_ANIM_TYPE_JUMP,
        PLAYER_ANIM_TYPE_DEATH,
        PLAYER_ANIM_TYPE_ATTACK,
        PLAYER_ANIM_TYPE_INVALID = 0xFF
} anim_type_t;

/* XXX: Hard coded */
extern smpc_peripheral_digital_t per_digital;
extern loaded_level_t *loaded_level;

static const animation_t _player_anims[] = {
        ANIMATION_DEFINE("idle",    0,  4,  8, true),
        ANIMATION_DEFINE("walk",    4,  4,  8, true),
        ANIMATION_DEFINE("run",     8,  4,  8, true),
        ANIMATION_DEFINE("jump",   12,  2,  8, true),
        ANIMATION_DEFINE("death",  14,  8, 12, false),
        ANIMATION_DEFINE("attack", 22,  4,  7, false)
};

static sprite_t _player;
static anim_type_t _prev_player_anim_type = PLAYER_ANIM_TYPE_INVALID;
static anim_type_t _player_anim_type = PLAYER_ANIM_TYPE_IDLE;
static bool _player_jumps = false;
static bool _player_grounded = false;

static void _loaded_level_state_update(void);

void
player_init(void)
{
        sprite_make(0, 0, &_player);

        const loaded_spr_t *loaded_spr = sprite_load(OBJECT_ID_GUY);

        _player.loaded_spr = loaded_spr;

        _player.render_width = fix16_int32_from(_player.loaded_spr->sprites[0].width);
        _player.render_height = fix16_int32_from(_player.loaded_spr->sprites[0].height);

        /* XXX: Hard coded -- add this to SPR! */
        _player.bb_width = FIX16(16.0f);
        _player.bb_height = FIX16(16.0f);
        _player.bb_x = FIX16(16.0f);
        _player.bb_y = FIX16(32.0f);

        _loaded_level_state_update();
}

void
player_lvl_process(const lvl_t *lvl)
{
        player_init();

        _player.x_pos = loaded_level->player_start.x;
        _player.y_pos = loaded_level->player_start.y;

        _loaded_level_state_update();
}

int
player_cankill(const sprite_t *sprite)
{
        return ((_player.dy > 0)) && (_player.y_pos < sprite->y_pos);
}

// Run when the player kills an enemy
void
player_killenemy(void)
{
        _player.dy = PLAYER_JUMP_SPEED;
}

// Run when the player dies
void
player_die(void)
{
        _player.x_pos = loaded_level->player_start.x;
        _player.y_pos = loaded_level->player_start.y;
        _player.dx = 0;
        _player.dy = 0;

        _loaded_level_state_update();

        _player_jumps = false;

        /* XXX: Hack */
        level_reload();
}

void
player_input(void)
{
        if ((per_digital.pressed.raw & PERIPHERAL_DIGITAL_LEFT) != 0) {
                if (_player.dx > -PLAYER_MAX_SPEED) {
                        _player.dx -= PLAYER_ACCEL;
                } else {
                        _player.dx += PLAYER_ACCEL;
                }

                _player.mirror = SPRITE_MIRROR_HORZ;
        } else if ((per_digital.pressed.raw & PERIPHERAL_DIGITAL_RIGHT) != 0) {
                if (_player.dx < PLAYER_MAX_SPEED) {
                        _player.dx += PLAYER_ACCEL;
                } else {
                        _player.dx -= PLAYER_ACCEL;
                }

                _player.mirror &= ~SPRITE_MIRROR_HORZ;
        } else {
                if (_player.dx > 0) {
                        _player.dx -= PLAYER_ACCEL;
                } else if (_player.dx < 0) {
                        _player.dx += PLAYER_ACCEL;
                }
        }

        if ((per_digital.pressed.raw & PERIPHERAL_DIGITAL_A) != 0) {
        }

        _player.x_pos += _player.dx;

        player_animate();
        collision_horiz_eject(&_player);

        if (((per_digital.pressed.raw & PERIPHERAL_DIGITAL_B) != 0) && !_player_jumps) {
                _player.dy = PLAYER_JUMP_SPEED;
                _player_jumps = true;

                sound_play(SOUND_JUMP);
        }
        // If you hold the jump button longer, jump higher
        else if (((per_digital.pressed.raw & PERIPHERAL_DIGITAL_B) != 0) && (_player.dy < 0)) {
                _player.dy += FIX16(0.25f);
        } else {
                _player.dy += SPRITE_GRAVITY;

                if (_player.dy > SPRITE_MAXFALLSPEED) {
                        _player.dy = SPRITE_MAXFALLSPEED;
                }

                // Jump button
                // If we're falling and on a solid surface or a slope
                if (((per_digital.pressed.raw & PERIPHERAL_DIGITAL_B) == 0) &&
                    (_player.dy > 0) &&
                    collision_below_check(&_player)) {
                        _player_jumps = false;
                }
        }

        if (collision_below_check(&_player)) {
                _player_grounded = true;
        } else {
                _player_grounded = false;
        }

        _player.y_pos += _player.dy;

        /* XXX: Hard coded */
        if (_player.y_pos >= FIX16(512.0f)) {
                player_die();
        }

        collision_vert_eject(&_player);

        const trigger_id_t trigger_id = collision_trigger_get(&_player);

        if (trigger_id != TRIGGER_ID_INVALID) {
                switch (trigger_id) {
                case TRIGGER_ID_RESTART_LEVEL:
                        player_die();
                        break;
                case TRIGGER_ID_PREVIOUS_LEVEL:
                        /* XXX: Hack */
                        level_load(loaded_level->level - 1);
                        break;
                case TRIGGER_ID_NEXT_LEVEL:
                        /* XXX: Hack */
                        level_load(loaded_level->level + 1);
                        break;
                default:
                        break;
                }
        }

        _loaded_level_state_update();
}

void
player_animate(void)
{
        if (_player_jumps && _player_grounded) {
                _player_anim_type = PLAYER_ANIM_TYPE_JUMP;
        } else if (_player.dx != FIX16(0.0f)) {
                _player_anim_type = PLAYER_ANIM_TYPE_RUN;
        } else {
                _player_anim_type = PLAYER_ANIM_TYPE_IDLE;
        }

        const animation_t * const animation =
            &_player_anims[_player_anim_type];

        /* print_buffer(0, 0, "Animation: %s", animation->name); */

        animation_update(&_player, animation);
}

/* Allows me to treat the player sprite like any other sprite while only moving
 * the screen around him */
void
player_draw(void)
{
        sprite_t copy_player;

        fix16_t bg_scroll_val;

        (void)memcpy(&copy_player, &_player, sizeof(sprite_t));

        if (copy_player.x_pos > PLAYER_MAX_X_POS) {
                fix16_t new_x_pos = copy_player.x_pos - PLAYER_MAX_X_POS;

                scroll_set(SCROLL_BG_PLAYFIELD, new_x_pos, 0);
                scroll_set(SCROLL_BG_OVERLAP, new_x_pos, 0);

                bg_scroll_val = new_x_pos >> 1;

                copy_player.x_pos = PLAYER_MAX_X_POS;
        } else {
                scroll_set(SCROLL_BG_PLAYFIELD, 0, 0);
                scroll_set(SCROLL_BG_OVERLAP, 0, 0);

                bg_scroll_val = 0;
        }

        fix16_t top_point = copy_player.y_pos + copy_player.render_height;

        /* XXX: Hard coded values */
        const fix16_t top_camera_y = FIX16(224.0f);
        const fix16_t bottom_camera_y = FIX16(448.0f);

        if ((top_point > top_camera_y) &&
            (copy_player.y_pos < bottom_camera_y)) {
                copy_player.y_pos -= top_camera_y;

                sprite_draw(&copy_player);
        }

        scroll_linescroll4(SCROLL_BG_NEAR, bg_scroll_val, 3 * 16, 6 * 16, 224);
}

static void
_loaded_level_state_update(void)
{
        loaded_level->player_pos.x = _player.x_pos;
        loaded_level->player_pos.y = _player.y_pos;
        loaded_level->player_width = fix16_int32_to(_player.bb_width);
        loaded_level->player_height = fix16_int32_to(_player.bb_height);
}
