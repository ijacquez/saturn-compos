#ifndef SPRITECODE_EXPLOSION_H
#define SPRITECODE_EXPLOSION_H

#include "../sprite.h"

void explosion_load(void);
void explosion_make(fix16_t x, fix16_t y);
void explosion_move(sprite_t *explosion);

#endif /* !SPRITECODE_EXPLOSION_H */
