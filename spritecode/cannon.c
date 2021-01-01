#include <stdint.h>
#include <math.h>

#include "cannon.h"
#include "missile.h"

#include "../graphics_refs.h"
#include "../sprite.h"
#include "../sound.h"
#include "../player.h"
#include "level.h"

#define CANNON_FIRE (150)

extern loaded_level_t *loaded_level;

static const loaded_spr_t *_loaded_spr;

void
cannon_load(void)
{
        _loaded_spr = sprite_load(OBJECT_ID_CANNON);
        assert(_loaded_spr != NULL);

        missile_load();
}

sprite_t *
cannon_make(fix16_t x, fix16_t y)
{
        sprite_t *cannon = sprite_list_next();
        sprite_make(x, y, cannon);
        cannon->iterate = &cannon_move;
        cannon->anim_timer = CANNON_FIRE;
        cannon->loaded_spr = _loaded_spr;
        cannon->render_width = fix16_int32_from(cannon->loaded_spr->sprites[0].width);
        cannon->render_height = fix16_int32_from(cannon->loaded_spr->sprites[0].height);

        return cannon;
}

void
cannon_move(sprite_t *cannon)
{
        cannon->anim_timer--;

        if (cannon->anim_timer == 0) {
                sprite_t * const missile = missile_make(cannon->x_pos, cannon->y_pos);

                missile->sort = cannon->sort + 1;

                cannon->anim_timer = CANNON_FIRE;
        }

        if (loaded_level->player_pos.x > cannon->x_pos) {
                cannon->mirror = SPRITE_MIRROR_HORZ;
        } else {
                cannon->mirror = 0;
        }
}
