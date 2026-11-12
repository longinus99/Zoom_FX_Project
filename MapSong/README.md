# MapSong

An input-triggered pitch sequencer for the original Zoom MS-70CDR.

[Download MapSong.ZDL](MapSong.ZDL?raw=true) and follow the [installation steps](../README.md#installation). The effect appears in the **Delay** category with effect ID 1715 (`0x06B3`).

## Playback

Enable the effect before the song and strike the guitar on the first beat. The sequence starts at bar 1, step 1 as soon as the input crosses the trigger threshold (normalized amplitude 0.008). Playback continues through rests and subsequent notes. Toggle the effect off and on to arm it again.

The first four bars play **AAAA** once, then **AAAD** repeats. The first D is on bar 8, followed by bars 12, 16, and so on. Each bar has 16 sixteenth-note steps.

| Pattern | Pitch pairs |
| --- | --- |
| A | `-0 +0 +0 -0 +0 +0 -0 +0` |
| D | `+0 -0 +0 +0 -0 +0 -0 +0` |

Each character represents one step. `-` means one octave down, `0` means unchanged pitch, and `+` means one octave up. The patterns were derived from the [reference MIDI file](https://drive.google.com/file/d/1dJas6UfTAPr44Pn5cwvmF25F-okRchHF/view).

- **BPM** defaults to 125.
- **Mix** defaults to 100. At 0, only the dry signal is audible. At 100, only the processed mono signal is audible.

PC tests verified the input trigger, playback through silence, the intro-to-loop transition, and re-arming. Physical pedal behavior has yet to be verified.

See [build instructions](../README.md#building-from-source) and the [license](LICENSE.txt).
