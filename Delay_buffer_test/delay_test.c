#include <stdint.h>
#pragma CODE_SECTION(Fx_FLT_PitchSHFT,".audio")
#define ZDL_PTR(type,word)((type)(uintptr_t)(word))

static unsigned int wp=0;

void Fx_FLT_PitchSHFT(unsigned int *ctx)
{
    float *params=ZDL_PTR(float *,ctx[1]);
    float *fxBuf=ZDL_PTR(float *,ctx[5]);

    unsigned int *ramDesc=
        ZDL_PTR(unsigned int *,ctx[3]);

    float *delay=
        ZDL_PTR(float *,ramDesc[0]);

    unsigned int *magicSrc=
        ZDL_PTR(unsigned int *,ctx[12]);

    unsigned int *magicDst=
        ZDL_PTR(
            unsigned int *,
            *(unsigned int *)
            ZDL_PTR(unsigned int *,ctx[11])
        );

    *magicDst=*magicSrc;

    if(params[0]==0.0f){
        return;
    }

    if(delay==0){
        return;
    }

    int i;

    for(i=0;i<8;i++){

        float in=fxBuf[i];

        delay[wp]=in;

        unsigned int rp=
            (wp+4096-4000)&4095;

        float wet=delay[rp];

        fxBuf[i]=wet;

        wp=(wp+1)&4095;
    }
}

