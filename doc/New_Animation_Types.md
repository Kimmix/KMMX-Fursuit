# New Expressive Animation Types

This document describes the new animation easing types added to the TimeBasedAnimation system to make your fursuit more alive and reactive!

## Overview

We've added 7 new expressive animation types with unique personalities and behaviors. Each one creates a different feel and can be used for different emotional states or reactions.

## New Easing Types

### 1. BOUNCE_OVERSHOOT
**Feel:** Alive and reactive! Bouncy and energetic.

**Behavior:** 
- Overshoots to ~150% at the midpoint
- Then settles back to 100% at the end
- Creates that satisfying "bounce" feeling

**Best for:** 
- Reactions to interactions
- Excited movements
- Making things feel responsive and alive

**Presets:**
- `CONFIG_BOUNCE_OVERSHOOT` - 600ms duration
- `CONFIG_BOUNCE_OVERSHOOT_FAST` - 350ms duration (snappier)

---

### 2. ANTICIPATION
**Feel:** Adds personality to reactions!

**Behavior:**
- Pulls back to ~-20% (anticipation)
- Then launches forward to 100%
- Classic animation principle

**Best for:**
- Reaction animations
- Before a big movement
- Adding character and personality

**Preset:**
- `CONFIG_ANTICIPATION` - 500ms duration

---

### 3. ELASTIC_SNAP
**Feel:** Fun and bouncy with multiple wiggles!

**Behavior:**
- Quick snap with multiple small bounces
- Uses damped sine wave for elastic effect
- Settles after several oscillations

**Best for:**
- Playful interactions
- Springy, elastic movements
- Fun, energetic moments

**Preset:**
- `CONFIG_ELASTIC_SNAP` - 700ms duration

---

### 4. EXCITED_PULSE
**Feel:** Excitement gradually calming down!

**Behavior:**
- Rapid pulsing at the start
- Gradually slows down and calms
- Multiple bounces with exponential decay

**Best for:**
- Showing excitement that calms
- After receiving attention
- Energetic to calm transitions

**Preset:**
- `CONFIG_EXCITED_PULSE` - 1200ms duration

---

### 5. CURIOUS_PEEK
**Feel:** Peeking at something interesting!

**Behavior:**
- Slow start (cautious)
- Quick middle (looking)
- Slow end (settling)

**Best for:**
- Curiosity behaviors
- Looking at something
- Gentle, inquisitive movements

**Preset:**
- `CONFIG_CURIOUS_PEEK` - 800ms duration

---

### 6. STARTLED_JUMP
**Feel:** Startled reaction!

**Behavior:**
- Instant jump to 100% (first 10%)
- Slow settle back with slight overshoot
- Perfect for surprise reactions

**Best for:**
- Startled/surprised reactions
- Quick responses to unexpected events
- Jump scares or boops

**Preset:**
- `CONFIG_STARTLED_JUMP` - 400ms duration

---

### 7. BREATHING_NATURAL
**Feel:** Natural, organic breathing!

**Behavior:**
- Asymmetric timing
- Inhale faster (40% of time)
- Exhale slower (60% of time)
- More realistic than symmetric breathing

**Best for:**
- Idle breathing animations
- Calm, resting states
- Natural, organic movement

**Preset:**
- `CONFIG_BREATHING` - Now uses this easing! (2000ms with pauses)
- `CONFIG_BREATHING_SLOW` - Also uses this! (5000ms with longer pauses)

---

## Usage Examples

```cpp
// Example 1: Bouncy reaction to a boop
TimeBasedAnimation::setConfig(myAnim, TimeBasedAnimation::CONFIG_BOUNCE_OVERSHOOT_FAST);

// Example 2: Anticipation before a big smile
TimeBasedAnimation::setConfig(smileAnim, TimeBasedAnimation::CONFIG_ANTICIPATION);

// Example 3: Excited pulse when getting attention
TimeBasedAnimation::setConfig(eyeAnim, TimeBasedAnimation::CONFIG_EXCITED_PULSE);

// Example 4: Startled jump when surprised
TimeBasedAnimation::setConfig(reactionAnim, TimeBasedAnimation::CONFIG_STARTLED_JUMP);

// Example 5: Natural breathing while idle
TimeBasedAnimation::setConfig(idleAnim, TimeBasedAnimation::CONFIG_BREATHING);
```

## Technical Details

All easing functions are implemented in `TimeBasedAnimation::applyEasing()` and work by transforming the linear progress (0.0 to 1.0) into a curved progress value. Some easings (like BOUNCE_OVERSHOOT and ANTICIPATION) can produce values outside the 0.0-1.0 range, which is intentional and creates the overshoot effects.

## Migration Notes

The old `useEasing` boolean has been replaced with `easingType` enum:
- `useEasing = false` → `easingType = EasingType::NONE`
- `useEasing = true` → `easingType = EasingType::EASE_IN_OUT`

All existing presets have been updated automatically.

