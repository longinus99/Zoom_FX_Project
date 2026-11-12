# Zoom FX Project

Custom pitch-sequencing effects for the original Zoom MS-70CDR.

| Effect | Behavior | Download |
| --- | --- | --- |
| [MapSeq](#mapseq) | An editable 16-step pattern that repeats every bar | [MapSeq.ZDL](MapSeq/MapSeq.ZDL?raw=true) |
| [MapSong](MapSong/README.md) | Starts on the first guitar input, plays four A bars, then loops AAAD | [MapSong.ZDL](MapSong/MapSong.ZDL?raw=true) |

## Installation

1. Download the ZDL for the effect you want.
2. Back up your patches and effects in Zoom Effect Manager.
3. In **Settings → Read Effects from folder**, select the folder containing the ZDL.
4. Add the effect, write it to the pedal, and select it in the **Delay** category.

## MapSeq

MapSeq shifts the input down one octave, leaves its pitch unchanged, or shifts it up one octave on each sixteenth-note step. The 16 steps repeat as a one-bar loop.

- **BPM** defaults to 125. The usable range is 40–240 BPM. Values below 40 use 125 BPM.
- **S0102** through **S1516** each edit two consecutive steps. The left symbol applies to the first step and the right symbol to the second. Use `-` for an octave down, `0` for unchanged pitch, and `+` for an octave up.
- Knob values 0–8 correspond to `--`, `-0`, `-+`, `0-`, `00`, `0+`, `+-`, `+0`, and `++`.
- The processed signal is mono and Mix is fixed at 100%.

| Knob | S0102 | S0304 | S0506 | S0708 | S0910 | S1112 | S1314 | S1516 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Default pair | `-0` | `+0` | `+0` | `-0` | `+0` | `+0` | `-0` | `+0` |
| Numeric value | 1 | 7 | 7 | 1 | 7 | 7 | 1 | 7 |

The default pattern matches the first bar of the [2024 Whammy V MIDI sequence for “Map of the Problematique”](https://www.reddit.com/r/Muse/comments/yrc34v/map_of_the_problematiqu%C3%A9_midi_sequence/).

## MapSong

Enable MapSong before playing, then strike the guitar on the first beat. The first four bars are **AAAA**. From bar 5 onward, **AAAD** repeats, with D on bars 8, 12, 16, and every fourth bar thereafter. The sequence keeps running through rests. Toggle the effect off and on to arm it again. BPM defaults to 125 and Mix defaults to 100. See the [MapSong guide](MapSong/README.md) for the pitch patterns.

## Building from source

1. Check out the [pinned Matujuice effect pack revision](https://github.com/matujuice/zoom-ms-zdl-effects-pack/tree/826ebb963315ee4eda99cd3f9cdf87b5095e20f6).
2. Copy the three files from [MapSeq/source](MapSeq/source) to `src/custom/mapseq/`, or from [MapSong/source](MapSong/source) to `src/custom/mapsong/`.
3. Set `TI_CGT_ROOT` to the TI C6000 CGT 8.5.0.LTS installation directory. Run the build for your effect from the effect pack root.

```sh
python3 src/custom/mapseq/build.py
python3 src/custom/mapsong/build.py
```

The ZDL files are generated in `dist/`. See the [MapSeq license](MapSeq/LICENSE.txt) and [MapSong license](MapSong/LICENSE.txt).

Both effects were built for C6740 and tested on a PC. Installation and operation on a physical MS-70CDR have yet to be verified.
