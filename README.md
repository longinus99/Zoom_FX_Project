# Zoom_FX_Project

기타 이펙터인 Zoom MS-70CDR에 원하는 FX를 만들어서 넣는 프로젝트

만드는법자체는 https://github.com/repeat98/ZoomMultistompZDL 에서 참고가 가능한듯

만들고 싶은 FX
1. Dual Pitch Shifter.

1차 목표.
Dissamble ZDL File.

## MapSeq

구형 **Zoom MS-70CDR**용 16스텝 피치 시퀀서입니다. 기타 입력을 스텝마다 한 옥타브 아래, 원래 음높이, 한 옥타브 위로 바꿉니다.

### 설치

1. [MapSeq.ZDL](MapSeq/MapSeq.ZDL?raw=true)을 다운로드합니다.
2. Zoom Effect Manager에서 기존 패치와 이펙트 목록을 백업합니다.
3. **Settings → Read Effects from folder**에서 ZDL을 저장한 폴더를 선택합니다.
4. `MapSeq`를 추가해 기기에 쓴 뒤, 페달의 **Delay** 카테고리에서 선택합니다.

### 노브와 기본 패턴

- `BPM` 기본값은 125입니다. 40~240 BPM에서 조절할 수 있고, 40 미만 값은 125로 처리합니다. 각 스텝은 16분음표 길이입니다.
- `S0102`부터 `S1516`까지 각 노브가 연속된 두 스텝을 담당합니다. `-0`은 앞 스텝에서 한 옥타브 아래, 뒤 스텝에서 원래 음높이로 설정합니다.
- 노브 값 0~8은 순서대로 `--`, `-0`, `-+`, `0-`, `00`, `0+`, `+-`, `+0`, `++`입니다.
- 처리음은 모노이며 Mix는 100%로 고정입니다.

| 노브 | S0102 | S0304 | S0506 | S0708 | S0910 | S1112 | S1314 | S1516 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 기본값 | `-0` | `+0` | `+0` | `-0` | `+0` | `+0` | `-0` | `+0` |
| 숫자 | 1 | 7 | 7 | 1 | 7 | 7 | 1 | 7 |

기본값은 「Map of the Problematique」 Whammy V용 2024 MIDI의 첫 마디에 해당합니다. MapSeq는 이 16스텝을 반복합니다. [MIDI 출처](https://www.reddit.com/r/Muse/comments/yrc34v/map_of_the_problematiqu%C3%A9_midi_sequence/)

### 소스와 빌드

[MapSeq 소스](MapSeq/source)는 [Matujuice 효과 팩의 이 리비전](https://github.com/matujuice/zoom-ms-zdl-effects-pack/tree/826ebb963315ee4eda99cd3f9cdf87b5095e20f6)을 기준으로 빌드합니다. 소스 파일 세 개를 해당 체크아웃의 `src/custom/mapseq/`에 복사하고, `TI_CGT_ROOT`를 TI C6000 CGT 8.5.0.LTS 설치 디렉터리로 지정한 뒤 저장소 루트에서 실행합니다.

```sh
python3 src/custom/mapseq/build.py
```

결과 파일은 `dist/MapSeq.ZDL`에 생성됩니다. [라이선스 및 크레딧](MapSeq/LICENSE.txt)

C6740 빌드와 PC에서 노브 조합, 기본 패턴, 타이밍을 확인했습니다. 페달 실기에서 설치, 노브 표시, 패치 저장·재호출은 미확인입니다.

## MapSong

구형 **Zoom MS-70CDR**용 기타 입력 감지 피치 시퀀서입니다. 페달을 켠 뒤 첫 기타 입력에 시작합니다. 첫 4마디는 `AAAA`이고, 5마디부터 `AAAD`를 반복합니다. D는 8, 12, 16마디부터 4마디마다 나옵니다. 기본값은 BPM 125, Mix 100입니다.

[MapSong.ZDL 다운로드](MapSong/MapSong.ZDL?raw=true) · [설치 및 빌드 안내](MapSong/README.md) · [소스](MapSong/source)
