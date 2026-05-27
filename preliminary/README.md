새로운 FX 를 만들기위한 하드웨/소프트웨어적 지식 정리


# 용어 정리

## ctx[]
펌웨어가 오디오 함수에 넘겨주는 상태 포인터.

| 슬롯 | 의미 |
|------|------|
| `ctx[1]` | parameters table (파라미터 float 배열) |
| `ctx[4]` | Dry buffer (원본 기타 입력 신호) |
| `ctx[5]` | Fx buffer (업스트림 체인이 수정한 신호) |
| `ctx[6]` | Output buffer (누산기 — 여기에 ADD) |
| `ctx[11]` / `ctx[12]` | magic shuttle (목적 불명, 반드시 유지) |


## Fx buffer
LLLLLLLLRRRRRRRR
총 16개의 Index로 구성되어있음
모노로 쓸경우 LLLLLLLL 해당 8개의 Index만 활용해도됨.
