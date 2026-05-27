/*
 * vol.c — 1-knob Volume plugin for Zoom MS-70CDR
 * Based on gain.c from the v2 builder pipeline.
 */

#include <stdint.h>

#pragma CODE_SECTION(Fx_FLT_VOL, ".audio")

#define ZDL_PTR(type, word)  ((type)(uintptr_t)(word))

void Fx_FLT_VOL(unsigned int *ctx)
{
    float        *params   = ZDL_PTR(float        *, ctx[1]);
    float        *fxBuf    = ZDL_PTR(float        *, ctx[5]);

    unsigned int *magicSrc = ZDL_PTR(unsigned int *, ctx[12]);
    unsigned int *magicDst = ZDL_PTR(unsigned int *, *(unsigned int *)ZDL_PTR(unsigned int *, ctx[11]));
    *magicDst = *magicSrc;

    if (params[0] == 0.0f) {
        return;
    }

    /* gain.c original formula: params[5] * params[4] * params[0]
     * params[0] = on/off, params[4] = 1/max, params[5] = raw knob value */
    float lvl = params[5] * params[4] * params[0];

    int i;
    for (i = 0; i < 8; i++) {
        fxBuf[i]     = fxBuf[i] * lvl;
    }
}
