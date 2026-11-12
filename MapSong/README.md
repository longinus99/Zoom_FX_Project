# MapSong

Zoom Effect Manager에서 기존 패치와 이펙트를 백업합니다.
압축을 풀고 Settings > Read Effects from folder에서 압축을 푼 폴더를
지정해 MapSong.ZDL을 추가합니다. 페달의 Delay 카테고리에 나타납니다.
이펙트 ID는 Delay / 1715 (0x06B3)입니다.

곡 시작 전에 이펙트를 켜고, 첫 박에 기타를 칩니다. 입력이 임계값
(정규화 진폭 0.008)을 넘으면 1마디 1스텝부터 시작합니다.
이후 기타를 쉬거나 다시 쳐도 시퀀스는 계속 진행합니다.
첫 4마디는 A → A → A → A입니다. 5마디부터 A → A → A → D를 반복합니다.
D는 8, 12, 16…마디에 나옵니다. 다시 시작하려면 껐다 켭니다.

BPM 기본값은 125, Mix 기본값은 100입니다. Mix 0은 원음만,
100은 처리음만 출력합니다. 처리음은 모노입니다.

곡 순서는 아래 MIDI의 Aftertouch 값을 기존 옥타브 매핑으로 해석했습니다.
7 = -1옥타브, 1 = +1옥타브, 22/28 = 원래 음높이.
A = -0 +0 +0 -0 +0 +0 -0 +0
D = +0 -0 +0 +0 -0 +0 -0 +0
각 마디는 16분음표 16스텝입니다.
https://drive.google.com/file/d/1dJas6UfTAPr44Pn5cwvmF25F-okRchHF/view

PC에서 첫 입력 시작, 무음 중 진행, 첫 4마디와 이후 반복, 재대기를 확인했습니다. 페달 실기 동작은 미확인입니다.

재빌드
https://github.com/matujuice/zoom-ms-zdl-effects-pack/tree/826ebb963315ee4eda99cd3f9cdf87b5095e20f6
위 리비전의 src/custom/mapsong/에 source/ 파일을 복사합니다.
TI C6000 CGT 8.5.0.LTS 설치 경로를 TI_CGT_ROOT로 지정하고
python3 src/custom/mapsong/build.py를 실행합니다.
