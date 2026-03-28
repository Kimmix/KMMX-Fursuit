# Motion Detection Events Reference

This document provides a comprehensive reference for all motion detection events in the KMMX Fursuit Controller.

## Table of Contents
- [Configuration](#configuration)
- [Event Summary](#event-summary)
- [Detailed Event Specifications](#detailed-event-specifications)
- [Logging Reference](#logging-reference)
- [Tuning Guide](#tuning-guide)

---

## Configuration

### Global Settings

```cpp
const bool enableMotionDebug = false;              // Set to true for detailed debug output
const uint16_t motionDetectionStartupDelay = 3000; // Delay before motion detection starts (ms)
```

**Startup Delay:** Motion detection is disabled for the first 3 seconds after boot to prevent false triggers during accelerometer initialization and calibration.

**State Management:** The eye and mouth states are explicitly set to IDLE at startup to prevent lingering states from previous sessions. SMILE state is not saved as a previous state to prevent it from persisting after motion detection ends.

### Feature Enable/Disable Flags
Each motion detection feature can be individually enabled or disabled:

| Feature | Config Flag | Default |
|---------|-------------|---------|
| Tilt Detection | `enableTiltDetection` | `true` |
| Petting Detection | `enablePettingDetection` | `true` |

---

## Event Summary

| Motion Type | Trigger | Detection Method | Idle Timer | Priority |
|-------------|---------|------------------|------------|----------|
| **Tilt** | Sustained head tilt (>500ms) | Accelerometer axis angles | No change | High |
| **Petting** | Gentle rocking (>1.5s) | Low-frequency oscillation | No change | Low |

---

## Detailed Event Specifications

### 1. Tilt Detection

**Purpose:** Detects sustained head tilt for curious/confused expressions

**Detection Algorithm:**
- Monitors accelerometer X-axis (forward/back) and Z-axis (left/right)
- Requires sustained tilt for 500ms before triggering
- Debounce period of 300ms between tilt changes
- **Priority:** Forward/Back (up/down) takes precedence over Left/Right
- **Strict Lock:** Once Forward/Back triggers, Left/Right is completely blocked until return to neutral
- **Direction Change Cooldown:** 3000ms (3 seconds) cooldown when switching from Left/Right to Forward/Back
- **Clean State Machine:** Simplified logic prevents edge cases and ensures predictable behavior

**Thresholds:**

| Parameter | Value | Description |
|-----------|-------|-------------|
| `tiltThreshold` | 4.0 m/s² | Minimum tilt to trigger (reduced for easier triggering) |
| `tiltNeutralThreshold` | 3.0 m/s² | Return to neutral threshold (increased for easier return) |
| `tiltSustainTime` | 500 ms | Time to hold tilt before triggering |
| `tiltDebounceTime` | 300 ms | Cooldown between tilt changes |
| `tiltDirectionChangeCooldown` | 3000 ms | Cooldown when switching from Left/Right to Forward/Back (prevents rapid direction changes) |

**Responses:**

| Direction | Eye State | Mouth State | LED Color | Notes |
|-----------|-----------|-------------|-----------|-------|
| **Left/Right** (Z-axis) | DOUBTED | EH | MAGENTA | Curious/confused |
| **Forward** (X-axis +) | UNIMPRESSED | IDLE | BLUE | Looking down |
| **Back** (X-axis -) | ROUNDED | IDLE | CYAN | Looking up |

**State Management:**
- Saves previous eye/mouth states on tilt start
- Returns to previous states when head returns to neutral
- Does NOT reset idle timer (allows sleep while tilted)

---

### 2. Petting Detection

**Purpose:** Detects gentle rocking motion for contentment (simplified to single SMILE response)

**Detection Algorithm:**
- Monitors oscillation amplitude (deviation from average magnitude)
- Requires gentle oscillation (1.0-3.5 m/s²) for sustained period
- Tracks oscillation history over 400ms window (20 samples @ 50Hz)
- **Cooldown:** 1 second cooldown after ANY tilt returns to neutral to prevent immediate petting trigger
- **Note:** Cooldown activates when head returns to neutral position from any tilt direction

**Thresholds:**

| Parameter | Value | Description |
|-----------|-------|-------------|
| `pettingMinMagnitude` | 1.0 m/s² | Minimum oscillation amplitude (balanced for easier triggering) |
| `pettingMaxMagnitude` | 3.5 m/s² | Maximum oscillation amplitude (narrowed to prevent vigorous movement triggers) |
| `pettingMinDuration` | 1500 ms | Minimum duration to trigger SMILE response (balanced sensitivity) |
| `pettingCooldownAfterTilt` | 1000 ms | Cooldown after tilt ends before petting can trigger |
| `enablePettingCooldownAfterTilt` | true | Enable/disable cooldown feature |
| `pettingMinFrequency` | 0.5 Hz | Minimum rocking frequency (unused in current impl) |
| `pettingMaxFrequency` | 2.0 Hz | Maximum rocking frequency (unused in current impl) |

**Response:**

| Duration | Eye State | Mouth State | LED Color | Cheek Brightness | Notes |
|----------|-----------|-------------|-----------|------------------|-------|
| **1.5+ seconds** | SMILE | IDLE | PINK | 200 | Happy (single response) |

**State Management:**
- Saves previous eye/mouth states on petting start
- Returns to previous state when petting stops (after 2 seconds)
- Does NOT reset idle timer (allows natural sleep transition)
- Petting state resets if oscillation stops for >2 seconds

---

## Logging Reference

### Motion Detection Logs (Always Enabled)

These logs appear whenever motion is first detected, regardless of the `enableMotionDebug` flag:

| Feature | Log Format | Example |
|---------|------------|---------|
| **Tilt** | `[TILT] ✓ Motion detected! Angle: X.XX, Direction: Left/Right\|Forward/Back (threshold: X.XX)` | `[TILT] ✓ Motion detected! Angle: 5.67, Direction: Left/Right (threshold: 4.00)` |
| **Spin** | `[SPIN] ✓ Motion detected! Angular velocity: X.XX (threshold: X.XX)` | `[SPIN] ✓ Motion detected! Angular velocity: 9.34 (threshold: 8.00)` |
| **Petting** | `[PETTING] ✓ Motion detected! Amplitude: X.XX (range: X.XX-X.XX)` | `[PETTING] ✓ Motion detected! Amplitude: 3.21 (range: 2.00-5.00)` |

### Detailed Debug Logs (Enabled with `enableMotionDebug = true`)

These logs provide detailed state transitions and are only shown when `enableMotionDebug` is set to `true` in `config.h`:

#### Tilt Detection
```
[TILT] Left/Right tilt response! Angle: 5.67
[TILT] Forward tilt response! Angle: 4.32
[TILT] Back tilt response! Angle: -4.89
[TILT] Returned to neutral
```

#### Spin Detection
```
[SPIN] Spin response triggered! Angular velocity: 9.34
[SPIN] Ended - dizzy state
```

#### Petting Detection
```
[PETTING] Petting response! (SMILE)
[PETTING] Ended - returning to previous state
```

---

## Tuning Guide

### How to Tune Motion Detection

1. **Enable Debug Logging**
   ```cpp
   // In config.h
   const bool enableMotionDebug = true;
   ```

2. **Upload and Monitor**
   - Upload code to ESP32-S3
   - Open Serial Monitor at 115200 baud
   - Perform the motion you want to tune

3. **Observe Logged Values**
   - Motion detection logs show actual values vs. thresholds
   - Use these values to adjust thresholds in `config.h`

4. **Adjust Thresholds**
   - **Too sensitive:** Increase threshold values
   - **Not sensitive enough:** Decrease threshold values
   - **False triggers:** Increase sustain/duration times
   - **Missed triggers:** Decrease sustain/duration times

### Common Tuning Scenarios

#### Shake Detection Too Sensitive
```cpp
// Increase thresholds
const float shakeLowIntensity = 4.0f;    // Was 3.0
const float shakeMediumIntensity = 8.0f; // Was 6.0
const float shakeHighIntensity = 12.0f;  // Was 10.0
```

#### Tilt Not Triggering
```cpp
// Decrease threshold or sustain time
const float tiltThreshold = 3.0f;        // Was 4.0
const uint16_t tiltSustainTime = 300;    // Was 500
```

#### Bounce Too Easy to Trigger
```cpp
// Increase thresholds
const float bounceUpThreshold = 15.0f;   // Was 12.0
const float bounceDownThreshold = 10.0f; // Was 8.0
```

#### Spin Not Detecting
```cpp
// Decrease threshold or duration
const float spinThreshold = 6.0f;        // Was 8.0
const uint16_t spinMinDuration = 600;    // Was 800
```

#### Petting Too Sensitive
```cpp
// Narrow the magnitude range or increase duration
const float pettingMinMagnitude = 2.5f;  // Was 2.0
const float pettingMaxMagnitude = 4.5f;  // Was 5.0
const uint16_t pettingMinDuration = 7000; // Was 5000
```

### Testing Checklist

- [ ] **Shake:** Shake head rapidly at different intensities
- [ ] **Tilt:** Tilt head left, right, forward, back and hold
- [ ] **Bounce:** Jump/bounce head vertically 1, 2, 3, 4+ times
- [ ] **Spin:** Rotate head in circles (horizontal plane)
- [ ] **Petting:** Gently rock head back and forth for 5+ seconds

### Performance Notes

- All motion detection runs at **50Hz** on Core 0
- Minimal CPU overhead (~5-10% with all features enabled)
- No blocking operations - all detection is non-blocking
- Thread-safe double-buffer pattern for sensor data access

---

## Priority Hierarchy

When multiple motions are detected simultaneously, the following priority order is used:

1. **Boop Detection** (highest priority - always wins)
2. **Active Motion** (Shake, Bounce, Spin)
3. **Passive Motion** (Tilt, Petting)
4. **Idle/Sleep States** (lowest priority)

**Note:** Tilt and Petting do NOT reset the idle timer, allowing the suit to naturally transition to sleep if the wearer is still.

---

## Integration with Existing Features

### Idle/Sleep Detection
- Motion detection uses the same accelerometer data as idle detection
- Active motions (Shake, Bounce, Spin) reset the idle timer
- Passive motions (Tilt, Petting) allow natural sleep transition

### Boop Detection
- Boop (proximity sensor) always has highest priority
- Motion detection is suppressed during boop events
- Motion states resume after boop ends

### Facial Animations
- Motion detection temporarily overrides facial states
- Previous states are saved and restored when motion ends
- Smooth transitions between states

---

## Troubleshooting

### Motion Not Detected
1. Check that feature is enabled in `config.h`
2. Enable `enableMotionDebug` to see actual values
3. Verify accelerometer is working (check idle detection)
4. Lower threshold values if needed

### False Triggers
1. Increase threshold values
2. Increase sustain/duration times
3. Check for mechanical vibrations in mounting

### Flickering Between States
1. Increase debounce/cooldown times
2. Increase hysteresis (difference between trigger and release thresholds)
3. Check for loose connections causing sensor noise

### No Serial Output
1. Verify Serial Monitor is at 115200 baud
2. Check that USB cable supports data (not just power)
3. Ensure `enableMotionDebug` is set to `true` for detailed logs

---

## File Locations

- **Configuration:** `src/config.h` (lines 111-150)
- **Implementation:** `src/KMMXController/ControllerMotionDetection.cpp`
- **Header:** `src/KMMXController/KMMXController.h`
- **Integration:** `src/KMMXController/ControllerSensor.cpp`

---

**Last Updated:** 2026-03-24
**Version:** 1.0
**Author:** KMMX Fursuit Controller Team

