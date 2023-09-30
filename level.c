#include <yaul.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "cd.h"
#include "enemies.h"
#include "player.h"
#include "scroll.h"
#include "sprite.h"
#include "level.h"
#include "vdp1_hw.h"
#include "print.h"

#include "format_lvl.h"

#define LEVEL_COUNT 4

extern loaded_level_t *loaded_level;

static const char *_level_filenames[] = {
        "LEVEL0.LVL",
        "LEVEL1.LVL",
        "LEVEL0.LVL",
        "LEVEL1.LVL"
};

static lvl_t * const _lvl = (lvl_t *)LWRAM(0x00010000);

void
level_init(void)
{
        loaded_level->level = -1;
}

/* XXX: Hack */
static void
_level_loading_screen_show(void)
{
        vdp2_scrn_display_set(VDP2_SCRN_DISP_NONE);

        scroll_backcolor_set(RGB1555(1, 0, 0, 0));

        /* XXX: Ugly way of clearing VDP1 screen */
        vdp2_sprite_priority_set(0, 0);

        vdp1_sync();
        vdp2_sync();
        vdp1_sync_wait();
        vdp2_sync_wait();

        vdp2_sprite_priority_set(0, 5);

        vdp1_hw_cmdt_list_start(); {
                print_buffer(1, 1, "NOW LOADING...");
        } vdp1_hw_cmdt_list_finish();

        vdp1_sync();
        vdp2_sync();
        vdp1_sync_wait();
        vdp2_sync_wait();

        vdp2_scrn_display_set(VDP2_SCRN_DISPTP_NBG0 |
                              VDP2_SCRN_DISPTP_NBG1 |
                              VDP2_SCRN_DISPTP_NBG2 |
                              VDP2_SCRN_DISPTP_NBG3);
}

#define LEVEL_REQUEST_NONE      (0)
#define LEVEL_REQUEST_LOAD      (1)
#define LEVEL_REQUEST_RELOAD    (2)

static uint32_t _loading_request = LEVEL_REQUEST_NONE;

static void
_state_level_reload(void)
{
        loaded_level->player_start.x = _lvl->player_x;
        loaded_level->player_start.y = _lvl->player_y;

        sprite_list_delete_all();

        player_lvl_process(_lvl);
        scroll_lvl_process(_lvl);
        enemies_lvl_process(_lvl);

        scroll_backcolor_set(RGB888_RGB1555(1, 36, 34, 52));

        const int32_t start_row = (fix16_int32_to(loaded_level->player_start.y) / 224) * 14;

        scroll_screen_move(start_row);
}

static void
_state_level_load(void)
{
        _level_loading_screen_show();

        /* Don't try to read from actual CD */
        const char * const level_filename = _level_filenames[loaded_level->level];
        cd_load_nosize(level_filename, _lvl);

        sprite_load_reset_all();

        _state_level_reload();
}

void
level_load_loop(void)
{
        if (_loading_request == LEVEL_REQUEST_NONE) {
                return;
        }

        switch (_loading_request) {
        case LEVEL_REQUEST_LOAD:
                _state_level_load();
                break;
        case LEVEL_REQUEST_RELOAD:
                _state_level_reload();
                break;
        }

        _loading_request = LEVEL_REQUEST_NONE;
}

void
level_load(uint8_t level)
{
        if (_loading_request != LEVEL_REQUEST_NONE) {
                return;
        }

        loaded_level->level = level & (LEVEL_COUNT - 1);

        _loading_request = LEVEL_REQUEST_LOAD;
}

void
level_reload(void)
{
        if (loaded_level->level < 0) {
                return;
        }

        if (_loading_request != LEVEL_REQUEST_NONE) {
                return;
        }

        _loading_request = LEVEL_REQUEST_RELOAD;
}
