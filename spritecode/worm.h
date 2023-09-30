#ifndef SPRITECODE_WORM_H
#define SPRITECODE_WORM_H

#include <stdint.h>

#include "../sprite.h"

void worm_init(void);
void worm_load(void);
sprite_t *worm_make(fix16_t x, fix16_t y);

#endif /* !SPRITECODE_WORM_H */
