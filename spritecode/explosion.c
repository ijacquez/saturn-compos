#include <stdint.h>

#include "explosion.h"

#include "../graphics_refs.h"
#include "../sprite.h"
#include "../sound.h"
#include "../player.h"

// How many frames to wait before switching to next frame
#define FRAME_DELAY (5)

static const loaded_spr_t *_loaded_spr;

void
explosion_load(void)
{
        _loaded_spr = sprite_load(OBJECT_ID_EXPLOSION);
        assert(_loaded_spr != NULL);
}

void
explosion_make(fix16_t x, fix16_t y)
{
        sprite_t *explosion = sprite_list_next();
        assert(explosion != NULL);

        sprite_make(x, y, explosion);
        explosion->loaded_spr = _loaded_spr;
        explosion->render_width = fix16_int32_from(explosion->loaded_spr->sprites[0].width);
        explosion->render_height = fix16_int32_from(explosion->loaded_spr->sprites[0].height);
        explosion->anim_timer = FRAME_DELAY;
        explosion->iterate = &explosion_move;

        sound_play(SOUND_EXPLOSION);
}

void
explosion_move(sprite_t *explosion)
{
        explosion->spr_index = explosion->anim_cursor;

        if (explosion->anim_timer == 0) {
                explosion->anim_cursor++;

                if (explosion->anim_cursor >= explosion->loaded_spr->sprite_count) {
                        sprite_list_delete(explosion);
                        return;
                }

                explosion->anim_timer = FRAME_DELAY;
        } else {
                explosion->anim_timer--;
        }
}
