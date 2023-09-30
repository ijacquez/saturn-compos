#ifndef SPRITECODE_CANNON_H
#define SPRITECODE_CANNON_H

#include <stdint.h>

#include "../sprite.h"

void cannon_load(void);
sprite_t *cannon_make(fix16_t x, fix16_t y);
void cannon_move(sprite_t *cannon);

#endif /* !SPRITECODE_CANNON_H */
