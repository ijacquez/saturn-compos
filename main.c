#include <yaul.h>

#include <string.h>
#include <math.h>

#include "cd.h"
#include "enemies.h"
#include "graphics_refs.h"
#include "level.h"
#include "player.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "vdp1_hw.h"

static loaded_level_t _loaded_level;

/// Globals

loaded_level_t *loaded_level = &_loaded_level;
uint32_t frame_count = 0;

smpc_peripheral_digital_t per_digital;

static const vdp2_vram_ctl_t _vdp2_vram_ctl = {
        .coefficient_table = VDP2_VRAM_CTL_COEFFICIENT_TABLE_VRAM,
        .vram_mode =         VDP2_VRAM_CTL_MODE_PART_BANK_BOTH
};

static void _vblank_in_handler(void *work);
static void _vblank_out_handler(void *work);

int
main(void)
{
        cd_init();
        vdp1_init();
        print_init();
        sprite_init();
        sound_init();
        level_init();
        /* sound_cdda(2); */

        level_load(0);

        while (true) {
                frame_count++;

                level_load_loop();

                smpc_peripheral_process();
                smpc_peripheral_digital_port(1, &per_digital);

                /* dbgio_printf("[H[2J"); */
                /* dbgio_printf("frame_count: %lu\n", frame_count); */

                vdp1_hw_cmdt_list_start(); {
                        player_input();

                        scroll_move(SCROLL_BG_NEAR, FIX16(1.0f), FIX16(0.0f));
                        scroll_update(SCROLL_BG_PLAYFIELD);
                        scroll_update(SCROLL_BG_OVERLAP);

                        player_draw();
                        sprite_list_draw_all();
                } vdp1_hw_cmdt_list_finish();

                /* dbgio_flush(); */
                vdp_sync();
        }

        return 0;
}

void
user_init(void)
{
        vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
                                  VDP2_TVMD_VERT_224);

        vdp_sync_vblank_in_set(_vblank_in_handler);
        vdp_sync_vblank_out_set(_vblank_out_handler);

        vdp2_cram_mode_set(1);
        vdp2_vram_control_set(&_vdp2_vram_ctl);

        /* dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC); */
        /* dbgio_dev_font_load(); */
        /* dbgio_dev_font_load_wait(); */

        vdp2_tvmd_display_set();

        cpu_cache_purge();

        cpu_intc_mask_set(0);
}

void
_vblank_in_handler(void *work __unused)
{
        /* XXX: Have this be a callback list */
        sound_vblank_in();
}

static void
_vblank_out_handler(void *work __unused)
{
        smpc_peripheral_intback_issue();
}
