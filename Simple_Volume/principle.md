# Vol

1-knob Volume plugin for Zoom MS-70CDR.

## Signal Flow

```
기타 입력
    ↓
fxBuf (ctx[5])
    ↓
params[0] == 0.0 → return (fxBuf 그대로, 원음 통과)
    ↓
lvl = params[5] * params[4] * params[0]
      (노브값)   (1/max)     (on/off=1.0)
    ↓
fxBuf[0~7]   = fxBuf[0~7]   * lvl  (Left 8샘플)
fxBuf[8~15]  = fxBuf[8~15]  * lvl  (Right 8샘플)
```

## Parameters

| Name  | Min | Max | Default |
|-------|-----|-----|---------|
| Level | 0   | 150 | 100     |

## Files

| File            | Purpose                        |
|-----------------|--------------------------------|
| `vol.c`         | Audio function (`Fx_FLT_VOL`)  |
| `manifest.json` | Effect metadata, knob descriptor |
| `build.py`      | `cl6x` → `linker.link()` → `Vol.ZDL` |
