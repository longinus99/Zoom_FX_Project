/*
 * MapSong: 16th-note octave sequencing for the original Zoom MultiStomp.
 * Granular voice adapted from Matujuice's MIT-licensed DualShft.
 */
#include <stdint.h>
#include "../common/zarena.h"

#ifdef __TI_COMPILER_VERSION__
#define MS_PRAGMA_IMPL(p) _Pragma(#p)
#define MS_PRAGMA(p) MS_PRAGMA_IMPL(p)
#define MS_INLINE(fn) MS_PRAGMA(FUNC_ALWAYS_INLINE(fn))
#else
#define MS_PRAGMA(p)
#define MS_INLINE(fn)
#endif

#define RING_SIZE 2048u
#define RING_MASK (RING_SIZE - 1u)
#define HISTORY 1024u
#define STEP_PERIOD (44100u * 15u)
#define FADE_LENGTH 128u
#define MAGIC 0x4d535136u
#define INTRO_BARS 4u
#define LOOP_BARS 4u
#define TRIGGER_LEVEL 0.008f

typedef struct {
    unsigned int magic;
    unsigned int write_pos, filled;
    unsigned int step, phase, fade, from, active;
    unsigned int bar, pattern;
    float phase_down, phase_up;
    unsigned int seal[ZA_NSEAL(12)];
    float ring[RING_SIZE];
} MapSong;
ZA_CHECK(MapSong);

MS_INLINE(step_voice)
static inline unsigned int step_voice(unsigned int pattern, unsigned int step)
{
    return (pattern >> (2u * step)) & 3u;
}

/* Play four A bars once, then loop three A bars and one D bar. */
MS_INLINE(song_pattern)
static inline unsigned int song_pattern(unsigned int bar)
{
    return bar == INTRO_BARS + LOOP_BARS - 1u ? 0x64646646u : 0x64664664u;
}

MS_INLINE(ring_read)
static inline float ring_read(const MapSong *state, unsigned int write_pos, float delay)
{
    unsigned int distance = (unsigned int)(int)delay;
    unsigned int index = (write_pos - distance) & RING_MASK;
    float fraction = delay - (float)(int)distance;
    float sample = distance < state->filled ? state->ring[index] : 0.0f;
    float previous = distance + 1u < state->filled ? state->ring[(index - 1u) & RING_MASK] : 0.0f;
    return sample + fraction * (previous - sample);
}

MS_INLINE(octave_voice)
static inline float octave_voice(const MapSong *state, unsigned int write_pos,
                                 float *phase, float increment)
{
    float p = *phase + increment;
    float q, weight, a, b;
    if (p >= 1.0f) p -= 1.0f;
    if (p < 0.0f) p += 1.0f;
    *phase = p;

    q = (p < 0.5f) ? p + 0.5f : p - 0.5f;
    weight = (p < 0.5f) ? 2.0f * p : 2.0f * (1.0f - p);
    a = ring_read(state, write_pos, 1.0f + 1022.0f * p);
    b = ring_read(state, write_pos, 1.0f + 1022.0f * q);
    return weight * a + (1.0f - weight) * b;
}

MS_INLINE(select_voice)
static inline float select_voice(unsigned int voice, float down, float unity, float up)
{
    return voice == 0u ? down : (voice == 1u ? unity : up);
}

MS_INLINE(process)
static inline void process(MapSong *state, float *left, float *right,
                           unsigned int bpm, float mix, int enabled)
{
    unsigned int i;
    if (state->magic != MAGIC) {
        state->write_pos = 0u;
        state->filled = 0u;
        state->active = 0u;
        state->step = 0u;
        state->phase = 0u;
        state->from = 1u;
        state->fade = 0u;
        state->phase_down = 0.0f;
        state->phase_up = 0.0f;
        state->magic = MAGIC;
    }
    if (!enabled) state->active = 0u;

    for (i = 0; i < 8u; ++i) {
        unsigned int w = state->write_pos;
        unsigned int voice;
        float l = left[i], r = right[i];
        float mono = 0.5f * (l + r);
        float down, up, unity, wet;
        state->ring[w] = mono;
        state->write_pos = (w + 1u) & RING_MASK;
        if (state->filled < HISTORY) ++state->filled;
        if (!enabled) continue;

        if (state->active == 0u) {
            if (l < TRIGGER_LEVEL && l > -TRIGGER_LEVEL &&
                r < TRIGGER_LEVEL && r > -TRIGGER_LEVEL) continue;
            state->active = 1u;
            state->bar = 0u;
            state->step = 0u;
            state->phase = 0u;
            state->pattern = song_pattern(0u);
            state->from = step_voice(state->pattern, 0u);
            state->fade = FADE_LENGTH;
            state->phase_down = 0.0f;
            state->phase_up = 0.0f;
        }
        down = octave_voice(state, w, &state->phase_down, 0.5f / 1022.0f);
        up = octave_voice(state, w, &state->phase_up, -1.0f / 1022.0f);
        unity = ring_read(state, w, 512.0f);
        voice = step_voice(state->pattern, state->step);
        wet = select_voice(voice, down, unity, up);
        if (state->fade < FADE_LENGTH) {
            float old = select_voice(state->from, down, unity, up);
            float blend = (float)(int)state->fade * (1.0f / FADE_LENGTH);
            wet = old + blend * (wet - old);
            ++state->fade;
        }
        left[i] = l + mix * (wet - l);
        right[i] = r + mix * (wet - r);

        state->phase += bpm;
        if (state->phase >= STEP_PERIOD) {
            state->phase -= STEP_PERIOD;
            state->from = voice;
            state->step = (state->step + 1u) & 15u;
            if (state->step == 0u) {
                ++state->bar;
                if (state->bar == INTRO_BARS + LOOP_BARS) state->bar = INTRO_BARS;
                state->pattern = song_pattern(state->bar);
            }
            state->fade = 0u;
        }
    }
}

#ifndef MAPSONG_HOST_TEST
#include "mapsong_params.h"

#define ZDL_PTR(type, word) ((type)(uintptr_t)(word))

MS_PRAGMA(CODE_SECTION(Fx_DLY_MapSong, ".audio"))
void Fx_DLY_MapSong(unsigned int *ctx)
{
    float *params = ZDL_PTR(float *, ctx[1]);
    float *audio = ZDL_PTR(float *, ctx[5]);
    unsigned int *source = ZDL_PTR(unsigned int *, ctx[12]);
    unsigned int *target = ZDL_PTR(unsigned int *, *ZDL_PTR(unsigned int *, ctx[11]));
    volatile unsigned int *desc = ZDL_PTR(volatile unsigned int *, ctx[3]);
    uintptr_t base, end;
    float bpm, mix;
    MapSong *state;

    *target = *source;
    if (!desc || !params || !audio) return;
    base = (uintptr_t)desc[0];
    end = (uintptr_t)desc[1];
    if (!base || (base & 3u) || end <= base ||
        end - base < sizeof(MapSong) || desc[2] < end - base) return;

    bpm = 100.0f * params[MAPSONG_BPM_SLOT];
    if (!(bpm >= 40.0f && bpm <= 240.01f)) bpm = MAPSONG_BPM_UI_DEFAULT;
    mix = params[MAPSONG_MIX_SLOT];
    if (!(mix >= 0.0f && mix <= 1.0f)) mix = 1.0f;

    state = (MapSong *)base;
    za_unseal(state, ZA_HDR(MapSong), state->seal);
    process(state, audio, audio + 8, (unsigned int)(int)(bpm + 0.5f), mix, params[0] >= 0.5f);
    za_seal(state, ZA_HDR(MapSong), state->seal);
}
#endif
