기타 입력
    ↓
fxBuf (ctx[5])
    ↓
params[0] == 0.0 → return (fxBuf 그대로)
    ↓
lvl = params[5] * params[4] * params[0]
      (노브값)   (1/max)     (on/off=1.0)
    ↓
fxBuf[0~7]   = fxBuf[0~7]   * lvl
fxBuf[8~15]  = fxBuf[8~15]  * lvl



