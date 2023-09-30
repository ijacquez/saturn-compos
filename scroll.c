#include <yaul.h>

#include <stdint.h>

#include <string.h>

#include <vdp2.h>

#include "cd.h"
#include "gamemath/uint32.h"
#include "graphics_refs.h"
#include "scroll.h"
#include "level.h"

#include "format_lvl.h"

/* Use: BG far */
#define NBG0_CPD                VDP2_VRAM_ADDR(1, 0x000000)
#define NBG1_CPD                VDP2_VRAM_ADDR(1, 0x010000)
#define NBG2_CPD                VDP2_VRAM_ADDR(2, 0x000000)
#define NBG3_CPD                VDP2_VRAM_ADDR(2, 0x010000)

#define NBG0_PAL                VDP2_CRAM_MODE_1_OFFSET(0, 0, 0)
#define NBG1_PAL                VDP2_CRAM_MODE_1_OFFSET(1, 0, 0)
#define NBG2_PAL                VDP2_CRAM_MODE_1_OFFSET(2, 0, 0)
#define NBG3_PAL                VDP2_CRAM_MODE_1_OFFSET(3, 0, 0)

#define NBG0_MAP_PLANE_A        VDP2_VRAM_ADDR(0, 0x000000)
#define NBG1_MAP_PLANE_A        VDP2_VRAM_ADDR(0, 0x000800)
#define NBG2_MAP_PLANE_A        VDP2_VRAM_ADDR(0, 0x001000)
#define NBG3_MAP_PLANE_A        VDP2_VRAM_ADDR(0, 0x001800)

#define NBG0_MAP_PLANE_B        NBG0_MAP_PLANE_A
#define NBG1_MAP_PLANE_B        NBG1_MAP_PLANE_A
#define NBG2_MAP_PLANE_B        NBG2_MAP_PLANE_A
#define NBG3_MAP_PLANE_B        NBG3_MAP_PLANE_A

#define NBG0_MAP_PLANE_C        NBG0_MAP_PLANE_A
#define NBG1_MAP_PLANE_C        NBG1_MAP_PLANE_A
#define NBG2_MAP_PLANE_C        NBG2_MAP_PLANE_A
#define NBG3_MAP_PLANE_C        NBG3_MAP_PLANE_A

#define NBG0_MAP_PLANE_D        NBG0_MAP_PLANE_A
#define NBG1_MAP_PLANE_D        NBG1_MAP_PLANE_A
#define NBG2_MAP_PLANE_D        NBG2_MAP_PLANE_A
#define NBG3_MAP_PLANE_D        NBG3_MAP_PLANE_A

#define BACK_SCREEN_ADDR        VDP2_VRAM_ADDR(3, 0x01FFFE)
#define NBG0_LINE_SCROLL        VDP2_VRAM_ADDR(0, 0x002000)

#define BG_CPD_PTR(scroll)      ((uint16_t *)_table_cpd_addrs[(scroll)])
#define BG_PAL_PTR(scroll)      ((uint16_t *)_table_pal_addrs[(scroll)])
#define BG_MAP_PND_PTR(scroll)  ((uint16_t *)_table_pnd_addrs[(scroll)])
#define BG_MAP_PND_BITS(scroll) ((uint16_t)_table_pnd_bits[(scroll)])

extern loaded_level_t *loaded_level;

fix16_t scrolls_x[4]   = {0, 0, 0, 0};
fix16_t scrolls_y[4]   = {0, 0, 0, 0};
int32_t map_tiles_x[4] = {0, 0, 0, 0};
int32_t map_tiles_y[4] = {0, 0, 0, 0};
uint32_t copy_modes[4] = {0, 0, 0, 0};
const void *maps[4]    = {NULL, NULL, NULL, NULL}; // Map locations in WRAM for the scrolling backgrounds
int scroll_xsize = 0;
int scroll_ysize = 0;

static const uintptr_t _table_cpd_addrs[] = {
        NBG0_CPD,
        NBG1_CPD,
        NBG2_CPD,
        NBG3_CPD
};

static const uintptr_t _table_pal_addrs[] = {
        NBG0_PAL,
        NBG1_PAL,
        NBG2_PAL,
        NBG3_PAL
};

static const uintptr_t _table_pnd_addrs[] = {
        NBG0_MAP_PLANE_A,
        NBG1_MAP_PLANE_A,
        NBG2_MAP_PLANE_A,
        NBG3_MAP_PLANE_A
};

/// Holds the precalculated PND bits for each scroll screen
static const uint16_t _table_pnd_bits[] = {
        VDP2_SCRN_PND_CONFIG_6(1, NBG0_CPD, NBG0_PAL, /* vf = */ 0, /* hf = */ 0),
        VDP2_SCRN_PND_CONFIG_6(1, NBG1_CPD, NBG1_PAL, /* vf = */ 0, /* hf = */ 0),
        VDP2_SCRN_PND_CONFIG_6(1, NBG2_CPD, NBG2_PAL, /* vf = */ 0, /* hf = */ 0),
        VDP2_SCRN_PND_CONFIG_6(1, NBG3_CPD, NBG3_PAL, /* vf = */ 0, /* hf = */ 0)
};

static const vdp2_vram_cycp_t _vram_cycp = {
        {
                {
                        .raw = 0x0123EEEE
                },
                {
                        .raw = 0x4455EEEE
                },
                {
                        .raw = 0x6677FFFF
                },
                {
                        .raw = 0xEEEEEEEE
                }
        }
};

static const vdp2_scrn_cell_format_t _nbg0_format = {
        .scroll_screen = VDP2_SCRN_NBG0,
        .ccc           = VDP2_SCRN_CCC_PALETTE_256,
        .char_size     = VDP2_SCRN_CHAR_SIZE_2X2,
        .pnd_size      = 1,
        .aux_mode      = VDP2_SCRN_AUX_MODE_0,
        .plane_size    = VDP2_SCRN_PLANE_SIZE_1X1,
        .cpd_base      = NBG0_CPD,
        .palette_base  = NBG0_PAL
};

static const vdp2_scrn_normal_map_t _nbg0_normal_map = {
        .plane_a = NBG0_MAP_PLANE_A,
        .plane_b = NBG0_MAP_PLANE_B,
        .plane_c = NBG0_MAP_PLANE_C,
        .plane_d = NBG0_MAP_PLANE_D
};

static const vdp2_scrn_cell_format_t _nbg1_format = {
        .scroll_screen = VDP2_SCRN_NBG1,
        .ccc           = VDP2_SCRN_CCC_PALETTE_256,
        .char_size     = VDP2_SCRN_CHAR_SIZE_2X2,
        .pnd_size      = 1,
        .aux_mode      = VDP2_SCRN_AUX_MODE_0,
        .plane_size    = VDP2_SCRN_PLANE_SIZE_1X1,
        .cpd_base      = NBG1_CPD,
        .palette_base  = NBG1_PAL,
};

static const vdp2_scrn_normal_map_t _nbg1_normal_map = {
        .plane_a = NBG1_MAP_PLANE_A,
        .plane_b = NBG1_MAP_PLANE_B,
        .plane_c = NBG1_MAP_PLANE_C,
        .plane_d = NBG1_MAP_PLANE_D
};

static const vdp2_scrn_cell_format_t _nbg2_format = {
        .scroll_screen = VDP2_SCRN_NBG2,
        .ccc           = VDP2_SCRN_CCC_PALETTE_256,
        .char_size     = VDP2_SCRN_CHAR_SIZE_2X2,
        .pnd_size      = 1,
        .aux_mode      = VDP2_SCRN_AUX_MODE_0,
        .plane_size    = VDP2_SCRN_PLANE_SIZE_1X1,
        .cpd_base      = NBG2_CPD,
        .palette_base  = NBG2_PAL
};

static const vdp2_scrn_normal_map_t _nbg2_normal_map = {
        .plane_a = NBG2_MAP_PLANE_A,
        .plane_b = NBG2_MAP_PLANE_B,
        .plane_c = NBG2_MAP_PLANE_C,
        .plane_d = NBG2_MAP_PLANE_D
};

static const vdp2_scrn_cell_format_t _nbg3_format = {
        .scroll_screen = VDP2_SCRN_NBG3,
        .ccc           = VDP2_SCRN_CCC_PALETTE_256,
        .char_size     = VDP2_SCRN_CHAR_SIZE_2X2,
        .pnd_size      = 1,
        .aux_mode      = VDP2_SCRN_AUX_MODE_0,
        .plane_size    = VDP2_SCRN_PLANE_SIZE_1X1,
        .cpd_base      = NBG3_CPD,
        .palette_base  = NBG3_PAL
};

static const vdp2_scrn_normal_map_t _nbg3_normal_map = {
        .plane_a = NBG3_MAP_PLANE_A,
        .plane_b = NBG3_MAP_PLANE_B,
        .plane_c = NBG3_MAP_PLANE_C,
        .plane_d = NBG3_MAP_PLANE_D
};

static const vdp2_scrn_ls_format_t _nbg0_ls_format = {
        .scroll_screen = VDP2_SCRN_NBG0,
        .table_base    = NBG0_LINE_SCROLL,
        .interval      = 0,
        .type          = VDP2_SCRN_LS_TYPE_HORZ
};

static void _scroll_lvl_playfield_process(const lvl_t *level, const layer_t *layer);
static void _scroll_lvl_near_process(const lvl_t *level, const layer_t *layer);
static void _scroll_lvl_far_process(const lvl_t *level, const layer_t *layer);
static void _scroll_lvl_overlap_process(const lvl_t *level, const layer_t *layer);

static void _scroll_screen_move(scroll_t scroll, uint32_t row);

static bool _scroll_bounds_check(scroll_t scroll, int32_t x, int32_t y);
static uint16_t _scroll_pnd_get(scroll_t scroll, int32_t x, int32_t y);

static vdp2_scrn_t _scroll_scrn_map(scroll_t scroll);

void
scroll_init(void)
{
        vdp2_scrn_cell_format_set(&_nbg0_format, &_nbg0_normal_map);
        vdp2_scrn_cell_format_set(&_nbg1_format, &_nbg1_normal_map);
        vdp2_scrn_cell_format_set(&_nbg2_format, &_nbg2_normal_map);
        vdp2_scrn_cell_format_set(&_nbg3_format, &_nbg3_normal_map);

        vdp2_scrn_ls_set(&_nbg0_ls_format);

        vdp2_scrn_ls_h_t * const table = (vdp2_scrn_ls_h_t *)NBG0_LINE_SCROLL;

        for (int32_t i = 0; i < 224; i++) {
                table[i].horz = FIX16(0.0);
        }

        vdp2_vram_cycp_set(&_vram_cycp);

        vdp2_scrn_scroll_x_set(VDP2_SCRN_NBG0, FIX16(0.0));
        vdp2_scrn_scroll_y_set(VDP2_SCRN_NBG0, FIX16(0.0));

        vdp2_scrn_scroll_x_set(VDP2_SCRN_NBG1, FIX16(0.0));
        vdp2_scrn_scroll_y_set(VDP2_SCRN_NBG1, FIX16(0.0));

        vdp2_scrn_scroll_x_set(VDP2_SCRN_NBG2, FIX16(0.0));
        vdp2_scrn_scroll_y_set(VDP2_SCRN_NBG2, FIX16(0.0));

        vdp2_scrn_scroll_x_set(VDP2_SCRN_NBG3, FIX16(0.0));
        vdp2_scrn_scroll_y_set(VDP2_SCRN_NBG3, FIX16(0.0));

        vdp2_scrn_priority_set(VDP2_SCRN_NBG3, 6);
        vdp2_scrn_priority_set(VDP2_SCRN_NBG2, 3);
        vdp2_scrn_priority_set(VDP2_SCRN_NBG1, 2);
        vdp2_scrn_priority_set(VDP2_SCRN_NBG0, 1);

        vdp2_scrn_display_set(VDP2_SCRN_DISPTP_NBG0 |
                              VDP2_SCRN_DISPTP_NBG1 |
                              VDP2_SCRN_DISPTP_NBG2 |
                              VDP2_SCRN_DISPTP_NBG3);

        scroll_reset();
}

void
scroll_lvl_process(const lvl_t *lvl)
{
        scroll_init();

        uintptr_t offset;
        offset = (uintptr_t)lvl + sizeof(lvl_t);

        if (lvl->playfield_byte_size != 0) {
                dbgio_printf("playfield_byte_size: %i\n", lvl->playfield_byte_size);
                const layer_t * const playfield = (layer_t *)offset;

                _scroll_lvl_playfield_process(lvl, playfield);

                offset += lvl->playfield_byte_size;
        }

        if (lvl->near_byte_size != 0) {
                dbgio_printf("near_byte_size: %i\n", lvl->near_byte_size);
                const layer_t * const near = (layer_t *)offset;

                _scroll_lvl_near_process(lvl, near);

                offset += lvl->near_byte_size;
        }

        if (lvl->far_byte_size != 0) {
                dbgio_printf("far_byte_size: %i\n", lvl->far_byte_size);
                const layer_t * const far = (layer_t *)offset;

                _scroll_lvl_far_process(lvl, far);

                offset += lvl->far_byte_size;
        }

        if (lvl->overlap_byte_size != 0) {
                dbgio_printf("overlap_byte_size: %i\n", lvl->overlap_byte_size);
                const layer_t * const overlap = (layer_t *)offset;

                _scroll_lvl_overlap_process(lvl, overlap);

                offset += lvl->overlap_byte_size;
        }
}

void
scroll_move(scroll_t scroll, fix16_t x, fix16_t y)
{
        scrolls_x[scroll] += x;

        int32_t curr_tile_x;

        const uint32_t scrn = _scroll_scrn_map(scroll);

        /* Playfield and overlap use flip-style scrolling, so don't change y
         * position here for them */
        switch (scroll) {
        case SCROLL_BG_PLAYFIELD:
        case SCROLL_BG_OVERLAP:
                curr_tile_x = fix16_int32_to(scrolls_x[scroll]) >> 4; /* Tile size is 16x16 */

                /* curr_tile_y = fix16_int32_to(scrolls_y[num]) >> 4; */
                /* copy_modes[num] = 0; */
                if ((curr_tile_x - map_tiles_x[scroll]) > 0) { // If x value increasing
                        copy_modes[scroll] |= COPY_MODE_RCOL;
                } else if ((curr_tile_x - map_tiles_x[scroll]) < 0) { // If x value decreasing
                        copy_modes[scroll] |= COPY_MODE_LCOL;
                }

                map_tiles_x[scroll] = curr_tile_x;
                /* map_tiles_y[num] = curr_tile_y; */

                vdp2_scrn_scroll_x_set(scrn, scrolls_x[scroll]);
                vdp2_scrn_scroll_y_set(scrn, FIX16(0.0));
                break;
        case SCROLL_BG_NEAR:
        case SCROLL_BG_FAR:
                scrolls_y[scroll] += y;

                vdp2_scrn_scroll_x_set(scrn, scrolls_x[scroll]);
                vdp2_scrn_scroll_y_set(scrn, scrolls_y[scroll]);
                break;
        }
}

void
scroll_set(scroll_t scroll, fix16_t x, fix16_t y)
{
        scroll_move(scroll, x - scrolls_x[scroll], y - scrolls_y[scroll]);
}

void
scroll_scale(scroll_t scroll, fix16_t scale)
{
}

void
scroll_backcolor_set(rgb1555_t color)
{
        vdp2_scrn_back_color_set(BACK_SCREEN_ADDR, color);
}

map_value_flags_t
scroll_flags_get(scroll_t scroll, int32_t x, int32_t y)
{
        if (!(_scroll_bounds_check(scroll, x, y))) {
                return MAP_VALUE_FLAG_NONE;
        }

        const map_value_t * const map_value_ptr = (const map_value_t *)maps[scroll];

        switch (scroll) {
        case SCROLL_BG_PLAYFIELD:
                return map_value_ptr[x + (y * scroll_xsize)].flags;
        case SCROLL_BG_FAR:
        case SCROLL_BG_NEAR:
        case SCROLL_BG_OVERLAP:
        default:
                return MAP_VALUE_FLAG_NONE;
        }
}

trigger_id_t
scroll_trigger_index_get(scroll_t scroll, int32_t x, int32_t y)
{
        if (!(_scroll_bounds_check(scroll, x, y))) {
                return -1;
        }

        const map_value_t * const map_value_ptr = (const map_value_t *)maps[scroll];

        switch (scroll) {
        case SCROLL_BG_PLAYFIELD:
                return (trigger_id_t)map_value_ptr[x + (y * scroll_xsize)].index;
        case SCROLL_BG_FAR:
        case SCROLL_BG_NEAR:
        case SCROLL_BG_OVERLAP:
        default:
                return TRIGGER_ID_INVALID;
        }
}

void
scroll_update(scroll_t scroll)
{
        uint16_t * const vram_ptr = BG_MAP_PND_PTR(scroll);

        if ((copy_modes[scroll] & COPY_MODE_RCOL) == COPY_MODE_RCOL) {
                for (int32_t i = 0; i < 16; i++) {
                        const int32_t pos = (i * 32) + ((map_tiles_x[scroll] + SCREEN_TILES_X) & 31);

                        if (pos >= 0) {
                                const int32_t x = map_tiles_x[scroll] + SCREEN_TILES_X;
                                const int32_t y = i + map_tiles_y[scroll];

                                vram_ptr[pos] = _scroll_pnd_get(scroll, x, y);
                        }
                }
        }

        if ((copy_modes[scroll] & COPY_MODE_LCOL) == COPY_MODE_LCOL) {
                for (int32_t i = 0; i < 16; i++) {
                        const int32_t pos = (i * 32) + ((map_tiles_x[scroll] - 1) & 31);

                        if (pos >= 0) {
                                const int32_t x = map_tiles_x[scroll] - 1;
                                const int32_t y = i + map_tiles_y[scroll];

                                vram_ptr[pos] = _scroll_pnd_get(scroll, x, y);
                        }
                }
        }
}

void
scroll_reset(void)
{
        scroll_set(SCROLL_BG_PLAYFIELD, FIX16(0.0), FIX16(0.0));
        scroll_set(SCROLL_BG_OVERLAP, FIX16(0.0), FIX16(0.0));
        scroll_set(SCROLL_BG_FAR, FIX16(0.0), FIX16(0.0));
        scroll_set(SCROLL_BG_NEAR, FIX16(0.0), FIX16(0.0));
}

void
scroll_linescroll4(scroll_t scroll, fix16_t scroll_val, int boundary1, int boundary2, int boundary3)
{
        vdp2_scrn_ls_h_t * const table = (vdp2_scrn_ls_h_t *)NBG0_LINE_SCROLL;

        for (int32_t i = 0; i < 224; i++) {
                if (i < boundary1) {
                        table[i].horz = (scroll_val >> 3);
                } else if (i < boundary2) {
                        table[i].horz = (scroll_val >> 2);
                } else if (i < boundary3) {
                        table[i].horz = (scroll_val >> 1);
                } else {
                        table[i].horz = scroll_val;
                }
        }
}

void
scroll_screen_move(uint32_t row)
{
        _scroll_screen_move(SCROLL_BG_PLAYFIELD, row);
        _scroll_screen_move(SCROLL_BG_OVERLAP, row);
}

static void
_scroll_screen_move(scroll_t scroll, uint32_t row)
{
        int pos;

        map_tiles_y[scroll] = row;

        /* Convert to int: << 16, tiles to 16px : << 4 */
        scrolls_y[scroll] = fix16_int32_from(map_tiles_y[scroll]) << 4;

        uint16_t * const vram_ptr = BG_MAP_PND_PTR(scroll);

        for (int32_t i = 0; i < 14; i++) {
                for (int32_t j = -1; j < 31; j++) {
                        pos = (i * 32) + ((j + map_tiles_x[scroll]) & 31);

                        if (pos >= 0) {
                                const int32_t x = j + map_tiles_x[scroll];
                                const int32_t y = i + map_tiles_y[scroll];

                                vram_ptr[pos] = _scroll_pnd_get(scroll, x, y);
                        }
                }
        }
}

static void
_scroll_lvl_vram_copy(const layer_t *layer, uint32_t scroll)
{
        const void * const cpd_ptr =
            (void *)((uintptr_t)layer + sizeof(layer_t) + layer->pnd_byte_size);

        const rgb1555_t * const palette_ptr =
            (rgb1555_t *)((uintptr_t)cpd_ptr + layer->cg_byte_size);

        (void)memcpy(BG_CPD_PTR(scroll), cpd_ptr, layer->cg_byte_size);

        dbgio_printf("%s: CPD: 0x%08X (%iB) -> 0x%08X, PAL: 0x%08X (%i count) -> 0x%08X\n",
            "_scroll_lvl_vram_copy",
            (uintptr_t)cpd_ptr, layer->cg_byte_size, BG_CPD_PTR(scroll),
            (uintptr_t)palette_ptr, layer->palette_count, BG_PAL_PTR(scroll));

        rgb1555_t * const cram = (rgb1555_t *)BG_PAL_PTR(scroll);
        for (uint32_t i = 0; i < layer->palette_count; i++) {
                cram[i] = palette_ptr[i];
        }
}

static void
_scroll_lvl_playfield_process(const lvl_t *lvl, const layer_t *layer)
{
        _scroll_lvl_vram_copy(layer, SCROLL_BG_PLAYFIELD);

        const void * const map_ptr =
            (void *)((uintptr_t)layer + sizeof(layer_t));

        maps[SCROLL_BG_PLAYFIELD] = map_ptr;

        scroll_xsize = layer->width;
        scroll_ysize = layer->height;
}

static void
_scroll_lvl_near_process(const lvl_t *lvl, const layer_t *layer)
{
        _scroll_lvl_vram_copy(layer, SCROLL_BG_NEAR);

        const uint16_t * const pnd_ptr =
            (uint16_t *)((uintptr_t)layer + sizeof(layer_t));

        uint16_t * const tilemap_vram = BG_MAP_PND_PTR(SCROLL_BG_NEAR);
        for (uint32_t i = 0; i < (layer->pnd_byte_size / sizeof(uint16_t)); i++) {
                tilemap_vram[i] = pnd_ptr[i] | BG_MAP_PND_BITS(SCROLL_BG_NEAR);
        }
}

static void
_scroll_lvl_far_process(const lvl_t *lvl, const layer_t *layer)
{
        _scroll_lvl_vram_copy(layer, SCROLL_BG_FAR);

        const uint16_t * const pnd_ptr =
            (uint16_t *)((uintptr_t)layer + sizeof(layer_t));

        uint16_t * const tilemap_vram = BG_MAP_PND_PTR(SCROLL_BG_FAR);
        for (uint32_t i = 0; i < (layer->pnd_byte_size / sizeof(uint16_t)); i++) {
                tilemap_vram[i] = pnd_ptr[i] | BG_MAP_PND_BITS(SCROLL_BG_FAR);
        }
}

static void
_scroll_lvl_overlap_process(const lvl_t *lvl, const layer_t *layer)
{
        _scroll_lvl_vram_copy(layer, SCROLL_BG_OVERLAP);

        const uint16_t * const pnd_ptr =
            (uint16_t *)((uintptr_t)layer + sizeof(layer_t));

        maps[SCROLL_BG_OVERLAP] = pnd_ptr;
}

static bool
_scroll_bounds_check(scroll_t scroll, int32_t x, int32_t y)
{
        const void * const map_ptr = maps[scroll];

        if ((map_ptr == NULL) || (x >= scroll_xsize) || (y >= scroll_ysize) || (y < 0)) {
                return false;
        }

        if (x < 0) {
                return false;
        }

        return true;
}

static uint16_t
_scroll_pnd_get(scroll_t scroll, int32_t x, int32_t y)
{
        uint16_t value;
        value = 0x0000;

        if ((_scroll_bounds_check(scroll, x, y))) {
                const uint16_t * const pnd_ptr = (const uint16_t *)maps[scroll];
                const map_value_t * const map_value_ptr = (const map_value_t *)maps[scroll];

                const uint32_t map_offset = x + (y * scroll_xsize);

                switch (scroll) {
                case SCROLL_BG_PLAYFIELD:
                        value = map_value_ptr[map_offset].pnd;
                        break;
                case SCROLL_BG_FAR:
                case SCROLL_BG_NEAR:
                case SCROLL_BG_OVERLAP:
                        value = pnd_ptr[map_offset];
                        break;
                }
        }

        return (value | BG_MAP_PND_BITS(scroll));
}

static vdp2_scrn_t
_scroll_scrn_map(scroll_t scroll)
{
        switch (scroll) {
        case SCROLL_BG_OVERLAP:
                return VDP2_SCRN_NBG3;
        case SCROLL_BG_FAR:
                return VDP2_SCRN_NBG0;
        case SCROLL_BG_NEAR:
                return VDP2_SCRN_NBG1;
        case SCROLL_BG_PLAYFIELD:
        default:
                return VDP2_SCRN_NBG2;
        }
}
