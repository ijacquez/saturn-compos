#include <yaul.h>

#include <string.h>

#include "cd.h"
#include "graphics_refs.h"
#include "scroll.h"
#include "sprite.h"
#include "vdp1_hw.h"

#define LOADED_SPRS_COUNT               (64)
#define LOADED_SPR_SPRITES_COUNT        (256)

static sprite_t _sprites[SPRITE_LIST_COUNT];
static uint32_t _sprite_list_count = 0;

static const sprite_t * const _start_sprite = &_sprites[0];
static const sprite_t * const _end_sprite = &_sprites[SPRITE_LIST_COUNT - 1];

static loaded_spr_t _loaded_sprs[LOADED_SPRS_COUNT];
static loaded_spr_sprite_t _loaded_spr_sprites[LOADED_SPR_SPRITES_COUNT];

typedef struct spr {
        char signature[4];
        uint16_t sprite_count;
        uint16_t sprite_size;
        uint16_t palette_count;
} __packed spr_t;

static_assert(sizeof(spr_t) == 10);

typedef struct spr_sprite {
        uint16_t width;
        uint16_t height;
} __packed spr_sprite_t;

static_assert(sizeof(spr_sprite_t) == 4);

static struct {
        loaded_spr_t * const loaded_sprs;
        loaded_spr_sprite_t * const loaded_spr_sprites;
        uint16_t spr_index;
        uint16_t spr_sprites_index;
} _sprite_loading = {
        .loaded_sprs        = _loaded_sprs,
        .loaded_spr_sprites = _loaded_spr_sprites
};

void sprite_load_reset_all(void);

void
sprite_init(void)
{
        sprite_load_reset_all();
        sprite_list_delete_all();
}

void
sprite_draw(sprite_t *sprite)
{
        int16_vec2_t xy[4];

        vdp1_cmdt_t * const sprite_cmdt = vdp1_hw_cmdt_list_next();
        assert(sprite_cmdt != NULL);

        vdp1_cmdt_draw_mode_t draw_mode = {
                .raw                  = 0x0000,
                .trans_pixel_disable  = false,
                .pre_clipping_disable = true,
                .end_code_disable     = true,
                .color_mode           = VDP1_CMDT_CM_CLUT_16
        };

        vdp1_cmdt_draw_mode_set(sprite_cmdt, draw_mode);

        if (sprite->loaded_spr != NULL) {
                const loaded_spr_sprite_t * const loaded_spr_sprite =
                    &sprite->loaded_spr->sprites[sprite->spr_index];

                vdp1_cmdt_char_base_set(sprite_cmdt,
                    (vdp1_vram_t)loaded_spr_sprite->texture_base);
                vdp1_cmdt_color_mode1_set(sprite_cmdt,
                    (vdp1_vram_t)loaded_spr_sprite->clut_base);

                vdp1_cmdt_char_size_set(sprite_cmdt, fix16_int32_to(sprite->render_width),
                    fix16_int32_to(sprite->render_height));
        } else {
                vdp1_cmdt_color_mode1_set(sprite_cmdt, 0x00000000);
                vdp1_cmdt_char_base_set(sprite_cmdt, 0x00000000);
                vdp1_cmdt_char_size_set(sprite_cmdt, 0, 0);
        }

        vdp1_cmdt_char_flip_set(sprite_cmdt, sprite->mirror);

        vdp1_cmdt_gouraud_base_set(sprite_cmdt, 0x00000000);

        if ((sprite->scale == FIX16(1)) && (sprite->angle == FIX16(0))) {
                sprite_cmdt->cmd_vertices[0].x = (int16_t)fix16_int32_to(sprite->x_pos);
                sprite_cmdt->cmd_vertices[0].y = (int16_t)fix16_int32_to(sprite->y_pos);

                vdp1_cmdt_normal_sprite_set(sprite_cmdt);
        } else if (sprite->angle == FIX16(0)) {
                xy[0].x = (int16_t)fix16_int32_to(sprite->x_pos);
                xy[0].y = (int16_t)fix16_int32_to(sprite->y_pos);

                /* The way scale works is by giving the X/Y coordinates of the
                 * top left and bottom right corner of the sprite_t */
                xy[1].x = (int16_t)(fix16_int32_to(fix16_mul(sprite->render_width, sprite->scale) + sprite->x_pos));
                xy[1].y = (int16_t)(fix16_int32_to(fix16_mul(sprite->render_height, sprite->scale) + sprite->y_pos));

                vdp1_cmdt_scaled_sprite_set(sprite_cmdt);
                vdp1_cmdt_vtx_scale_set(sprite_cmdt, xy[0], xy[1]);
        } else {
                /* Offset of top left sprite corner from the origin */
                fix16_t x_offset = -(fix16_mul(sprite->render_width >> 1, sprite->scale));
                fix16_t y_offset = -(fix16_mul(sprite->render_height >> 1, sprite->scale));

                const fix16_t sin = fix16_sin(sprite->angle);
                const fix16_t cos = fix16_cos(sprite->angle);
                const fix16_t scaled_x = sprite->x_pos + fix16_mul(sprite->render_width >> 1, sprite->scale);
                const fix16_t scaled_y = sprite->y_pos + fix16_mul(sprite->render_height >> 1, sprite->scale);

                /* Formula from: <https://gamedev.stackexchange.com/questions/86755/> */
                for (uint32_t i = 0; i < 4; i++) {
                        if (i == 1) {
                                x_offset = -x_offset; /* Upper right */
                        }

                        if (i == 2) {
                                y_offset = -y_offset; /* Lower right */
                        }

                        if (i == 3) {
                                x_offset = -x_offset; /* Lower left */
                        }

                        xy[i].x = (int16_t)fix16_int32_to(fix16_mul(x_offset, cos) -
                            fix16_mul(y_offset, sin) + scaled_x);
                        xy[i].y = (int16_t)fix16_int32_to(fix16_mul(x_offset, sin) +
                            fix16_mul(y_offset, cos) + scaled_y);
                }

                vdp1_cmdt_distorted_sprite_set(sprite_cmdt);
                vdp1_cmdt_vtx_set(sprite_cmdt, xy);
        }

        /* Use the reversed half-word for the sorting value */
        sprite_cmdt->reserved = sprite->sort;
}

void
sprite_make(fix16_t x, fix16_t y, sprite_t *sprite)
{
        (void)memset(sprite, 0, sizeof(sprite_t));

        sprite->x_pos = x;
        sprite->y_pos = y;
        sprite->scale = FIX16(1.0f);
}

void
sprite_list_draw_all(void)
{
        for (uint32_t i = 0; i < SPRITE_LIST_COUNT; i++) {
                sprite_t sprite;

                const int32_t rel_x = _sprites[i].x_pos - fix16_integral(scrolls_x[SCROLL_BG_PLAYFIELD]);
                const int32_t rel_y = _sprites[i].y_pos - scrolls_y[SCROLL_BG_PLAYFIELD];

                /* If sprite is more than 1/2 screen off-screen, don't render
                 * it */
                if (((_sprites[i].options & OPTION_NODISP) == OPTION_NODISP) ||
                    ((rel_x + _sprites[i].render_width) < FIX16(-160)) ||
                    (rel_x > FIX16(480)) ||
                    ((rel_y + _sprites[i].render_height) < FIX16(-112)) ||
                    (rel_y > FIX16(336))) {
                        continue;
                }

                if (_sprites[i].iterate != NULL) {
                        _sprites[i].iterate(&_sprites[i]);
                }

                /* Check again because iterate function may have deleted
                 * sprite_t */
                if ((_sprites[i].options & OPTION_NODISP) != OPTION_NODISP) {
                        (void)memcpy((void *)&sprite, (void *)&_sprites[i], sizeof(sprite));

                        sprite.x_pos = rel_x;
                        sprite.y_pos = rel_y;

                        sprite_draw(&sprite);
                }
        }
}

sprite_t *
sprite_list_next(void)
{
        for (int32_t i = 0; i < SPRITE_LIST_COUNT; i++) {
                if ((_sprites[i].options & OPTION_NODISP) == OPTION_NODISP) {
                        _sprite_list_count++;

                        _sprites[i].index = i;
                        _sprites[i].iterate = NULL;

                        return &_sprites[i];
                }
        }

        return NULL;
}

void
sprite_list_delete(sprite_t *sprite)
{
        assert((sprite >= _start_sprite) && (sprite <= _end_sprite));

        (void)memset(sprite, 0x00, sizeof(sprite_t));

        sprite->options |= OPTION_NODISP;
        sprite->iterate = NULL;

        _sprite_list_count--;
}

void
sprite_list_delete_all(void)
{
        for (uint32_t i = 0; i < SPRITE_LIST_COUNT; i++) {
                sprite_list_delete(&_sprites[i]);
        }

        _sprite_list_count = 0;
}

const loaded_spr_t *
sprite_load(object_id_t object_id)
{
        if (object_id == OBJECT_ID_INVALID) {
                return NULL;
        }

        /* If already loaded, return the loaded SPR structure */
        for (uint32_t i = 0; i < LOADED_SPRS_COUNT; i++) {
                const loaded_spr_t * const loaded_spr =
                    &_sprite_loading.loaded_sprs[i];

                if (loaded_spr->object_id == object_id) {
                        return loaded_spr;
                }
        }

        /* XXX: This should change... possibly an allocated buffer? */
        void * const spr_ptr = (void *)LWRAM(0x0000);

        cd_load_nosize(ASSET_SPR_FILENAME(object_id), spr_ptr);

        const spr_t * const spr = spr_ptr;
        /* XXX: Simplify this heap of mess */
        const spr_sprite_t * const spr_sprites =
            (const spr_sprite_t *)((uintptr_t)spr_ptr +
                sizeof(spr_t));

        /* XXX: Simplify this heap of mess */
        const void * const cg = (void *)((uintptr_t)spr_ptr +
            sizeof(spr_t) +
            (sizeof(spr_sprite_t) * spr->sprite_count));
        const vdp1_clut_t * const palette =
            (vdp1_clut_t *)((uintptr_t)cg + spr->sprite_size);

        /* Unclear why GCC complains about casting an uintptr_t to void pointer */
        void *texture_base = (void *)(uintptr_t)vdp1_hw_vram_texture_alloc(spr->sprite_size);
        vdp1_clut_t * const clut_base = vdp1_hw_vram_clut_alloc();

        (void)memcpy(texture_base, cg, spr->sprite_size);
        (void)memcpy(clut_base, palette, spr->palette_count * sizeof(rgb1555_t));

        loaded_spr_t * const loaded_spr =
            &_sprite_loading.loaded_sprs[_sprite_loading.spr_index];
        _sprite_loading.spr_index++;

        loaded_spr->object_id = object_id;

        loaded_spr->sprites = &_sprite_loading.loaded_spr_sprites[_sprite_loading.spr_sprites_index];
        _sprite_loading.spr_sprites_index += spr->sprite_count;

        loaded_spr->sprite_count = spr->sprite_count;

        for (uint32_t i = 0, vram_offset = 0; i < spr->sprite_count; i++) {
                loaded_spr->sprites[i].texture_base = (vdp1_vram_t)texture_base + vram_offset;
                loaded_spr->sprites[i].clut_base = clut_base;
                loaded_spr->sprites[i].width = spr_sprites[i].width;
                loaded_spr->sprites[i].height = spr_sprites[i].height;

                uint32_t size =
                    spr_sprites[i].width * spr_sprites[i].height;

                switch (spr->palette_count) {
                case 16:
                        size >>= 1;
                        break;
                default:
                        break;
                }

                vram_offset += size;
        }

        return loaded_spr;
}

void
sprite_load_reset_all(void)
{
        _sprite_loading.spr_index = 0;
        _sprite_loading.spr_sprites_index = 0;

        for (uint32_t i = 0; i < LOADED_SPRS_COUNT; i++) {
                _sprite_loading.loaded_sprs[i].object_id = OBJECT_ID_INVALID;
        }
}
