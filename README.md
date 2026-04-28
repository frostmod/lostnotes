# NTS-1 mkII Polyphonic Saw OSC Plugin

Minimal oscillator plugin to reproduce missing note-on events on the Korg NTS-1 mkII when playing chords in legato:off mode.

Tracks all 128 MIDI notes independently and renders a polyphonic PolyBLEP saw wave, making dropped notes immediately audible.

## Setup

Clone or copy this repo's contents into your local [logue-sdk](https://github.com/korginc/logue-sdk) under:

```
platform/nts-1_mkii/<this-folder>/
```

Then build following the standard logue-sdk instructions.
