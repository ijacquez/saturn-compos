#include <stdarg.h>


#include "cd.h"
#include "print.h"
#include "vdp1_hw.h"

#define COLS            (40)
#define ROWS            (28)
#define FONT_X          (8)
#define FONT_Y          (8)
#define FONT_TILE_SIZE  ((FONT_X * FONT_Y) / 2)

static char _buffer[(ROWS * COLS) + 1] __aligned(4);

static vdp1_cmdt_t _template_cmdt;
static vdp1_vram_t _texture_base;
static vdp1_clut_t *_clut_base;

static void _print_clear(void);

typedef struct font {
        char signature[4];
        uint16_t byte_size;
} __packed font_t;

void
print_init(void)
{
        static const vdp1_cmdt_draw_mode_t draw_mode = {
                .raw                  = 0x0000,
                .trans_pixel_disable  = false,
                .pre_clipping_disable = true,
                .end_code_disable     = true,
                .color_mode           = VDP1_CMDT_CM_CLUT_16
        };

        font_t *const font = (font_t *)LWRAM(0x00000000);

        cd_load_nosize("FONT.FON", font);

        void * const cg = (void *)((uintptr_t)font + sizeof(font_t));
        void * const palette = (void *)((uintptr_t)cg + font->byte_size);

        _texture_base = vdp1_hw_vram_texture_alloc(font->byte_size);
        _clut_base = vdp1_hw_vram_clut_alloc();

        (void)memcpy((void *)_texture_base, cg, font->byte_size);
        /* XXX: Hard coded */
        (void)memcpy(_clut_base, palette, 16 * sizeof(rgb1555_t));

        vdp1_cmdt_normal_sprite_set(&_template_cmdt);
        vdp1_cmdt_draw_mode_set(&_template_cmdt, draw_mode);
        vdp1_cmdt_color_mode1_set(&_template_cmdt, (vdp1_vram_t)_clut_base);
        vdp1_cmdt_char_size_set(&_template_cmdt, FONT_X, FONT_Y);
        vdp1_cmdt_gouraud_base_set(&_template_cmdt, 0x00000000);

        _template_cmdt.reserved = -1000;

        _print_clear();
}

void
print_buffer(uint16_t start_x, uint16_t start_y, const char *__restrict fmt, ...)
{
        va_list args;

        va_start(args, fmt);
        (void)vsprintf(_buffer, fmt, args);
        va_end(args);

        uint16_t offset_x;
        offset_x = start_x;

        uint16_t offset_y;
        offset_y = start_y;

        for (uint32_t i = 0; _buffer[i] != '\0'; i++, offset_x++) {
                if (_buffer[i] == ' ') {
                        continue;
                }

                vdp1_cmdt_t * const cmdt = vdp1_hw_cmdt_list_next();

                (void)memcpy(cmdt, &_template_cmdt, sizeof(vdp1_cmdt_t));

                const uint16_t tile = _buffer[i] - ' ';
                const vdp1_vram_t texture_base = _texture_base + (tile * FONT_TILE_SIZE);

                vdp1_cmdt_char_base_set(cmdt, (vdp1_vram_t)texture_base);

                int16_vec2_t xy;

                cmdt->cmd_vertices[0].x = offset_x * FONT_X;
                cmdt->cmd_vertices[0].y = offset_y * FONT_Y;
        }
}

static void
_print_clear(void)
{
        (void)memset(_buffer, '\0', COLS * ROWS);
}
