#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "graphics/triggers.h"
#include "sprite.h"
#include "format_lvl.h"

void player_init(void);
void player_lvl_process(const lvl_t *lvl);
void player_animate(void);

// Returns 1 if the player can kill whatever sprite he's touching
int player_cankill(const sprite_t *sprite);

// Run when the player kills an enemy
void player_killenemy(void);

// Run to kill the player
void player_die(void);
void player_input(void);
void player_draw(void);

#endif
