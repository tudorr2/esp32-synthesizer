
---

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-F5822A?style=for-the-badge&logo=platformio&logoColor=white)
![3D Printing](https://img.shields.io/badge/3D%20Printed%20Case-PLA-9cf?style=for-the-badge)
![Handmade PCB](https://img.shields.io/badge/PCB-Handmade-green?style=for-the-badge)

---

## ◈

It's a tiny standalone synthesizer built from scratch on a handmade PCB, housed in a 3D-printed PLA enclosure. It generates polyphonic sawtooth wave audio through a low-pass filter with 3-voice detuning — all in real-time on the ESP32's DAC output, amplified through a PAM8403 into a 3mm speaker.

No synth shield. No MIDI interface. Just raw waveform math and a handful of components.

> **Inspired by and adapted from** → [`todbot/mozzi_experiments`](https://github.com/todbot/mozzi_experiments)
> The core logic and tone architecture draws heavily from todbot's work. I translated the original Python-based concepts to C++, rewired the patterns, reworked the tone arrays, and adapted the whole thing to run on the ESP32.

---

## ◈ Hardware

| Component | Role |
|-----------|------|
| **ESP32** (30-pin dev board) | Brain — runs audio ISR + sequencer logic |
| **PAM8403** | Class-D audio amplifier module |
| **40mm Speaker** | Audio output |
| **2× Potentiometers** | Root note + BPM control |
| **2× Tactile Buttons** | Pattern change + transpose control |
| **1× Slide/Toggle Switch** | Power / standby gate |
| **Handmade PCB** | Point-to-point etched board |
| **3D Printed PLA Case** | Custom enclosure, designed to fit the PCB snugly |

---

## ◈ Pin Map

```
ESP32 GPIO 36  ──  Knob A       (Root Note)
ESP32 GPIO 39  ──  Knob B       (BPM)
ESP32 GPIO 18  ──  Button A     (Change Pattern)
ESP32 GPIO 19  ──  Button B     (Transpose Step)
ESP32 GPIO 25  ──  DAC Out      → PAM8403 input
ESP32 GPIO 22  ──  LED          (Note trigger indicator)
ESP32 GPIO 15  ──  Power Switch 
```

---

## ◈ Controls

### 🎛 Knob A — Root Note
Sweeps the root note from **C1 (24)** to **C5 (72)**.
Turning it changes the key everything plays in — low and bassy to bright and cutting.

### 🎛 Knob B — Tempo (BPM)
Sets the playback speed from **40 BPM** to **180 BPM**.
Changes take effect immediately mid-pattern.

### 🔘 Button A — Cycle Pattern
Steps through the **15 built-in arpeggio patterns** one by one.
Each press advances to the next motif. Cycles back to the first after the last.

### 🔘 Button B — Transpose Steps
Cycles through **0 / 1 / 2 transpose steps** applied per loop.
With transpose active, each loop of the pattern shifts up by one octave (12 semitones), then wraps back — creating climbing or cascading phrases.



---

## ◈ Patterns

15 patterns, each 4 steps long. Intervals are in semitones from the root:

| # | Name | Steps | Character |
|---|------|-------|-----------|
| 0 | `major` | 0, 4, 7, 12 | Bright, uplifting |
| 1 | `minor7th` | 0, 3, 7, 10 | Moody, jazzy edge |
| 2 | `diminished` | 0, 3, 6, 3 | Tense, unsettled |
| 3 | `suspended4th` | 0, 5, 7, 12 | Open, floating |
| 4 | `octaves` | 0, 12, 0, −12 | Classic octave pump |
| 5 | `octaves2` | 0, 12, 24, −12 | Wide, dramatic sweep |
| 6 | `octaves3` | 0, −12, −12, 0 | Deep sub-drop |
| 7 | `new1` | 0, 1, 1, 1 | Chromatic crawl |
| 8 | `new2` | 0, 12, 12, 12 | Locked-octave pulse |
| 9 | `power` | 0, 7, 12, 7 | Power chord vibe |
| 10 | `jazzy` | 0, 4, 7, 11 | Major 7th, smooth |
| 11 | `fifth-jump` | 0, 12, 7, 19 | Leaping fifths |
| 12 | `glitchy` | 0, 6, 13, 5 | Dissonant, broken |
| 13 | `leap` | 0, 30, 20, 10 | Wild interval jumps |
| 14 | `root` | 0, 0, 0, 0 | Drone / root lock |

---

## ◈ Code

The project is split into two files:

- Replaced Mozzi's audio scheduler with `esp_timer` periodic ISR at 44.1kHz
- Rewrote all note/timing logic in fixed-step float arithmetic
- Added 3-voice detuned oscillator chorus
- Reworked the arpeggio/motif tables with new patterns (`glitchy`, `leap`, `new1`, `new2`, `power`, `fifth-jump`)

> Full credit to **[todbot](https://github.com/todbot)** whose `mozzi_experiments` repo was the direct inspiration and starting point for the pattern sequencer architecture. This is an adaptation — most of the higher-level motif/sequencer logic structure came from his work.

---

## ◈ Photos

> 📷 *Board prototype photos

<!-- Add your board photos here -->
<!-- ![Board Top View](photos/board_top.jpg) -->
<!-- ![Board Side View](photos/board_side.jpg) -->

---

## ◈ 3D Printed Case

> 📷 *PLA enclosure 

<!-- ![Case Render](photos/case_render.jpg) -->
<!-- ![Case Assembly](photos/case_assembly.jpg) -->

---


## ◈Credits

-Concept and sequencer architecture adapted from **[todbot/mozzi_experiments](https://github.com/todbot/mozzi_experiments)**


---
