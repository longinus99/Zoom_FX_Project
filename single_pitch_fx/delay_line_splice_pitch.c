#include <stdint.h>

#pragma CODE_SECTION(Fx_FLT_PitchSHFT, ".audio")

#define ZDL_PTR(type, word) ((type)(uintptr_t)(word))

#define PITCH_MAGIC 0x5055324fu

#define BUF_SIZE 4096
#define BUF_MASK 4095

#define READ_STEP 2.0f

#define RESET_BACK 3328
#define SWITCH_DIST 512

#define XFADE_LEN 192
#define XFADE_INC 0.005208333f

typedef struct
{
    uint32_t magic;

    unsigned int wp;

    float rpA;
    float rpB;

    int activeA;
    int overlap;
    int overlapPos;

    float outSmoothL;
    float outSmoothR;
} PitchState;

static inline PitchState *get_state(unsigned int *ctx)
{
    unsigned int *ramDesc = ZDL_PTR(unsigned int *, ctx[3]);

    if (ramDesc == 0)
        return 0;

    if (ramDesc[0] == 0)
        return 0;

    return ZDL_PTR(PitchState *, ramDesc[0]);
}

static inline float *get_delay(unsigned int *ctx)
{
    unsigned int *ramDesc = ZDL_PTR(unsigned int *, ctx[3]);

    if (ramDesc == 0)
        return 0;

    if (ramDesc[0] == 0)
        return 0;

    return ZDL_PTR(float *, ramDesc[0] + 256);
}

static inline float interp(float *delay, float pos)
{
    int i0 = (int)pos;
    int i1 = (i0 + 1) & BUF_MASK;

    float f = pos - (float)i0;

    i0 = i0 & BUF_MASK;

    return delay[i0] + ((delay[i1] - delay[i0]) * f);
}

static inline float smoothstep(float x)
{
    if (x < 0.0f)
        x = 0.0f;

    if (x > 1.0f)
        x = 1.0f;

    return x * x * (3.0f - (2.0f * x));
}

static inline float soft_clip(float x)
{
    if (x > 1.2f)
        return 1.2f;

    if (x < -1.2f)
        return -1.2f;

    return x;
}

void Fx_FLT_PitchSHFT(unsigned int *ctx)
{
    float *params = ZDL_PTR(float *, ctx[1]);
    float *fxBuf = ZDL_PTR(float *, ctx[5]);

    unsigned int *magicSrc = ZDL_PTR(unsigned int *, ctx[12]);
    unsigned int *magicDst = ZDL_PTR(unsigned int *, *(unsigned int *)ZDL_PTR(unsigned int *, ctx[11]));

    *magicDst = *magicSrc;

    if (params[0] == 0.0f)
        return;

    PitchState *st = get_state(ctx);
    float *delay = get_delay(ctx);

    if (st == 0)
        return;

    if (delay == 0)
        return;

    if (st->magic != PITCH_MAGIC)
    {
        st->magic = PITCH_MAGIC;

        st->wp = 0;

        st->rpA = 0.0f;
        st->rpB = 2048.0f;

        st->activeA = 1;
        st->overlap = 0;
        st->overlapPos = 0;

        st->outSmoothL = 0.0f;
        st->outSmoothR = 0.0f;

        int n;

        for (n = 0; n < BUF_SIZE; n++)
            delay[n] = 0.0f;
    }

    int i;

    for (i = 0; i < 8; i++)
    {
        float inL = fxBuf[i];
        float inR = fxBuf[i + 8];
        float in = (inL + inR) * 0.5f;

        unsigned int wp = st->wp & BUF_MASK;

        delay[wp] = in;

        st->rpA += READ_STEP;

        if (st->rpA >= 4096.0f)
            st->rpA -= 4096.0f;

        st->rpB += READ_STEP;

        if (st->rpB >= 4096.0f)
            st->rpB -= 4096.0f;

        int a0 = (int)st->rpA;
        int b0 = (int)st->rpB;

        unsigned int distA = (wp - (unsigned int)a0) & BUF_MASK;
        unsigned int distB = (wp - (unsigned int)b0) & BUF_MASK;

        if (!st->overlap)
        {
            if (st->activeA)
            {
                if (distA < SWITCH_DIST)
                {
                    st->rpB = (float)((wp + BUF_SIZE - RESET_BACK) & BUF_MASK);
                    st->overlap = 1;
                    st->overlapPos = 0;
                }
            }
            else
            {
                if (distB < SWITCH_DIST)
                {
                    st->rpA = (float)((wp + BUF_SIZE - RESET_BACK) & BUF_MASK);
                    st->overlap = 1;
                    st->overlapPos = 0;
                }
            }
        }

        float a = interp(delay, st->rpA);
        float b = interp(delay, st->rpB);

        float wet;

        if (!st->overlap)
        {
            if (st->activeA)
                wet = a;
            else
                wet = b;
        }
        else
        {
            float t = (float)st->overlapPos * XFADE_INC;

            t = smoothstep(t);

            float wa = 1.0f - t;
            float wb = t;

            if (st->activeA)
                wet = (a * wa) + (b * wb);
            else
                wet = (b * wa) + (a * wb);

            st->overlapPos++;

            if (st->overlapPos >= XFADE_LEN)
            {
                st->overlap = 0;

                if (st->activeA)
                    st->activeA = 0;
                else
                    st->activeA = 1;
            }
        }

        wet = soft_clip(wet);

        st->outSmoothL = st->outSmoothL + ((wet - st->outSmoothL) * 0.94f);
        st->outSmoothR = st->outSmoothR + ((wet - st->outSmoothR) * 0.94f);

        fxBuf[i] = st->outSmoothL;
        fxBuf[i + 8] = st->outSmoothR;

        st->wp = (wp + 1) & BUF_MASK;
    }
}


