#include <stdint.h>

#include <gamemath/fix16.h>

#include "worm.h"
#include "explosion.h"

#include "../animation.h"
#include "../collision.h"
#include "../graphics_refs.h"
#include "../player.h"
#include "../print.h"
#include "../sound.h"
#include "../sprite.h"

#define WORM_COUNT      (4)

#define WORM_MAX_SPEED  FIX16(1.0f)
#define WORM_ACCEL      FIX16(0.333f)

typedef enum state_type {
        STATE_CRAWLING,
        STATE_DYING,
        STATE_DEAD
} state_type_t;

typedef enum anim_type {
        ANIM_TYPE_IDLE,
        ANIM_TYPE_WALK,
        ANIM_TYPE_DEATH
} anim_type_t;

typedef struct state {
        bool used;
        state_type_t state_type;
        anim_type_t anim_type;
        int32_t direction;
} state_t;

static const loaded_spr_t *_loaded_spr;

static const animation_t _animations[] = {
        ANIMATION_DEFINE("idle",  0, 4, 15, true),
        ANIMATION_DEFINE("walk",  4, 4, 15, true),
        ANIMATION_DEFINE("death", 8, 4, 24, false)
};

static state_t _states[WORM_COUNT];

static void _worm_move(sprite_t *worm);

void
worm_init(void)
{
        for (uint32_t i = 0; i < WORM_COUNT; i++) {
                _states[i].used = false;
        }
}

void
worm_load(void)
{
        _loaded_spr = sprite_load(OBJECT_ID_WORM);
        assert(_loaded_spr != NULL);
}

sprite_t *
worm_make(fix16_t x, fix16_t y)
{
        sprite_t *worm = sprite_list_next();
        assert(worm != NULL);

        sprite_make(x, y, worm);

        worm->loaded_spr = _loaded_spr;
        worm->render_width = fix16_int32_from(worm->loaded_spr->sprites[0].width);
        worm->render_height = fix16_int32_from(worm->loaded_spr->sprites[0].height);

        /* XXX: Hard coded -- add this to SPR! */
        worm->bb_width = FIX16(32.0f);
        worm->bb_height = FIX16(16.0f);
        worm->bb_x = FIX16(8.0f);
        worm->bb_y = FIX16(32.0f);

        worm->iterate = &_worm_move;

        /* XXX: Change this to use MEMB? Possibly, you can dynamically allocate
         *      a MEMB if worm_load is called */
        for (uint32_t i = 0; i < WORM_COUNT; i++) {
                if (!_states[i].used) {
                        _states[i].used = true;
                        _states[i].state_type = STATE_CRAWLING;
                        _states[i].anim_type = ANIM_TYPE_WALK;
                        _states[i].direction = 1;

                        worm->data = &_states[i];

                        break;
                }
        }

        assert(worm->data != NULL);

        return worm;
}

static void
_state_crawl(sprite_t *worm)
{
        state_t * const state = worm->data;

        worm->dx += WORM_ACCEL;
        if (abs(worm->dx) > WORM_MAX_SPEED) {
                worm->dx = WORM_MAX_SPEED;
        }

        worm->x_pos += state->direction * WORM_ACCEL;

        const fix16_t offset = state->direction * (worm->bb_x + (worm->bb_width / 2));

        if (!(collision_below_offset_check(worm, offset, FIX16(0.0f)))) {
                state->direction *= -1;

                worm->dx = FIX16(0.0f);
                worm->mirror ^= SPRITE_MIRROR_HORZ;
        }

        if (collision_player(worm)) {
                state->state_type = STATE_DYING;
        }
}

static void
_state_dying(sprite_t *worm)
{
        state_t * const state = worm->data;

        if (state->anim_type != ANIM_TYPE_DEATH) {
                state->anim_type = ANIM_TYPE_DEATH;

                animation_restart(worm);

                sound_play(SOUND_EXPLOSION);
        }

        if (!(animation_playing(worm))) {
                state->state_type = STATE_DEAD;
        }
}

static void
_state_dead(sprite_t *worm)
{
        state_t * const state = worm->data;

        state->used = false;

        sprite_list_delete(worm);
}

static void
_worm_move(sprite_t *worm)
{
        state_t * const state = worm->data;

        switch (state->state_type) {
        case STATE_CRAWLING:
                _state_crawl(worm);
                break;
        case STATE_DYING:
                _state_dying(worm);
                break;
        case STATE_DEAD:
                _state_dead(worm);
                break;
        }

        collision_vert_eject(worm);

        animation_update(worm, &_animations[state->anim_type]);
}
