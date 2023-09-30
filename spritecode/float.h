#ifndef SPRITECODE_FLOAT_H
#define SPRITECODE_FLOAT_H

#include <stdint.h>

#include "../sprite.h"

void float_init(void);
void float_load(void);
sprite_t *float_make(fix16_t x, fix16_t y);

#endif /* !SPRITECODE_FLOAT_H */
