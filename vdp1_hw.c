#include <string.h>

#include "vdp1_hw.h"
#include "sprite.h"

#define ORDER_PREAMBLE_INDEX            0
#define ORDER_SPRITE_START_INDEX        3
#define ORDER_SPRITE_END_INDEX          (ORDER_SPRITE_START_INDEX + 256)
#define ORDER_COUNT                     (ORDER_SPRITE_END_INDEX + 1)

static vdp1_env_t _vdp1_env;
static vdp1_vram_partitions_t _vdp1_vram_partitions;

static vdp1_cmdt_list_t _cmdt_list;
static vdp1_cmdt_t _cmdts[ORDER_COUNT];

static struct {
        vdp1_cmdt_t * const cmdts;
        uint32_t index;
        bool processing;
} _state = {
        .cmdts = &_cmdts[ORDER_SPRITE_START_INDEX],
        .index = 0,
        .processing = false
};

static const int16_vec2_t _origin = INT16_VEC2_INITIALIZER(0, 0);

static vdp1_vram_partitions_t _vram_partitions;

static void _cmdts_bubble_sort(void);

void
vdp1_init(void)
{
        vdp1_sync_interval_set(0);

        vdp1_vram_partitions_get(&_vdp1_vram_partitions);

        vdp1_env_default_init(&_vdp1_env);

        _vdp1_env.sprite_type = 5;

        vdp1_env_set(&_vdp1_env);

        vdp2_sprite_priority_set(0, 5);
        vdp2_sprite_priority_set(1, 4);
        vdp2_sprite_priority_set(2, 0);
        vdp2_sprite_priority_set(3, 0);
        vdp2_sprite_priority_set(4, 0);
        vdp2_sprite_priority_set(5, 0);
        vdp2_sprite_priority_set(6, 0);
        vdp2_sprite_priority_set(7, 0);

        vdp1_cmdt_list_init(&_cmdt_list, _cmdts);
        vdp1_env_preamble_populate(_cmdts, &_origin);

        (void)memcpy(&_vram_partitions, &_vdp1_vram_partitions, sizeof(vdp1_vram_partitions_t));

        vdp1_hw_vram_texture_free();
        vdp1_hw_vram_clut_free();
}

void *
vdp1_hw_vram_texture_base(void)
{
        return _vram_partitions.texture_base;
}

vdp1_clut_t *
vdp1_hw_vram_clut_base(void)
{
        return _vram_partitions.clut_base;
}

vdp1_vram_t
vdp1_hw_vram_texture_alloc(uint32_t bytes)
{
        uint32_t rounded_bytes = ((bytes + 31) >> 5) << 5;

        const vdp1_vram_t old_vram = (vdp1_vram_t)_vram_partitions.texture_base;
        const vdp1_vram_t new_vram = (vdp1_vram_t)_vram_partitions.texture_base + rounded_bytes;

        const int32_t diff = (int32_t)(new_vram - old_vram);

        assert((diff >= 0) && (diff <= (int32_t)_vram_partitions.texture_size));

        _vram_partitions.texture_base = (void *)new_vram;

        return old_vram;
}

vdp1_clut_t *
vdp1_hw_vram_clut_alloc(void)
{
        const vdp1_vram_t old_vram = (vdp1_vram_t)_vram_partitions.clut_base;
        const vdp1_vram_t new_vram = (vdp1_vram_t)_vram_partitions.clut_base + sizeof(vdp1_clut_t);

        assert((new_vram - old_vram) <= _vram_partitions.clut_size);

        _vram_partitions.clut_base = (vdp1_clut_t *)new_vram;

        return (vdp1_clut_t *)old_vram;
}

void
vdp1_hw_vram_texture_free(void)
{
        _vram_partitions.texture_base = _vdp1_vram_partitions.texture_base;
}

void
vdp1_hw_vram_clut_free(void)
{
        _vram_partitions.clut_base = _vdp1_vram_partitions.clut_base;
}

vdp1_cmdt_t *
vdp1_hw_cmdt_list_next(void)
{
        assert(_state.processing);

        if (_state.index >= ORDER_SPRITE_END_INDEX) {
                return NULL;
        }

        vdp1_cmdt_t * const cmdt = &_state.cmdts[_state.index];

        _state.index++;

        return cmdt;
}

uint32_t
vdp1_hw_cmdt_list_used_get(void)
{
        return _state.index;
}

void
vdp1_hw_cmdt_list_start(void)
{
        assert(!_state.processing);

        _state.processing = true;

        _state.index = 0;
}

void
vdp1_hw_cmdt_list_finish(void)
{
        assert(_state.processing);

        _state.processing = false;

        _cmdts_bubble_sort();

        vdp1_cmdt_t * const end_cmdt = &_state.cmdts[_state.index];

        /* Mark the end of list. If there are no sprites to draw, then we'll
         * terminate after the preamble */
        vdp1_cmdt_end_set(end_cmdt);

        _cmdt_list.count = VDP1_ENV_CMDT_PREAMBLE_COUNT + _state.index;

        vdp1_sync_cmdt_list_put(&_cmdt_list, 0, NULL, NULL);
}

static void
_cmdts_bubble_sort(void)
{
        vdp1_cmdt_t * const cmdts = &_state.cmdts[0];

        int32_t count = _state.index;
        bool swapped = true;

        while (swapped) {
                swapped = false;

                for (int32_t i = 1; i <= (count - 1); i++) {
                        const uint16_t sort_1 = cmdts[i - 1].reserved;
                        const uint16_t sort_2 = cmdts[i].reserved;

                        if (sort_1 < sort_2) {
                                vdp1_cmdt_t cmdt_swap;

                                (void)memcpy(&cmdt_swap, &cmdts[i - 1], sizeof(vdp1_cmdt_t));
                                (void)memcpy(&cmdts[i - 1], &cmdts[i], sizeof(vdp1_cmdt_t));
                                (void)memcpy(&cmdts[i], &cmdt_swap, sizeof(vdp1_cmdt_t));

                                swapped = true;
                        }
                }

                count--;
        }
}
