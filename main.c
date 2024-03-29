#include <yaul.h>

#include <string.h>

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
        .coeff_table = VDP2_VRAM_CTL_COEFF_TABLE_VRAM,
        .vram_mode   = VDP2_VRAM_CTL_MODE_PART_BANK_BOTH
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

                dbgio_flush();
                vdp1_sync();
                vdp2_sync();
                vdp1_sync_wait();
                vdp2_sync_wait();
        }

        return 0;
}

void
user_init(void)
{
        vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
                                  VDP2_TVMD_VERT_224);

        vdp_sync_vblank_in_set(_vblank_in_handler, NULL);
        vdp_sync_vblank_out_set(_vblank_out_handler, NULL);

        vdp2_cram_mode_set(1);
        vdp2_vram_control_set(&_vdp2_vram_ctl);

        dbgio_init();
        dbgio_dev_default_init(DBGIO_DEV_MEDNAFEN_DEBUG);

        vdp2_tvmd_display_set();

        cpu_cache_purge();

        cpu_intc_mask_set(0);

        smpc_peripheral_init();
}

void
_vblank_in_handler(void *work __unused)
{
        smpc_peripheral_intback_issue();
        /* XXX: Have this be a callback list */
        sound_vblank_in();
}

static void
_vblank_out_handler(void *work __unused)
{
}
