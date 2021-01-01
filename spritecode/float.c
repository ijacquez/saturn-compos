#include <stdint.h>
#include <math.h>

#include "float.h"

#include "../collision.h"
#include "../graphics_refs.h"
#include "../sprite.h"
#include "../player.h"

static const loaded_spr_t *_loaded_spr;

static void _float_move(sprite_t *floater);

void
float_init(void)
{
}

void
float_load(void)
{
        _loaded_spr = sprite_load(OBJECT_ID_FLOAT);
        assert(_loaded_spr != NULL);
}

sprite_t *
float_make(fix16_t x, fix16_t y)
{
        sprite_t *floater = sprite_list_next();
        assert(floater != NULL);

        sprite_make(x, y, floater);
        floater->loaded_spr = _loaded_spr;
        floater->render_width = fix16_int32_from(floater->loaded_spr->sprites[0].width);
        floater->render_height = fix16_int32_from(floater->loaded_spr->sprites[0].height);
        floater->iterate = &_float_move;

        /* XXX: Hard coded -- add this to SPR! */
        floater->bb_width = FIX16(48.0f);
        floater->bb_height = FIX16(16.0f);
        floater->bb_x = FIX16(0.0f);
        floater->bb_y = FIX16(0.0f);

        return floater;
}

static void
_float_move(sprite_t *floater)
{
}
