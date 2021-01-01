#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <math.h>

typedef struct loaded_level {
        int8_t level;

        fix16_vec2_t player_pos;
        uint16_t player_width;
        uint16_t player_height;
        fix16_vec2_t player_start;
} loaded_level_t;

void level_init(void);
void level_load(uint8_t level);
void level_reload(void);
void level_load_loop(void);

#endif /* !LEVEL_H */
