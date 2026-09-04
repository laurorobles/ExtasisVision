# 🔬 EXTASIS VISION v2.0 — DETAILED SIGNAL & DSP ARCHITECTURE SPECIFICATION

This document provides a comprehensive technical breakdown of the audio signal flow, DSP processing blocks, threading topology, and Sync routing engine inside **Extasis Vision**.

---

## 🗺️ High-Level Signal Flow Diagram

```mermaid
flowchart TD
    subgraph ENGINE ["1. IMAGE ANALYSIS ENGINE (Audio Thread)"]
        DAW_TRANSPORT["DAW Host Sync / Internal PPQ"] --> SYNC["Scanner Sync Engine (BPM to Hz)"]
        SYNC --> SCAN1["Scanner 1 (Red)"]
        SYNC --> SCAN2["Scanner 2 (Cyan)"]
        SCAN1 --> |"X/Y Pixel Coordinates"| IMG_LOCK["Image Buffer (Critical Section)"]
        SCAN2 --> |"X/Y Pixel Coordinates"| IMG_LOCK
    end

    subgraph SYNTH ["2. DUAL SYNTHESIS PIPELINE"]
        IMG_LOCK --> |"Mode 0: Brightest Pixel"| ANALYTIC["Analytic Pitch/Amplitude Converter"]
        IMG_LOCK --> |"Mode 1: RGB Average"| RGB["RGB Subtractive Converter"]
        ANALYTIC --> QUANT["Scale Quantizer (Chromatic / Pentatonic)"]
        RGB --> QUANT
        QUANT --> OSC["Phase Accumulators (Sine Waves)"]
        OSC --> FILTER["One-Pole Lowpass (Controlled by Red Channel)"]
    end

    subgraph FX_RACK ["3. CYBERPUNK FX RACK"]
        FILTER --> DELAY["Echo (juce::dsp::DelayLine)"]
        DELAY --> REVERB["Space (juce::dsp::Reverb)"]
    end

    subgraph OUTPUT ["4. FINAL OUTPUT"]
        REVERB --> SOFT_CLIP["std::tanh (Soft Clipper)"]
        SOFT_CLIP --> DAW_OUT["DAW Stereo Output"]
    end
```

## 1. Drag & Drop and Threading
- **Message Thread**: Handles UI events, Drag & Drop (`filesDropped`). Images are loaded into a `juce::Image` and scaled down.
- **Audio Thread**: `processBlock` reads the `juce::Image`.
- **Synchronization**: A `juce::CriticalSection` (imageLock) prevents read/write race conditions between threads.

## 2. Sync & Polyrhythms
The scanners can operate in two time domains:
- **Free (Hz)**: Independent phase progression.
- **BPM Sync**: Derived from `AudioPlayHead::getPosition()`. `scanPositionX` becomes a direct mapping of `std::fmod(ppqPosition, beatDivisions)`.

## 3. Subtractive Synthesis Mode
- **Green**: Mapped to Pitch (Y-axis equivalent).
- **Blue**: Mapped to Amplitude.
- **Red**: Controls the coefficient of a One-Pole lowpass filter to sculpt tone.

## 4. License Management
- Handled offline via `LicenseManager.h`.
- Serial format: `EXTV-XXXX-XXXX-XXXX-XXXX`.
- Validated mathematically using a seeded hash generator and salt verification. Evaluated on plugin startup and loaded into an atomic boolean `isLicensedCached`.
