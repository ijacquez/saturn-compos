#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>

#include "sprite.h"

typedef struct animation {
        const char *name;
        bool loop;
        uint8_t frame_offset;
        uint8_t frame_count;
        uint8_t frame_duration;
} animation_t;

#define ANIMATION_DEFINE(_name, _offset, _count, _duration, _loop) {           \
        .name = _name,                                                         \
        .loop = _loop,                                                         \
        .frame_offset = (_offset),                                             \
        .frame_count = (_count),                                               \
        .frame_duration = (_duration)                                          \
    }

bool animation_playing(const sprite_t *sprite);
void animation_restart(sprite_t *sprite);
void animation_update(sprite_t *sprite, const animation_t *animation);

#endif /* !ANIMATION_H */
