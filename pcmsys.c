#include <yaul.h>

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "cd.h"
#include "pcmsys.h"

static const int32_t _logtbl[] = {
        /* 0 */   0,
        /* 1 */   1,
        /* 2 */   2, 2,
        /* 4 */   3, 3, 3, 3,
        /* 8 */   4, 4, 4, 4, 4, 4, 4, 4,
        /* 16 */  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        /* 32 */  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        /* 64 */  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        /* 128 */ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

#define PCM_MSK1(a)     ((a) & 0x0001)
#define PCM_MSK3(a)     ((a) & 0x0007)
#define PCM_MSK4(a)     ((a) & 0x000F)
#define PCM_MSK5(a)     ((a) & 0x001F)
#define PCM_MSK10(a)    ((a) & 0x03FF)

#define PCM_SCSP_FREQUENCY (44100L)

#define PCM_CALC_OCT(smpling_rate)                                             \
        ((int)_logtbl[PCM_SCSP_FREQUENCY / ((smpling_rate) + 1)])

#define PCM_CALC_SHIFT_FREQ(oct)                                               \
        (PCM_SCSP_FREQUENCY >> (oct))

#define PCM_CALC_FNS(smpling_rate, shift_freq)                                 \
        ((((smpling_rate) - (shift_freq)) << 10) / (shift_freq))

#define PCM_SET_PITCH_WORD(oct, fns)                                           \
        ((int)((PCM_MSK4(-(oct)) << 11) | PCM_MSK10(fns)))

#define DRV_SYS_END (10 * 1024) // System defined safe end of driver's address space

volatile driver_ctl_t *m68k_com  = (volatile driver_ctl_t *)(SNDPRG + DRV_SYS_END);
volatile uint32_t *scsp_load     = (volatile uint32_t *)(0x408 + DRV_SYS_END + 0x20); // Local loading address for sound data, is DRV_SYS_END ahead of the SNDPRG, and ahead of the communication data
volatile uint16_t *master_volume = (volatile uint16_t *)(SNDRAM + 0x100400);
int16_t pcm_count = 0;

void
load_driver_binary(char *filename, void *buffer)
{
        const int32_t file_size = cd_load_nosize(filename, buffer);

        // The immediacy of these commands is important. As per SEGA technical
        // bulletin 51, the Sound CPU is not to be turned off for more than 0.5
        // seconds.
        smpc_smc_call(SMPC_SMC_SNDOFF);

        *master_volume = 0x020F; // Set max master volume + 4mbit memory // Very important, douglath

        (void)memcpy((void *)SNDRAM, buffer, file_size);

        smpc_smc_call(SMPC_SMC_SNDON);
}

void
pcmsys_load_driver(void)
{
        // Make sure SCSP is set to 512k mode
        *(volatile uint8_t *)(0x25B00400) = 0x02;

        // Clear Sound RAM
        for (int i = 0; i < 0x80000; i += 4) {
                *(volatile uint32_t *)(SNDRAM + i) = 0x00000000;
        }

        const uint32_t sddrv_size = cd_byte_size_get("SDDRV.BIN");
        void * const sddrv_buffer = malloc(sddrv_size);
        assert(sddrv_buffer != NULL);

        /* Copy driver over */
        load_driver_binary("SDDRV.BIN", sddrv_buffer);
}

int16_t
calculate_bytes_per_blank(int sample_rate, int is_8bit, int is_pal)
{
        int frameCount = (is_pal) ? 50 : 60;
        int sampleSize = (is_8bit) ? 8 : 16;
        return ((sample_rate * sampleSize) >> 3) / frameCount;
}

int16_t
pcmsys_load_16bit_pcm(char *filename, int sample_rate)
{
        if ((int)scsp_load > 0x7F800) {
                return -1; // Illegal PCM data address, exit
        }

        int octr;
        int shiftr;
        int fnsr;
        int32_t file_size = cd_load_nosize(filename, (void *)((uint32_t)scsp_load + SNDRAM));

        // Make sure end of the sample is word aligned
        file_size += ((uint32_t)file_size & 1) ? 1 : 0;
        file_size += ((uint32_t)file_size & 3) ? 2 : 0;

        octr = PCM_CALC_OCT(sample_rate);
        shiftr = PCM_CALC_SHIFT_FREQ(octr);
        fnsr = PCM_CALC_FNS(sample_rate, shiftr);

        m68k_com->pcm_ctrl[pcm_count].hi_addr_bits = (uint16_t)((uint32_t)scsp_load >> 16);
        m68k_com->pcm_ctrl[pcm_count].lo_addr_bits = (uint16_t)((uint32_t)scsp_load & 0xFFFF);

        m68k_com->pcm_ctrl[pcm_count].pitch_word = PCM_SET_PITCH_WORD(octr, fnsr);
        m68k_com->pcm_ctrl[pcm_count].play_size = (file_size >> 1);
        m68k_com->pcm_ctrl[pcm_count].bytes_per_blank = calculate_bytes_per_blank(sample_rate, 0, PCM_SYS_REGION); // Initialize as max volume
        m68k_com->pcm_ctrl[pcm_count].bit_depth = 0; // Select 16-bit
        m68k_com->pcm_ctrl[pcm_count].loop_type = 0; // Initialize as non-looping
        m68k_com->pcm_ctrl[pcm_count].volume = 7;    // Initialize as max volume


        pcm_count++; // Increment pcm #
        scsp_load = (uint32_t *)((uint32_t )scsp_load + file_size);
        return (pcm_count - 1); // Return the PCM # this sound recieved
}

int16_t
pcmsys_load_8bit_pcm(char *filename, int sample_rate)
{
        if ((int)scsp_load > 0x7F800) {
                return -1; // Illegal PCM data address, exit
        }

        int octr;
        int shiftr;
        int fnsr;

        int32_t file_size = cd_load_nosize(filename, (void *)((uint32_t)scsp_load + SNDRAM));
        file_size += ((uint32_t)file_size & 1) ? 1 : 0;
        file_size += ((uint32_t)file_size & 3) ? 2 : 0;

        octr = PCM_CALC_OCT(sample_rate);
        shiftr = PCM_CALC_SHIFT_FREQ(octr);
        fnsr = PCM_CALC_FNS(sample_rate, shiftr);

        m68k_com->pcm_ctrl[pcm_count].hi_addr_bits = (uint16_t)( (uint32_t)scsp_load >> 16);
        m68k_com->pcm_ctrl[pcm_count].lo_addr_bits = (uint16_t)( (uint32_t)scsp_load & 0xFFFF);

        m68k_com->pcm_ctrl[pcm_count].pitch_word = PCM_SET_PITCH_WORD(octr, fnsr);
        m68k_com->pcm_ctrl[pcm_count].play_size = (file_size);
        m68k_com->pcm_ctrl[pcm_count].bytes_per_blank = calculate_bytes_per_blank(sample_rate, 1, PCM_SYS_REGION); // Initialize as max volume
        m68k_com->pcm_ctrl[pcm_count].bit_depth = 1; // Select 8-bit
        m68k_com->pcm_ctrl[pcm_count].loop_type = 0; // Initialize as non-looping
        m68k_com->pcm_ctrl[pcm_count].volume = 7; // Initialize as max volume

        pcm_count++; // Increment pcm #

        scsp_load = (uint32_t *)((uint32_t )scsp_load + file_size);

        return (pcm_count - 1); // Return the PCM # this sound recieved
}

void
pcm_play(int16_t pcm_id, int8_t ctrl_type, int8_t volume)
{
        m68k_com->pcm_ctrl[pcm_id].sh2_permit = 1;
        m68k_com->pcm_ctrl[pcm_id].volume = volume;
        m68k_com->pcm_ctrl[pcm_id].loop_type = ctrl_type;
}

void
pcm_parameter_change(int16_t pcm_id, int8_t volume, int8_t pan)
{
        m68k_com->pcm_ctrl[pcm_id].volume = volume;
        m68k_com->pcm_ctrl[pcm_id].pan = pan;
}

void
pcm_cease(int16_t pcm_id)
{
        // If it is a volatile or protected sound, the expected control method
        // is to mute the sound and let it end itself
        if (m68k_com->pcm_ctrl[pcm_id].loop_type <= 0) {
                // Protected sounds have a permission state of "until they end"
                m68k_com->pcm_ctrl[pcm_id].volume = 0;
        } else {
                // If it is a looping sound, the control method is to command it
                // to stop
                m68k_com->pcm_ctrl[pcm_id].sh2_permit = 0;
        }
}
