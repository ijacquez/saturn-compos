#ifndef VDP1_HW_H
#define VDP1_HW_H

#include <yaul.h>

#include <stdint.h>

void vdp1_init(void);

void *vdp1_hw_vram_texture_base(void);
vdp1_clut_t *vdp1_hw_vram_clut_base(void);

vdp1_vram_t vdp1_hw_vram_texture_alloc(uint32_t bytes);
vdp1_clut_t *vdp1_hw_vram_clut_alloc(void);

void vdp1_hw_vram_texture_free(void);
void vdp1_hw_vram_clut_free(void);

void vdp1_hw_cmdt_list_start(void);
void vdp1_hw_cmdt_list_finish(void);
vdp1_cmdt_t *vdp1_hw_cmdt_list_next(void);

#endif /* VDP1_HW_H */
