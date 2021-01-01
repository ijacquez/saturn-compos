#ifndef SPRITECODE_MISSILE_H
#define SPRITECODE_MISSILE_H

#include <stdint.h>
#include <math.h>

#include "../sprite.h"

void missile_load(void);
sprite_t *missile_make(fix16_t x, fix16_t y);
void missile_move(sprite_t *missile);

#endif /* !SPRITECODE_MISSILE_H */
