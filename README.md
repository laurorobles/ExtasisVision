# ExtasisVision

**ExtasisVision** is a cross-platform, multi-engine VST3 and Standalone synthesizer plugin built with JUCE and C++ that turns images into sound.

Inspired by the T-1000 and dark cyberpunk aesthetics, ExtasisVision scans your photos and translates their pixels into complex soundscapes and melodies in real-time.

## Engines
- **Analytic Scanner:** Reads the brightness and vertical position of the brightest pixels as the scan line moves across the image. Brightness = Amplitude, Y-Axis = Frequency.
- **RGB Synthesizer:** Extracts the average RGB values of the current column. Green controls pitch, Blue controls volume, and Red controls a low-pass filter to tame the frequencies.

## Musicality
The plugin includes a **Scale Quantization** system, ensuring that the generated frequencies snap to your selected musical scales:
- Free (Microtonal)
- Chromatic
- Minor Pentatonic

## Build Instructions (macOS)
1. Clone this repository.
2. Run `cmake -B build`
3. Run `cmake --build build`
4. The `.app` and `.vst3` will be in `build/ExtasisVision_artefacts/`

## Downloads
Check the **Actions** tab on GitHub to download the automated builds for **Windows (.vst3)** and **Linux (.vst3)**.

---
*coded by @laurorobles*
