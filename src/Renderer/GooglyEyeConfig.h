#pragma once

#include <stdint.h>

// ============================================================================
// GooglyEye Physics Configuration
// ============================================================================
// These constants control the physics simulation for the googly eye effect.
// Adjust these values to change how the pupil moves and bounces.
//
// Based on: https://github.com/adafruit/Adafruit_Learning_System_Guides
//           /blob/main/Hallowing_Googly_Eye/Hallowing_Googly_Eye.ino
// SPDX-FileCopyrightText: 2018 Phillip Burgess for Adafruit Industries
// SPDX-License-Identifier: MIT
// ============================================================================

// Physics parameters
const float googlyAccelScale = 1.0f;           // Acceleration scale factor (affects pupil responsiveness)
const float googlyElasticity = 0.95f;          // Edge-bounce coefficient - MUST be <1.0! (0.95 = slight energy loss on bounce)
const float googlyDrag = 0.997f;               // Velocity dampening factor (simulates air resistance)

// Size parameters (in pixels)
const float googlyEyeRadius = 3.0f;            // Radius of the eye boundary (floating-point pixel units)
const float googlyPupilSize = 16.0f;           // Diameter of the pupil (floating-point pixel units)

// Calculated values (automatically derived from size parameters)
const float googlyPupilRadius = googlyPupilSize / 2.0f;                      // Radius of pupil
const float googlyInnerRadius = googlyEyeRadius - googlyPupilRadius;         // Maximum pupil travel radius

// Pupil bitmap dimensions and position
const uint8_t googlyPupilWidth = 6;            // Width of the pupil bitmap
const uint8_t googlyPupilHeight = 6;           // Height of the pupil bitmap
const uint8_t googlyPupilOffsetY = 5;          // Vertical offset for pupil rendering

// Note: googlyPupilOffsetX is calculated in config.h based on eyeOffsetX
// to maintain proper positioning relative to the eye region
