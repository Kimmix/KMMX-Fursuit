#pragma once

#include <stdint.h>

// ============================================================================
// Motion Detection Configuration
// ============================================================================
// These constants configure various motion detection features that use the
// accelerometer to detect user interactions like tilting, petting, tapping,
// and upside-down orientation. Adjust these values to tune the sensitivity
// and behavior of each detection mode.
// ============================================================================

// Global Motion Detection Settings
const bool enableMotionDebug = true;                            // Enable detailed debug logging for all motion detection

// ============================================================================
// Tilt Detection - Detects sustained head tilt for curious/confused expressions
// ============================================================================
const bool enableTiltDetection = false;                         // Enable/disable tilt detection feature
const float tiltThreshold = 4.0f;                               // m/s² threshold for tilt detection (reduced for easier triggering)
const float tiltNeutralThreshold = 3.0f;                        // m/s² threshold to return to neutral (increased for easier return)
const uint16_t tiltSustainTime = 500;                           // Time to hold tilt before triggering (ms)
const uint16_t tiltDebounceTime = 300;                          // Cooldown between tilt changes (ms)
const uint16_t tiltDirectionChangeCooldown = 3000;              // Cooldown when switching from left/right to forward/back (ms)

// ============================================================================
// Upside Down Detection - Detects when the character is held upside down
// ============================================================================
const bool enableUpsideDownDetection = true;                    // Enable/disable upside-down detection feature
const float upsideDownThreshold = -7.0f;                        // m/s² threshold for Y-axis (negative = upside down)
const uint16_t upsideDownSustainTime = 1500;                    // Time to hold upside down before triggering (ms)
const uint16_t upsideDownDebounceTime = 500;                    // Cooldown between state changes (ms)

// ============================================================================
// Petting Detection - Detects quick pats/taps (spike-based) for contentment
// ============================================================================
const bool enablePettingDetection = true;                       // Enable/disable petting detection feature
const bool enablePettingCooldownAfterTilt = true;               // Enable cooldown after tilt to prevent immediate petting
const uint16_t pettingCooldownAfterTilt = 1000;                 // Cooldown duration after tilt ends (ms)

const float pettingSpikeThreshold = 0.8f;                       // m/s² threshold for detecting a gentle pat/tap spike
const uint16_t pettingSpikeCooldown = 300;                      // Minimum time between individual spikes (ms) - prevents double-counting

// Dynamic happiness system parameters
const float pettingHappinessPerPat = 20.0f;                     // Happiness added per pat (0-100 scale)
const float pettingHappinessTrigger = 80.0f;                    // Happiness level to trigger SMILE response (0-100)
const float pettingHappinessDecayRate = 15.0f;                  // Happiness decay per second when not petting
const float pettingHappinessEndThreshold = 20.0f;               // Happiness level below which response ends (allows natural fade-out)
const float pettingDeltaTimeMax = 2.0f;                         // Maximum delta time in seconds to accept (sanity check for time jumps)

// ============================================================================
// Tap Detection - Detects light taps for glitch effects
// ============================================================================
const bool enableTapDetection = true;                           // Enable/disable tap detection feature
const float tapSpikeThreshold = 1.0f;                           // m/s² threshold for detecting a light tap (higher than petting)
const uint16_t tapCooldown = 100;                               // Minimum time between taps (ms)

// Tap glitch effect scaling based on tap magnitude
const uint16_t tapGlitchMinDuration = 300;                      // Minimum duration of glitch effect (ms) for light taps
const uint16_t tapGlitchMaxDuration = 1200;                     // Maximum duration of glitch effect (ms) for hard taps
const int tapGlitchMinIntensity = 10;                           // Minimum intensity of glitch effect (0-100) for light taps
const int tapGlitchMaxIntensity = 60;                           // Maximum intensity of glitch effect (0-100) for hard taps
const float tapMagnitudeMin = 1.0f;                             // Minimum tap magnitude for scaling (m/s²)
const float tapMagnitudeMax = 5.0f;                             // Maximum tap magnitude for scaling (m/s²)
const uint16_t tapGlitchUpdateInterval = 75;                    // How often glitch pattern changes (ms)
const int tapGlitchFullScreenChance = 15;                       // Chance (0-100) for full-screen glitch instead of localized

// ============================================================================
// Idle Detection Configuration
// ============================================================================
// Settings for detecting when the device is idle and putting it to sleep
// ============================================================================
const uint32_t idleTimeout = 300000;                            // Time till sleep in ms (300 seconds = 5 minutes)
const uint16_t baselineUpdateDelay = 2000;                      // Time to wait before updating baseline when still (2 seconds)
const float idleAccThreshold = 1.5f;                            // Magnitude change threshold to detect motion (m/s²)
const float sleepingAccThreshold = 0.7f;                        // Magnitude change threshold to wake up from sleep (m/s²)
const uint8_t motionHysteresisCount = 3;                        // Number of motion detections needed to wake up
const uint8_t motionCounterDecayRate = 10;                      // Samples between counter decay (10 = ~200ms at 50Hz)
const float gravityMagnitude = 9.81f;                           // Earth's gravity for reference (m/s²)
