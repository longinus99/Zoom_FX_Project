/*
 * MapSeq: 16th-note octave sequencing for the original Zoom MultiStomp.
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
#define MAGIC 0x4d535132u
#define DEFAULT_PATTERN 0x64664664u

typedef struct {
    unsigned int magic;
    unsigned int write_pos, filled;
    unsigned int step, phase, fade, from, active;
    float phase_down, phase_up;
    unsigned int seal[ZA_NSEAL(10)];
    float ring[RING_SIZE];
} MapSeq;
ZA_CHECK(MapSeq);

MS_INLINE(step_voice)
static inline unsigned int step_voice(unsigned int pattern, unsigned int step)
{
    return (pattern >> (2u * step)) & 3u;
}

MS_INLINE(first_voice)
static inline unsigned int first_voice(unsigned int pair)
{
    return pair < 3u ? 0u : (pair < 6u ? 1u : 2u);
}

MS_INLINE(ring_read)
static inline float ring_read(const MapSeq *state, unsigned int write_pos, float delay)
{
    unsigned int distance = (unsigned int)(int)delay;
    unsigned int index = (write_pos - distance) & RING_MASK;
    float fraction = delay - (float)(int)distance;
    float sample = state->ring[index];
    return sample + fraction * (state->ring[(index - 1u) & RING_MASK] - sample);
}

MS_INLINE(octave_voice)
static inline float octave_voice(const MapSeq *state, unsigned int write_pos,
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
static inline void process(MapSeq *state, float *left, float *right,
                           unsigned int bpm, unsigned int pattern, int enabled)
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
        if (state->filled < HISTORY) {
            ++state->filled;
            continue;
        }
        if (!enabled) continue;

        if (!state->active) {
            state->active = 1u;
            state->step = 0u;
            state->phase = 0u;
            state->from = 1u;
            state->fade = 0u;
        }
        down = octave_voice(state, w, &state->phase_down, 0.5f / 1022.0f);
        up = octave_voice(state, w, &state->phase_up, -1.0f / 1022.0f);
        unity = state->ring[(w - 512u) & RING_MASK];
        voice = step_voice(pattern, state->step);
        wet = select_voice(voice, down, unity, up);
        if (state->fade < FADE_LENGTH) {
            float old = select_voice(state->from, down, unity, up);
            float blend = (float)(int)state->fade * (1.0f / FADE_LENGTH);
            wet = old + blend * (wet - old);
            ++state->fade;
        }
        left[i] = wet;
        right[i] = wet;

        state->phase += bpm;
        if (state->phase >= STEP_PERIOD) {
            state->phase -= STEP_PERIOD;
            state->from = voice;
            state->step = (state->step + 1u) & 15u;
            state->fade = 0u;
        }
    }
}

#define PAIR_LABEL(fn) \
int fn(unsigned int value, char *out) \
{ \
    unsigned int first, second; \
    if (value > 8u) value = 4u; \
    first = first_voice(value); \
    second = value - 3u * first; \
    out[0] = first == 0u ? '-' : (first == 1u ? '0' : '+'); \
    out[1] = second == 0u ? '-' : (second == 1u ? '0' : '+'); \
    out[2] = 0; \
    return 2; \
}

PAIR_LABEL(ZDL_GetLabel_1)
PAIR_LABEL(ZDL_GetLabel_2)
PAIR_LABEL(ZDL_GetLabel_3)
PAIR_LABEL(ZDL_GetLabel_4)
PAIR_LABEL(ZDL_GetLabel_5)
PAIR_LABEL(ZDL_GetLabel_6)
PAIR_LABEL(ZDL_GetLabel_7)
PAIR_LABEL(ZDL_GetLabel_8)

MS_INLINE(read_pattern)
static inline unsigned int read_pattern(const float *pairs)
{
    unsigned int pattern = 0u, i;
    for (i = 0u; i < 8u; ++i) {
        float raw = pairs[i];
        unsigned int pair, first, second;
        if (!(raw >= 0.0f && raw <= 0.085f)) {
            pattern |= DEFAULT_PATTERN & (15u << (4u * i));
            continue;
        }
        pair = (unsigned int)(int)(100.0f * raw + 0.5f);
        if (pair > 8u) pair = 8u;
        first = first_voice(pair);
        second = pair - 3u * first;
        pattern |= (first | (second << 2u)) << (4u * i);
    }
    return pattern;
}

#ifndef MAPSEQ_HOST_TEST
#include "mapseq_params.h"

#define ZDL_PTR(type, word) ((type)(uintptr_t)(word))

MS_PRAGMA(CODE_SECTION(Fx_DLY_MapSeq, ".audio"))
void Fx_DLY_MapSeq(unsigned int *ctx)
{
    float *params = ZDL_PTR(float *, ctx[1]);
    float *audio = ZDL_PTR(float *, ctx[5]);
    unsigned int *source = ZDL_PTR(unsigned int *, ctx[12]);
    unsigned int *target = ZDL_PTR(unsigned int *, *ZDL_PTR(unsigned int *, ctx[11]));
    volatile unsigned int *desc = ZDL_PTR(volatile unsigned int *, ctx[3]);
    uintptr_t base, end;
    float bpm;
    unsigned int pattern;
    MapSeq *state;

    *target = *source;
    if (!desc || !params || !audio) return;
    base = (uintptr_t)desc[0];
    end = (uintptr_t)desc[1];
    if (!base || (base & 3u) || end <= base ||
        end - base < sizeof(MapSeq) || desc[2] < end - base) return;

    bpm = 100.0f * params[MAPSEQ_BPM_SLOT];
    if (!(bpm >= 40.0f && bpm <= 240.01f)) bpm = MAPSEQ_BPM_UI_DEFAULT;
    pattern = read_pattern(params + MAPSEQ_S0102_SLOT);

    state = (MapSeq *)base;
    za_unseal(state, ZA_HDR(MapSeq), state->seal);
    process(state, audio, audio + 8, (unsigned int)(int)(bpm + 0.5f), pattern, params[0] >= 0.5f);
    za_seal(state, ZA_HDR(MapSeq), state->seal);
}
#endif
