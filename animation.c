#include <assert.h>

#include "animation.h"

bool
animation_playing(const sprite_t *sprite)
{
        return sprite->anim_playing;
}

void
animation_restart(sprite_t *sprite)
{
        assert(sprite != NULL);

        sprite->anim_playing = true;
        sprite->anim_cursor = 0;
        sprite->anim_timer = 0;
}

void
animation_update(sprite_t *sprite, const animation_t *animation)
{
        assert(sprite != NULL);

        if (animation == NULL) {
                sprite->anim_playing = false;

                return;
        }

        if (sprite->anim_cursor >= animation->frame_count) {
                sprite->anim_cursor = 0;
        }

        sprite->spr_index = animation->frame_offset + sprite->anim_cursor;

        if (sprite->anim_timer == 0) {
                /* If we're at the end of the animation, check if we need to
                 * loop */
                if (!animation->loop &&
                    (sprite->anim_cursor >= (animation->frame_count - 1))) {
                        sprite->anim_playing = false;
                        return;
                }

                sprite->anim_timer = animation->frame_duration;

                sprite->anim_cursor++;
        } else {
                sprite->anim_timer--;
        }

        sprite->anim_playing = true;
}
