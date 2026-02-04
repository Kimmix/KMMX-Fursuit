# Interactive Behaviors - Implementation Guide

## 🎭 Interaction Philosophy

The KMMX fursuit uses a **three-layer sensory architecture** to create natural, emotionally engaging interactions that feel alive and responsive. Each sensor layer builds upon the previous one to create contextual awareness and emotional depth.

### The Three Layers

```
┌─────────────────────────────────────────────────────────────┐
│  Layer 3: CONSENT (Capacitive Touch)                        │
│  "You touched me" - Physical affection & boundaries         │
├─────────────────────────────────────────────────────────────┤
│  Layer 2: COMMUNICATION (Gesture Recognition)               │
│  "I understand you" - Intent & playful interaction          │
├─────────────────────────────────────────────────────────────┤
│  Layer 1: AWARENESS (Time-of-Flight Distance)               │
│  "I see you" - Spatial awareness & anticipation             │
└─────────────────────────────────────────────────────────────┘
```

**Why this matters:**
- **Layer 1 (ToF)** creates anticipation - the character notices you before you touch
- **Layer 2 (Gesture)** enables communication - you can "talk" without touching
- **Layer 3 (Touch)** requires consent - physical interaction is the deepest level

This hierarchy prevents the character from feeling like a passive toy and instead creates the illusion of agency and emotional intelligence.

---

## 🔧 Hardware Stack

### Current Implementation (APDS9930)
- **Sensor:** APDS9930 proximity + ambient light
- **Range:** ~0-1023 (normalized, ~10cm max effective range)
- **Polling Rate:** 50Hz (20ms intervals)
- **Features:** Sunlight compensation, 5-sample averaging

### Planned Upgrades

#### VL53L1X Time-of-Flight Sensor
- **Range:** 4m maximum, 1-400mm optimal
- **Accuracy:** ±3% at 100mm
- **Polling Rate:** Up to 50Hz
- **Advantages:** Immune to ambient light, precise distance measurement
- **I2C Address:** 0x29 (configurable)

#### PAJ7620U2 Gesture Sensor
- **Gestures:** Up, Down, Left, Right, Forward, Backward, Clockwise, Counter-clockwise, Wave
- **Range:** 5-10cm optimal
- **Polling Rate:** 120Hz internal, recommend 30-50Hz polling
- **I2C Address:** 0x73
- **Advantages:** Built-in gesture recognition, low CPU overhead

#### Capacitive Touch (Current: APDS9930 proximity)
- **Current:** Proximity-based "boop" detection
- **Future:** True capacitive touch (MPR121 or similar)
- **Advantages:** Distinguishes between hover and actual touch

---

## 📊 Sensor Polling & Timing

### Recommended Polling Rates

| Sensor | Polling Rate | Rationale |
|--------|--------------|-----------|
| **VL53L1X (ToF)** | 50Hz (20ms) | Matches accelerometer, smooth distance tracking |
| **PAJ7620U2 (Gesture)** | 30Hz (33ms) | Balances responsiveness with CPU load |
| **Capacitive Touch** | 50Hz (20ms) | Fast enough for touch detection, matches sensor task |
| **Accelerometer (LIS3DH)** | 50Hz (20ms) | Current implementation baseline |

### Current Sensor Task Implementation

```cpp
// From ControllerSensor.cpp
const uint8_t sensorUpdateInterval = 20;  // 50Hz polling rate

void KMMXController::readSensorTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    constexpr TickType_t xFrequency = pdMS_TO_TICKS(sensorUpdateInterval);

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Double-buffer pattern for thread-safe access
        uint8_t writeBuffer = 1 - ctrl->activeBuffer;

        // Read sensors into inactive buffer
        // ... sensor reading code ...

        // Atomic swap
        ctrl->activeBuffer = writeBuffer;
    }
}
```

**Key Design Patterns:**
- ✅ **FreeRTOS task** on Core 0 (isolated from rendering on Core 1)
- ✅ **Double-buffering** prevents race conditions
- ✅ **vTaskDelayUntil** ensures consistent timing
- ✅ **Priority 2** balances responsiveness with rendering

---

## 🎯 Layer 1: Awareness (ToF Distance Sensing)

### Distance Zones & Expressions

| Distance Range | State | Expression | Visual Cues | Emotional Context |
|----------------|-------|------------|-------------|-------------------|
| **>500mm** | `IDLE` | Neutral idle | Slow breathing animation, occasional blinks | "Just existing peacefully" |
| **500-300mm** | `CURIOUS` | Awakening | Eyes brighten, pupils appear, subtle scan | "Oh? Someone's approaching" |
| **300-150mm** | `FOCUSED` | Interested | Eyes track center, eyelids raised, glow increases | "You have my attention" |
| **150-50mm** | `ANTICIPATION` | Excited | Big shiny eyes ✨, single blink, cheeks glow, micro bounce | "Are you gonna boop me??" |
| **<50mm** | `BOOP_READY` | Primed | Maximum brightness, eyes wide, ready for touch | "So close!" |

### Implementation Constants

```cpp
// Add to config.h
namespace ToFZones {
    constexpr uint16_t IDLE_DISTANCE = 500;        // mm - beyond this, return to idle
    constexpr uint16_t CURIOUS_DISTANCE = 300;     // mm - initial awareness
    constexpr uint16_t FOCUSED_DISTANCE = 150;     // mm - active attention
    constexpr uint16_t ANTICIPATION_DISTANCE = 50; // mm - expecting interaction
    constexpr uint16_t BOOP_READY_DISTANCE = 30;   // mm - very close, ready for touch
}

// Timing constants
namespace ToFTiming {
    constexpr uint16_t ZONE_TRANSITION_DELAY = 200;  // ms - debounce zone changes
    constexpr uint16_t CURIOSITY_TIMEOUT = 5000;     // ms - return to idle if no closer approach
    constexpr uint16_t ANTICIPATION_TIMEOUT = 3000;  // ms - return to focused if no boop
}
```

### State Machine Logic

```cpp
enum class AwarenessState {
    IDLE,
    CURIOUS,
    FOCUSED,
    ANTICIPATION,
    BOOP_READY
};

class AwarenessLayer {
private:
    AwarenessState currentState = AwarenessState::IDLE;
    AwarenessState previousState = AwarenessState::IDLE;
    uint16_t currentDistance = 9999;
    unsigned long lastTransitionTime = 0;
    unsigned long lastApproachTime = 0;

public:
    void update(uint16_t distanceMm) {
        currentDistance = distanceMm;
        unsigned long now = millis();

        // Determine target state based on distance
        AwarenessState targetState = determineState(distanceMm);

        // Apply transition delay to prevent flickering
        if (targetState != currentState) {
            if (now - lastTransitionTime >= ToFTiming::ZONE_TRANSITION_DELAY) {
                transitionTo(targetState);
                lastTransitionTime = now;
            }
        }

        // Timeout logic - return to lower states if no progression
        applyTimeouts(now);
    }

private:
    AwarenessState determineState(uint16_t distance) {
        if (distance < ToFZones::BOOP_READY_DISTANCE) return AwarenessState::BOOP_READY;
        if (distance < ToFZones::ANTICIPATION_DISTANCE) return AwarenessState::ANTICIPATION;
        if (distance < ToFZones::FOCUSED_DISTANCE) return AwarenessState::FOCUSED;
        if (distance < ToFZones::CURIOUS_DISTANCE) return AwarenessState::CURIOUS;
        return AwarenessState::IDLE;
    }

    void transitionTo(AwarenessState newState) {
        previousState = currentState;
        currentState = newState;
        onStateEnter(newState);
    }

    void onStateEnter(AwarenessState state) {
        switch (state) {
            case AwarenessState::CURIOUS:
                // Trigger: Eyes wake up, pupils appear
                eyeState.setState(EyeStateEnum::CURIOUS);
                hornLED.setBrightness(30, 5);  // Gentle glow
                break;

            case AwarenessState::FOCUSED:
                // Trigger: Eyes track center, brightness increase
                eyeState.setState(EyeStateEnum::FOCUSED);
                hornLED.setBrightness(50, 3);
                break;

            case AwarenessState::ANTICIPATION:
                // Trigger: Big eyes, cheek glow, micro bounce
                eyeState.setState(EyeStateEnum::ANTICIPATION);
                cheekPanel.setBrightness(200);  // Cheeks glow
                hornLED.setBrightness(80, 2);
                // TODO: Add micro bounce animation
                break;

            case AwarenessState::BOOP_READY:
                // Trigger: Maximum excitement
                eyeState.setState(EyeStateEnum::BOOP_READY);
                hornLED.setBrightness(100, 1);
                break;

            case AwarenessState::IDLE:
                // Return to neutral
                eyeState.setState(EyeStateEnum::IDLE);
                hornLED.setBrightness(hornInitBrightness, 10);
                cheekPanel.setBrightness(sideLEDBrightness);
                break;
        }
    }

    void applyTimeouts(unsigned long now) {
        // If curious but no closer approach for 5 seconds, return to idle
        if (currentState == AwarenessState::CURIOUS) {
            if (now - lastApproachTime >= ToFTiming::CURIOSITY_TIMEOUT) {
                transitionTo(AwarenessState::IDLE);
            }
        }

        // If anticipating but no boop for 3 seconds, return to focused
        if (currentState == AwarenessState::ANTICIPATION) {
            if (now - lastApproachTime >= ToFTiming::ANTICIPATION_TIMEOUT) {
                transitionTo(AwarenessState::FOCUSED);
            }
        }
    }
};
```

---

## 🤚 Layer 2: Communication (Gesture Recognition)

### Gesture Mapping

| Gesture | Expression | Visual Response | Emotional Context | Cooldown |
|---------|------------|-----------------|-------------------|----------|
| **Swipe LEFT** | Confused/Thinking | One eye squints, mouth flat/zigzag, head tilt left | "Huh?" | 1000ms |
| **Swipe RIGHT** | Acknowledging | Both eyes wide, happy arc, short nod | "Got it!" | 1000ms |
| **Swipe UP** | Excited/Powered Up | Starry eyes ⭐, big smile, cheeks brighten | "LET'S GO!" | 2000ms |
| **Swipe DOWN** | Sleepy/Power Save | Eyes half-closed, slow blink, dim LEDs | "Eepy…" | 3000ms |
| **Wave** | Playful | Rapid eye tracking, smug smile, bounce | "Stop that 😏" | 500ms |
| **Clockwise** | Dizzy | Spiral eyes, wobbly animation | "Whoa…" | 2000ms |
| **Counter-CW** | Reverse Dizzy | Opposite spiral | "Oof…" | 2000ms |

### Implementation Structure

```cpp
enum class GestureType {
    NONE,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    SWIPE_UP,
    SWIPE_DOWN,
    WAVE,
    CLOCKWISE,
    COUNTER_CLOCKWISE,
    FORWARD,
    BACKWARD
};

class GestureLayer {
private:
    GestureType lastGesture = GestureType::NONE;
    unsigned long lastGestureTime = 0;
    std::map<GestureType, unsigned long> gestureCooldowns = {
        {GestureType::SWIPE_LEFT, 1000},
        {GestureType::SWIPE_RIGHT, 1000},
        {GestureType::SWIPE_UP, 2000},
        {GestureType::SWIPE_DOWN, 3000},
        {GestureType::WAVE, 500},
        {GestureType::CLOCKWISE, 2000},
        {GestureType::COUNTER_CLOCKWISE, 2000}
    };

public:
    void update(GestureType gesture, uint16_t distance) {
        unsigned long now = millis();

        // Ignore if in cooldown
        if (isInCooldown(gesture, now)) return;

        // Context-aware response based on distance
        bool hasToFContext = (distance < ToFZones::FOCUSED_DISTANCE);

        if (gesture != GestureType::NONE) {
            handleGesture(gesture, hasToFContext);
            lastGesture = gesture;
            lastGestureTime = now;
        }
    }

private:
    bool isInCooldown(GestureType gesture, unsigned long now) {
        if (gesture == lastGesture) {
            unsigned long cooldown = gestureCooldowns[gesture];
            return (now - lastGestureTime) < cooldown;
        }
        return false;
    }

    void handleGesture(GestureType gesture, bool hasContext) {
        // Gestures without ToF context get muted response
        if (!hasContext) {
            handleDismissiveGesture(gesture);
            return;
        }

        switch (gesture) {
            case GestureType::SWIPE_LEFT:
                eyeState.setState(EyeStateEnum::CONFUSED);
                mouthState.setState(MouthStateEnum::FLAT);
                // TODO: Head tilt animation
                break;

            case GestureType::SWIPE_RIGHT:
                eyeState.setState(EyeStateEnum::HAPPY);
                // TODO: Nod animation
                break;

            case GestureType::SWIPE_UP:
                eyeState.setState(EyeStateEnum::EXCITED);
                mouthState.setState(MouthStateEnum::BIG_SMILE);
                cheekPanel.setBrightness(255);
                fxState.setState(FXStateEnum::SPARKLE);
                break;

            case GestureType::SWIPE_DOWN:
                eyeState.setState(EyeStateEnum::SLEEPY);
                hornLED.setBrightness(5, 20);  // Slow dim
                cheekPanel.setBrightness(50);
                // TODO: Slow down idle animation
                break;

            case GestureType::WAVE:
                eyeState.setState(EyeStateEnum::PLAYFUL);
                mouthState.setState(MouthStateEnum::SMUG);
                // TODO: Rapid eye tracking, bounce animation
                break;
        }
    }

    void handleDismissiveGesture(GestureType gesture) {
        // Subtle acknowledgment without full reaction
        eyeState.setGlance(GLANCE_SIDEWAYS);  // Quick side-eye
        // No mouth change, no LED change
    }
};
```

---

## 💖 Layer 3: Consent (Capacitive Touch)

### Touch Patterns & Responses

| Touch Pattern | Duration | Expression | Visual Response | Emotional Context | Cooldown |
|---------------|----------|------------|-----------------|-------------------|----------|
| **Single Boop** | <500ms | Happy | Big round eyes, smile, heart pop, soft blink | "Hehe~ 💕" | 500ms |
| **Long Hold** | 500-2000ms | Very Happy | Eyes → crescents, blush/pink glow, slow breathing | "I like this…" | 1000ms |
| **Rapid Spam** | >3 boops in 1s | Annoyed (Cute) | Angry eyes, cheeks flash, zigzag mouth, tiny shake | "HEY—!" | 3000ms |
| **Continuous** | >2000ms | Melting | Maximum blush, eyes closed, purring animation | "This is nice…" | 2000ms |

### Current Implementation (APDS9930-based)

```cpp
// From ControllerBooping.cpp - Current proximity-based "boop"
void KMMXController::handleBoop() {
    if (millis() >= nextBoop) {
        nextBoop = millis() + 50;  // 20Hz polling

        boop.getBoop(
            sensorBuffer[activeBuffer].proximity,
            inBoopRange,      // Close enough to boop
            isBooping,        // Active boop detected
            boopSpeed,        // Speed of approach
            isContinuousBoop, // Held for extended time
            isAngry           // Spam detected
        );

        if (isBooping) {
            fxState.setFlyingSpeed(boopSpeed);
            fxState.setState(FXStateEnum::Heart);
            eyeState.setState(EyeStateEnum::BOOP);
            mouthState.setState(MouthStateEnum::BOOP);
            resetIdleTime();
            statusLED.setColor(Color::CYAN);
        } else if (inBoopRange) {
            mouthState.setState(MouthStateEnum::BOOP);
            statusLED.setColor(Color::LIGHT_PINK);
        } else if (isContinuousBoop) {
            eyeState.setState(EyeStateEnum::BOOP);
            statusLED.setColor(Color::PINK);
        } else if (isAngry) {
            nextBoop = millis() + 1500;  // Cooldown after angry
            eyeState.setState(EyeStateEnum::ANGRY);
            mouthState.setState(MouthStateEnum::ANGRYBOOP);
            statusLED.setColor(Color::RED);
        }
    }
}
```

### Enhanced Touch Layer (Future: True Capacitive)

```cpp
class TouchLayer {
private:
    enum class TouchState {
        NONE,
        SINGLE_BOOP,
        LONG_HOLD,
        CONTINUOUS,
        SPAM_DETECTED
    };

    TouchState currentState = TouchState::NONE;
    unsigned long touchStartTime = 0;
    unsigned long lastTouchTime = 0;
    uint8_t touchCount = 0;
    unsigned long spamWindowStart = 0;

    // Timing constants
    static constexpr uint16_t LONG_HOLD_THRESHOLD = 500;      // ms
    static constexpr uint16_t CONTINUOUS_THRESHOLD = 2000;    // ms
    static constexpr uint16_t SPAM_WINDOW = 1000;             // ms
    static constexpr uint8_t SPAM_COUNT_THRESHOLD = 3;        // touches
    static constexpr uint16_t SPAM_COOLDOWN = 3000;           // ms

public:
    void update(bool isTouching, uint16_t distance) {
        unsigned long now = millis();

        // Detect touch start
        if (isTouching && currentState == TouchState::NONE) {
            onTouchStart(now);
        }

        // Update touch duration
        if (isTouching) {
            updateTouchDuration(now);
        }

        // Detect touch end
        if (!isTouching && currentState != TouchState::NONE) {
            onTouchEnd(now);
        }

        // Decay spam counter
        if (now - spamWindowStart > SPAM_WINDOW) {
            touchCount = 0;
        }
    }

private:
    void onTouchStart(unsigned long now) {
        touchStartTime = now;

        // Check for spam
        if (now - lastTouchTime < SPAM_WINDOW) {
            touchCount++;
            if (touchCount >= SPAM_COUNT_THRESHOLD) {
                currentState = TouchState::SPAM_DETECTED;
                handleSpam();
                return;
            }
        } else {
            touchCount = 1;
            spamWindowStart = now;
        }

        currentState = TouchState::SINGLE_BOOP;
    }

    void updateTouchDuration(unsigned long now) {
        unsigned long duration = now - touchStartTime;

        if (duration >= CONTINUOUS_THRESHOLD && currentState != TouchState::CONTINUOUS) {
            currentState = TouchState::CONTINUOUS;
            handleContinuous();
        } else if (duration >= LONG_HOLD_THRESHOLD && currentState == TouchState::SINGLE_BOOP) {
            currentState = TouchState::LONG_HOLD;
            handleLongHold();
        }
    }

    void onTouchEnd(unsigned long now) {
        lastTouchTime = now;

        if (currentState == TouchState::SINGLE_BOOP) {
            handleSingleBoop();
        }

        currentState = TouchState::NONE;
    }

    void handleSingleBoop() {
        eyeState.setState(EyeStateEnum::HAPPY);
        mouthState.setState(MouthStateEnum::SMILE);
        fxState.setState(FXStateEnum::Heart);
        fxState.setFlyingSpeed(0.5f);
        statusLED.setColor(Color::PINK);
        // TODO: Soft blink animation
    }

    void handleLongHold() {
        eyeState.setState(EyeStateEnum::VERY_HAPPY);  // Crescent eyes
        cheekPanel.setBrightness(255);  // Blush
        hornLED.setBrightness(80, 5);
        // TODO: Slow breathing animation
        statusLED.setColor(Color::LIGHT_PINK);
    }

    void handleContinuous() {
        eyeState.setState(EyeStateEnum::MELTING);  // Eyes closed, content
        cheekPanel.setBrightness(255);
        // TODO: Purring/vibration animation
        statusLED.setColor(Color::MAGENTA);
    }

    void handleSpam() {
        eyeState.setState(EyeStateEnum::ANGRY);
        mouthState.setState(MouthStateEnum::ANGRYBOOP);
        cheekPanel.flash(3, 100);  // Flash 3 times, 100ms each
        // TODO: Tiny shake animation
        statusLED.setColor(Color::RED);
    }
};
```

---

## 🔥 Combo Interactions (Multi-Layer Context)

### Interaction Chains

These create the most emotionally engaging experiences by combining multiple sensor layers:

#### 1. **Natural Approach → Boop**
```
ToF: Far → Curious → Focused → Anticipation
Touch: Single Boop
Result: Full happy sequence with maximum emotional payoff
```

**Implementation:**
```cpp
if (awarenessState == AwarenessState::ANTICIPATION && touchDetected) {
    // Maximum happiness - they followed the full interaction flow
    eyeState.setState(EyeStateEnum::ECSTATIC);
    fxState.setState(FXStateEnum::HEART_BURST);  // Extra hearts!
    fxState.setFlyingSpeed(1.0f);  // Fast hearts
    cheekPanel.pulse(3);  // Triple pulse
}
```

#### 2. **Gesture → Boop Combo**
```
ToF: Focused
Gesture: Swipe Up (excited)
Touch: Boop within 2 seconds
Result: "Powered up" celebration
```

**Implementation:**
```cpp
if (lastGesture == GestureType::SWIPE_UP &&
    (now - lastGestureTime) < 2000 &&
    touchDetected) {
    // Combo bonus!
    eyeState.setState(EyeStateEnum::SUPER_EXCITED);
    fxState.setState(FXStateEnum::SPARKLE_BURST);
    hornLED.setBrightness(100, 1);
    // TODO: Victory animation
}
```

#### 3. **Sudden Touch (No ToF Context)**
```
ToF: Idle (no awareness)
Touch: Sudden boop
Result: Startled → Happy
```

**Implementation:**
```cpp
if (awarenessState == AwarenessState::IDLE && touchDetected) {
    // Startled sequence
    eyeState.setState(EyeStateEnum::STARTLED);  // Eyes pop open
    delay(200);  // Brief pause
    eyeState.setState(EyeStateEnum::HAPPY);     // Then happy
    mouthState.setState(MouthStateEnum::SMILE);
}
```

#### 4. **Dismissive Gesture (No ToF)**
```
ToF: Far (>500mm)
Gesture: Any swipe
Result: Side-eye, minimal reaction
```

**Implementation:**
```cpp
if (awarenessState == AwarenessState::IDLE && gestureDetected) {
    // Dismissive response
    eyeState.setGlance(GLANCE_SIDEWAYS);
    // No other changes - "I saw that, but nah"
}
```

---

## 🧠 Internal Mood System

### Mood States

To prevent repetitive responses and create the illusion of personality:

```cpp
enum class MoodState {
    CALM,       // Default, balanced responses
    PLAYFUL,    // More energetic, faster animations
    SLEEPY,     // Slower responses, dimmer LEDs
    ANNOYED,    // After spam, less responsive
    EXCITED     // After positive interactions
};

class MoodSystem {
private:
    MoodState currentMood = MoodState::CALM;
    float moodDecayRate = 0.001f;  // Mood returns to calm over time
    unsigned long lastMoodChange = 0;

public:
    void update() {
        unsigned long now = millis();

        // Gradually return to calm
        if (currentMood != MoodState::CALM) {
            if (now - lastMoodChange > 30000) {  // 30 seconds
                transitionTo(MoodState::CALM);
            }
        }
    }

    void applyMoodModifier(EyeStateEnum& eyeState) {
        switch (currentMood) {
            case MoodState::PLAYFUL:
                // Same input, more energetic output
                if (eyeState == EyeStateEnum::HAPPY) {
                    eyeState = EyeStateEnum::EXCITED;
                }
                break;

            case MoodState::SLEEPY:
                // Muted responses
                if (eyeState == EyeStateEnum::EXCITED) {
                    eyeState = EyeStateEnum::HAPPY;
                }
                break;

            case MoodState::ANNOYED:
                // Less responsive
                if (eyeState == EyeStateEnum::CURIOUS) {
                    eyeState = EyeStateEnum::IDLE;  // Ignore
                }
                break;
        }
    }

    void onPositiveInteraction() {
        if (currentMood == MoodState::CALM) {
            transitionTo(MoodState::PLAYFUL);
        } else if (currentMood == MoodState::PLAYFUL) {
            transitionTo(MoodState::EXCITED);
        }
    }

    void onSpamDetected() {
        transitionTo(MoodState::ANNOYED);
    }

private:
    void transitionTo(MoodState newMood) {
        currentMood = newMood;
        lastMoodChange = millis();
    }
};
```

---

## ⚡ Priority Handling & Conflict Resolution

When multiple sensors trigger simultaneously, use this priority system:

### Priority Hierarchy (Highest to Lowest)

1. **Touch (Layer 3)** - Physical contact always wins
2. **Gesture (Layer 2)** - Active communication takes precedence
3. **ToF Awareness (Layer 1)** - Passive awareness is lowest priority
4. **Idle/Sleep** - Default state when no input

### Implementation Pattern

```cpp
void KMMXController::updateInteractions() {
    // Read all sensors (already done in sensor task)
    uint16_t distance = getToFDistance();
    GestureType gesture = getGesture();
    bool isTouching = getTouchState();

    // Priority cascade
    if (isTouching) {
        // Layer 3: Touch overrides everything
        touchLayer.update(isTouching, distance);
        return;  // Skip lower priority layers
    }

    if (gesture != GestureType::NONE) {
        // Layer 2: Gesture takes priority over awareness
        gestureLayer.update(gesture, distance);
        return;
    }

    // Layer 1: Awareness (only if no higher priority input)
    awarenessLayer.update(distance);

    // Apply mood modifiers to final expression
    moodSystem.update();
    moodSystem.applyMoodModifier(eyeState.getCurrentState());
}
```

### Interrupt Handling

Some interactions can interrupt others:

```cpp
// Touch can interrupt gesture animations
if (isTouching && gestureLayer.isAnimating()) {
    gestureLayer.interrupt();
    touchLayer.update(isTouching, distance);
}

// Spam detection interrupts everything
if (touchLayer.isSpamDetected()) {
    awarenessLayer.reset();
    gestureLayer.reset();
    // Show angry response
}
```

---

## 🔧 Integration with Existing Codebase

### Adding to KMMXController

```cpp
// In KMMXController.h
class KMMXController {
private:
    // Existing members...

    // New interaction layers
    AwarenessLayer awarenessLayer;
    GestureLayer gestureLayer;
    TouchLayer touchLayer;
    MoodSystem moodSystem;

    // Sensor data
    uint16_t tofDistance = 9999;
    GestureType currentGesture = GestureType::NONE;
    bool capacitiveTouched = false;

public:
    void updateInteractions();  // New method
};

// In KMMXController.cpp
void KMMXController::updateInteractions() {
    // Read sensor data from double-buffer
    tofDistance = sensorBuffer[activeBuffer].tofDistance;
    currentGesture = sensorBuffer[activeBuffer].gesture;
    capacitiveTouched = sensorBuffer[activeBuffer].touched;

    // Update layers with priority handling
    if (capacitiveTouched) {
        touchLayer.update(capacitiveTouched, tofDistance);
    } else if (currentGesture != GestureType::NONE) {
        gestureLayer.update(currentGesture, tofDistance);
    } else {
        awarenessLayer.update(tofDistance);
    }

    // Update mood system
    moodSystem.update();
}
```

### Updating SensorData Structure

```cpp
// In src/Types/SensorData.h
struct SensorData {
    // Existing fields
    float accelX, accelY, accelZ;
    float accelMagnitude;
    uint16_t proximity;  // Keep for backward compatibility

    // New fields for enhanced sensors
    uint16_t tofDistance;      // VL53L1X distance in mm
    GestureType gesture;       // PAJ7620U2 gesture
    bool touched;              // Capacitive touch state
    uint8_t touchElectrode;    // Which electrode was touched (MPR121)
};
```

### Sensor Task Updates

```cpp
// In ControllerSensor.cpp
void KMMXController::readSensorTask(void *parameter) {
    KMMXController* ctrl = static_cast<KMMXController*>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    constexpr TickType_t xFrequency = pdMS_TO_TICKS(sensorUpdateInterval);

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        uint8_t writeBuffer = 1 - ctrl->activeBuffer;

        // Read accelerometer (existing)
        ctrl->accel.getAcceleration(
            &ctrl->sensorBuffer[writeBuffer].accelX,
            &ctrl->sensorBuffer[writeBuffer].accelY,
            &ctrl->sensorBuffer[writeBuffer].accelZ
        );

        // Read ToF sensor (new)
        ctrl->sensorBuffer[writeBuffer].tofDistance = ctrl->tofSensor.readRangeSingleMillimeters();

        // Read gesture sensor (new) - poll at 30Hz (every other cycle)
        static uint8_t gesturePollCounter = 0;
        if (++gesturePollCounter >= 2) {
            gesturePollCounter = 0;
            ctrl->sensorBuffer[writeBuffer].gesture = ctrl->gestureSensor.readGesture();
        }

        // Read capacitive touch (new)
        ctrl->sensorBuffer[writeBuffer].touched = ctrl->touchSensor.touched();
        ctrl->sensorBuffer[writeBuffer].touchElectrode = ctrl->touchSensor.getTouchedElectrode();

        // Atomic buffer swap
        ctrl->activeBuffer = writeBuffer;
    }
}
```

---

## 📊 Performance Considerations

### CPU Load Estimates

| Task | Core | Frequency | CPU Time | % Load |
|------|------|-----------|----------|--------|
| Sensor Reading | Core 0 | 50Hz | ~2ms | ~10% |
| Gesture Processing | Core 0 | 30Hz | ~1ms | ~3% |
| Interaction Logic | Core 1 | 50Hz | ~0.5ms | ~2.5% |
| Rendering | Core 1 | Variable | ~1.3ms | ~5-10% |
| **Total** | - | - | - | **~20-25%** |

**Headroom:** ~75-80% CPU available for animations, audio, BLE, etc.

### Memory Usage

```cpp
// Approximate memory footprint
sizeof(AwarenessLayer)  ≈ 32 bytes
sizeof(GestureLayer)    ≈ 64 bytes (includes std::map)
sizeof(TouchLayer)      ≈ 24 bytes
sizeof(MoodSystem)      ≈ 16 bytes
sizeof(SensorData) * 2  ≈ 80 bytes (double buffer)
// Total: ~216 bytes additional RAM
```

---

## 🧪 Testing & Debugging

### Debug Output

```cpp
// Add to config.h
const bool enableInteractionDebug = true;

// In interaction update loop
#if enableInteractionDebug
void KMMXController::debugInteractions() {
    Serial.printf("ToF: %dmm | Gesture: %d | Touch: %d | Mood: %d\n",
        tofDistance,
        static_cast<int>(currentGesture),
        capacitiveTouched,
        static_cast<int>(moodSystem.getCurrentMood())
    );
}
#endif
```

### Test Scenarios

1. **Distance Transitions**
   - Move hand from 600mm → 0mm slowly
   - Verify smooth state transitions
   - Check timeout behavior

2. **Gesture Recognition**
   - Test each gesture type at optimal range (5-10cm)
   - Verify cooldown prevents spam
   - Test dismissive response when far away

3. **Touch Patterns**
   - Single tap
   - Long hold (2+ seconds)
   - Rapid spam (3+ taps in 1 second)
   - Verify cooldowns work

4. **Combo Interactions**
   - Approach slowly → gesture → boop
   - Verify enhanced responses
   - Test interrupt behavior

---

## 🎨 Future Enhancements

### Planned Features

- **Directional Awareness**: Use ToF sensor angle to track where person is standing
- **Multi-Person Detection**: Respond differently when multiple people approach
- **Learning System**: Remember frequent interactions and adapt responses
- **Sound Integration**: Add purring/chirping sounds for touch responses
- **Haptic Feedback**: Vibration motor for tactile responses
- **Advanced Gestures**: Custom gesture sequences (e.g., "secret handshake")

### Hardware Wishlist

- **VL53L5CX**: 8x8 zone ToF sensor for spatial awareness
- **DRV2605L**: Haptic motor driver for vibration feedback
- **MAX98357A**: I2S audio amplifier for sound effects
- **Additional MPR121**: Multiple touch zones (ears, cheeks, nose)

---

## 📚 References

- **VL53L1X Datasheet**: [STMicroelectronics](https://www.st.com/resource/en/datasheet/vl53l1x.pdf)
- **PAJ7620U2 Datasheet**: [PixArt Imaging](https://www.pixart.com/products-detail/10/PAJ7620U2)
- **ESP32-S3 Technical Reference**: [Espressif](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- **FreeRTOS Documentation**: [FreeRTOS.org](https://www.freertos.org/Documentation/RTOS_book.html)

---

## ✅ Implementation Checklist

- [ ] Order VL53L1X ToF sensor module
- [ ] Order PAJ7620U2 gesture sensor module
- [ ] Order MPR121 capacitive touch breakout (optional)
- [ ] Update `SensorData` structure with new fields
- [ ] Implement `AwarenessLayer` class
- [ ] Implement `GestureLayer` class
- [ ] Implement `TouchLayer` class (enhance existing boop logic)
- [ ] Implement `MoodSystem` class
- [ ] Add sensor initialization in `setup()`
- [ ] Update sensor task to read new sensors
- [ ] Integrate `updateInteractions()` into main loop
- [ ] Add new `EyeStateEnum` values (CURIOUS, FOCUSED, ANTICIPATION, etc.)
- [ ] Add new `MouthStateEnum` values as needed
- [ ] Create combo interaction handlers
- [ ] Test each layer independently
- [ ] Test combo interactions
- [ ] Tune timing constants based on real-world testing
- [ ] Add debug output for troubleshooting
- [ ] Document final sensor wiring in hardware guide

---

**Created:** 2026-02-04
**Status:** Implementation-ready documentation
**Next Step:** Order sensors and begin hardware integration

