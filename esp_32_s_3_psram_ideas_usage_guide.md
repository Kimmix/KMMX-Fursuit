# ESP32-S3 PSRAM Ideas & Usage Guide

> Target chip: **ESP32-S3 N16N8 (16 MB Flash + 8 MB PSRAM)**
> Use case: **HUB75 DMA display, face animations, sensors, WS2812 accents, NPC-style behavior**

---

## 1. Mental Model: How to Think About PSRAM

Think of PSRAM as:
- **Large, slower memory**
- Perfect for **big, sequential data**
- Similar to **GPU VRAM** in spirit

Think of internal SRAM (DRAM) as:
- **Fast, low-latency memory**
- For timing-critical code and hot data

> Rule of thumb:
> - **Big + sequential → PSRAM**
> - **Small + hot → Internal RAM**

---

## 2. Framebuffers (Beyond Double Buffering)

### 2.1 Double Buffering

- Buffer A → DMA display
- Buffer B → CPU rendering

Already hides most PSRAM latency.

### 2.2 Triple Buffering (Recommended)

- Buffer A → DMA display
- Buffer B → Current render
- Buffer C → Pre-render next state

Benefits:
- Zero frame drops
- Preloading expressions
- Seamless transitions

```cpp
uint16_t* fb[3];
for (int i = 0; i < 3; i++) {
  fb[i] = (uint16_t*) heap_caps_malloc(
    WIDTH * HEIGHT * sizeof(uint16_t),
    MALLOC_CAP_SPIRAM
  );
}
```

---

## 3. Expression Cache (Huge UX Upgrade)

Instead of rendering everything live:

- Pre-render expressions once
- Store them in PSRAM
- Instantly swap frames

Examples:
- Neutral
- Happy
- Sleepy
- Angry
- Error / warning
- Idle variants

Result:
- Snappier face
- Less CPU load
- More expressive personality

---

## 4. Smooth Transitions & Post-Effects

PSRAM enables multi-frame effects:

- Crossfades between expressions
- Motion blur / eye trails
- Cheap glow / bloom
- Ghosting on fast movement

Technique:
- Keep 2–3 full frames
- Blend during render phase

---

## 5. Audio & Microphone History Buffers

Perfect for:
- Mouth animation
- VU meters
- Beat / envelope detection

```cpp
int16_t* micHistory = (int16_t*) heap_caps_malloc(
  2048 * sizeof(int16_t),
  MALLOC_CAP_SPIRAM
);
```

Allows:
- Sliding RMS windows
- Temporal smoothing
- Audio-reactive expressions

---

## 6. Sensor History & Gesture Intelligence

Ideal for IMU-heavy projects:

- Store seconds of motion data
- Gesture recognition
- Intent vs noise detection

Examples:
- Nod vs shake
- Lean-and-hold gestures
- Calm vs energetic movement states

This enables **organic, NPC-like behavior**.

---

## 7. Asset Streaming (Flash → PSRAM)

Store assets in Flash / LittleFS:
- Icons
- Fonts
- Pixel art
- Boot animations

Load into PSRAM at runtime:

```cpp
uint8_t* asset = (uint8_t*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
```

Then:
- Decode
- Render
- Free when done

---

## 8. Behavior Brain / NPC State Memory

Use PSRAM to store:
- State history
- Cooldown timers
- Random seed pools
- Emotional inertia variables

Result:
- Less repetitive behavior
- More lifelike idle loops
- Long-term memory effects

---

## 9. Debug & Diagnostics Buffers (Underrated)

During development:
- Store last N frames
- Log sensor snapshots
- Dump on error via Serial

Zero performance cost in normal operation.

---

## 10. Dirty Rectangles & Partial Redraw

Instead of full redraw:
- Track changed regions
- Only update dirty areas

Benefits:
- Higher FPS
- Lower power usage
- Cooler operation

Requires extra memory → PSRAM makes it easy.

---

## 11. Recommended Memory Layout

| Component | Memory |
|---------|-------|
| HUB75 DMA descriptors | Internal RAM |
| Scanline metadata | Internal RAM |
| Framebuffers | PSRAM |
| Expression cache | PSRAM |
| Sensor history | PSRAM |
| Gamma tables | Flash (PROGMEM) |
| WS2812 buffers | Internal RAM |

---

## 12. What NOT to Put in PSRAM

Avoid placing these in PSRAM:

- ISR variables
- DMA descriptors
- Tight inner-loop data
- WS2812 bit-bang buffers
- Frequently accessed small structs

---

## 13. Performance Notes (HUB75 DMA)

- PSRAM framebuffers typically cause **5–15% refresh drop**
- Double/triple buffering hides latency
- ≥150 Hz refresh remains camera-safe

For face animations, the difference is **visually negligible**.

---

## 14. Final Takeaway

PSRAM turns the ESP32-S3 into a **tiny graphics computer**:

- Multiple frames
- Cached expressions
- Smooth transitions
- Rich sensor memory
- NPC-style behavior logic

You’ll hit creative limits long before memory limits.

---

*End of document*

