# Runtime Reverse Engineering Status

## Confirmed Runtime Findings

### Audio Buffer

`ctx[5]`는 실제 오디오 버퍼 포인터이다.

다음 방식으로 출력 오디오를 직접 수정할 수 있음이 확인되었다.

```c
float *fxBuf=ZDL_PTR(float *,ctx[5]);
fxBuf[i]=sample;
```

직접 overwrite 시 실제 오디오 출력이 변경되는 것이 확인되었다.

---

### `ctx[3]`는 직접 state struct가 아님

초기에는 다음과 같이 `ctx[3]`를 직접적인 writable state struct pointer로 가정하였다.

```c
PitchState *st=
    ZDL_PTR(PitchState *,ctx[3]);
```

그러나 이는 잘못된 접근임이 확인되었다.

특히 다음과 같은 large local buffer 선언 시:

```c
float buffer[4096];
```

다음 문제가 발생하였다.

* boot freeze
* watchdog reset
* DSP instability
* high-frequency oscillation

즉 `ctx[3]`는 직접적인 사용자 state 영역이 아니며, 단순 struct pointer로 사용하면 안 된다.

---

### Descriptor 기반 RAM 접근

`ctx[3]`는 descriptor pointer처럼 동작함이 확인되었다.

다음 접근 방식이 정상 동작하였다.

```c
unsigned int *ramDesc=
    ZDL_PTR(unsigned int *,ctx[3]);

float *delay=
    ZDL_PTR(float *,ramDesc[0]);
```

특히 `ramDesc[0]`는 다음 특성을 가진 실제 DSP RAM 포인터로 확인되었다.

* writable
* readable
* contiguous
* large DSP RAM

이를 통해 stock delay / reverb / pitch 계열 effect들이 내부적으로 external DSP RAM 기반으로 동작할 가능성이 매우 높아졌다.

---

### Delay RAM Validation

`ramDesc[0]` 기반으로 다음 동작이 정상 수행됨이 확인되었다.

* sequential write
* sequential read
* circular buffer access
* delayed playback

정상 동작 예시:

```c
delay[wp]=in;
wet=delay[rp];
```

RAM write/read 수행 시:

* watchdog reset 없음
* crash 없음
* noise 없음
* 정상 delayed playback 발생

---

### Circular Buffer Validation

`ramDesc[0]` 위에 구현한 fixed-size circular delay buffer가 정상 동작하였다.

다음 코드가 안정적으로 동작함이 확인되었다.

```c
rp=(wp+4096-4000)&4095;
```

결과:

* boot freeze 없음
* watchdog reset 없음
* audible delayed playback 확인
* long delay 정상 동작

이를 통해 실제 DSP RAM 위에 circular buffer 구현이 가능함이 확인되었다.

---

## Confirmed Working Delay Test Code

```c
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

    float mix=params[5]*params[4];

    if(mix<0.0f)mix=0.0f;
    if(mix>1.0f)mix=1.0f;

    int i;

    for(i=0;i<8;i++){

        float in=fxBuf[i];

        delay[wp]=in;

        unsigned int rp=
            (wp+4096-4000)&4095;

        float wet=delay[rp];

        fxBuf[i]=
            (in*(1.0f-mix))+
            (wet*mix);

        wp=(wp+1)&4095;
    }
}
```

---

## Current Confirmed Capabilities

현재 runtime 환경에서 다음 구현 가능성이 확인되었다.

* large delay buffer
* circular delay processing
* long delayed playback
* DSP RAM read/write access
* granular processing 기반
* future pitch-shift / harmonizer 구현 기반 확보
