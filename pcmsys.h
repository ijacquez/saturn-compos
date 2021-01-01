#ifndef PCMSYS_H
#define PCMSYS_H

#include <stdint.h>

// Address mapping shorthand
#define MAP_TO_SCSP(sh2map_snd_adr) ((sh2map_snd_adr - SNDRAM))

// No touchy sound RAM start!
#define SNDRAM  (631242752)
// 1KiB here is reserved for interrupts
#define SNDPRG  (SNDRAM + 0x408)
// Also the end of sound RAM
#define PCMEND  (SNDRAM + 0x7F000)

#define PCM_ALT_LOOP    (3)
#define PCM_RVS_LOOP    (2)
#define PCM_FWD_LOOP    (1)
#define PCM_VOLATILE    (0)
#define PCM_PROTECTED   (-1)
#define PCM_SEMI        (-2)

#define PCM_SYS_REGION  (0) // 0 for NTSC, 1 for PAL

#define PCM_PAN_LEFT    (1 << 4)
#define PCM_PAN_RIGHT   (0)

typedef struct pcm_ctrl {
        int8_t loop_type;         // [0,1,2,3] No loop, normal loop, reverse
                                  // loop, alternating loop
        uint8_t bit_depth;        // 0 or 1, boolean
        uint16_t hi_addr_bits;    // Bits 19-16 of...
        uint16_t lo_addr_bits;    // Two 16-bit chunks that when combined, form
                                  // the start address of the sound.
        uint16_t lsa;             // The # of samples forward from the start
                                  // address to return to after loop.
        uint16_t play_size;       // The # of samples to play before the sound
                                  // shall loop. **Otherwise used as the length
                                  // of the sound.** Do not leave at 0! 8 bit
                                  // PCM is 1 byte per sample. 16 bit PCM is 2
                                  // bytes per sample. Therefore an 8bit PCM is
                                  // a maximum of 64KB, and 16bit is 128KB.
        uint16_t pitch_word;      // The OCT & FNS word to use in the ICSR, verbatim.
        uint8_t pan;              // Direct pan setting
        uint8_t volume;           // Direct volume setting
        uint16_t bytes_per_blank; // Bytes the PCM will play every time the
                                  // driver is run (vblank)
        uint8_t sh2_permit;       // Does the SH2 permit this command? If TRUE,
                                  // run the command. If FALSE, key its ICSR
                                  // OFF.
        int8_t icsr_target;       // Which explicit ICSR is this to land in? Can
                                  // be controlled by SH2 or by driver.
} pcm_ctrl_t;

typedef struct {
        uint16_t start; // System Start boolean
        uint16_t dt_ms; // Delta time supplied by SH2 in miliseconds
        pcm_ctrl_t *pcm_ctrl;
} driver_ctl_t;

extern volatile driver_ctl_t *m68k_com;
extern volatile uint32_t *scsp_load;
extern volatile uint16_t *master_volume;
extern int16_t pcm_count;

int16_t pcmsys_load_16bit_pcm(char *filename, int sample_rate);
int16_t pcmsys_load_8bit_pcm(char *filename, int sample_rate);
void pcmsys_load_driver(void);

void pcm_play(int16_t pcm_id, int8_t ctrl_type, int8_t volume);
void pcm_parameter_change(int16_t pcm_id, int8_t volume, int8_t pan);
void pcm_cease(int16_t pcm_id);

#endif /* !PCMSYS_H */
