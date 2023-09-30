#ifndef SOUND_H
#define SOUND_H

#define SOUND_EXPLOSION (0)
#define SOUND_JUMP      (1)
#define SOUND_DASH      (2)

// Must be called after cd_init
void sound_init(void);
// Play an audio track
void sound_cdda(int track);
// Play a pcm sound
void sound_play(short num);

void sound_vblank_in(void);

#endif /* !SOUND_H */
