// Original logic from: https://github.com/adafruit/Adafruit_Learning_System_Guides/blob/main/Hallowing_Googly_Eye/Hallowing_Googly_Eye.ino
// SPDX-FileCopyrightText: 2018 Phillip Burgess for Adafruit Industries
// SPDX-License-Identifier: MIT
#include <Arduino.h>

#define G_SCALE     40.0   // Accel scale; no science, just looks good
#define ELASTICITY  0.90  // Edge-bounce coefficient (MUST be <1.0!)
#define DRAG        0.996 // Dampens motion slightly
#define EYE_RADIUS  3.0  // Radius of eye, floating-point pixel units
#define PUPIL_SIZE  16.0
#define PUPIL_RADIUS (PUPIL_SIZE / 2.0)  // Radius of pupil, same units
// Pupil motion is computed as a single point constrained within a circle
// whose radius is the eye radius minus the pupil radius.
#define INNER_RADIUS (EYE_RADIUS - PUPIL_RADIUS)

class GooglyEye {
private:
    unsigned long lastTime = 0;
    float vx, vy = 0.0;
    bool firstFrame = true;  // Force full-screen update on initial frame

public:
    GooglyEye() {}

    float x, y;
    void update(float inputX, float inputY) {
        // Get time since last frame, in floating-point seconds
        unsigned long t = millis();
        float elapsed = (float)(t - lastTime) / 1000.0;
        lastTime = t;

        // Scale accelerometer readings based on an empirically-derived constant
        // (i.e. looks good, nothing scientific) and time since prior frame.
        // On HalloWing, accelerometer's Y axis is horizontal, X axis is vertical,
        // (vs screen's and conventional Cartesian coords being X horizontal,
        // Y vertical), so swap while we're here, store in ax, ay;
        float scale = G_SCALE * elapsed;
        float ax = inputY * scale, // Horizontal acceleration, pixel units
            ay = inputX * scale; // Vertical acceleration "

        // Add scaled accelerometer readings to pupil velocity, store interim
        // values in vxNew, vyNew...a little friction prevents infinite bounce.
        float vxNew = (vx + ax) * DRAG,
            vyNew = (vy + ay) * DRAG;

        // Limit velocity to pupil size to avoid certain overshoot situations
        float v = vxNew * vxNew + vyNew * vyNew;
        if (v > (PUPIL_SIZE * PUPIL_SIZE)) {
            v = PUPIL_SIZE / sqrt(v);
            vxNew *= v;
            vyNew *= v;
        }

        // Add new velocity to prior position, store interim in xNew, yNew;
        float xNew = x + vxNew,
            yNew = y + vyNew;

        // Get pupil position (center point) distance-squared from origin...
        // here's why we put (0,0) at the center...
        float d = xNew * xNew + yNew * yNew;

        // Is pupil heading out of the eye constraints?  No need for a sqrt()
        // yet...since we're just comparing against a constant at this point,
        // we can square the constant instead, avoid math...
        float r2 = INNER_RADIUS * INNER_RADIUS; // r^2
        if (d >= r2) {

            // New pupil center position is outside the circle, now the math
            // suddenly gets intense...

            float dx = xNew - x, // Vector from old to new position
                dy = yNew - y; // (crosses INNER_RADIUS perimeter)

            // Find intersections between unbounded line and circle...
            float x2 = x * x,  //  x^2
                y2 = y * y,  //  y^2
                a2 = dx * dx, // dx^2
                b2 = dy * dy, // dy^2
                a2b2 = a2 + b2,
                n1, n2,
                n = a2 * r2 - a2 * y2 + 2.0 * dx * dy * x * y + b2 * r2 - b2 * x2;
            if ((n >= 0.0) & (a2b2 > 0.0)) {
                // Because there's a square root here...
                n = sqrt(n);
                // There's two possible intersection points.  Consider both...
                n1 = (n - dx * x - dy * y) / a2b2;
                n2 = -(n + dx * x + dy * y) / a2b2;
            }
            else {
                n1 = n2 = 0.0; // Avoid divide-by-zero
            }
            // ...and use the 'larger' one (may be -0.0, that's OK!)
            if (n2 > n1) n1 = n2;
            float ix = x + dx * n1, // Single intersection point of
                iy = y + dy * n1; // movement vector and circle.

            // Pupil needs to be constrained within eye circle, but we can't just
            // stop it's motion at the edge, that's cheesy and looks wrong.  On its
            // way out, it was moving with a certain direction and speed, and needs
            // to bounce back in with suitable changes to both...

            float mag1 = sqrt(dx * dx + dy * dy), // Full velocity vector magnitude
                dx1 = (ix - x),                // Vector from prior pupil pos.
                dy1 = (iy - y),                // to point of edge intersection
                mag2 = sqrt(dx1 * dx1 + dy1 * dy1); // Magnitude of above vector
            // Difference between the above two magnitudes is the distance the pupil
            // will bounce back into the eye circle on this frame (i.e. it rarely
            // stops exactly at the edge...in the course of a single frame, it will
            // be moving outward a certain amount, contact edge, and move inward
            // a certain amount.  The latter amount is scaled back slightly as it
            // loses some energy in edge the collision.
            float mag3 = (mag1 - mag2) * ELASTICITY;

            float ax = -ix / INNER_RADIUS, // Unit surface normal (magnitude 1.0)
                ay = -iy / INNER_RADIUS, // at contact point with circle.
                rx, ry;                  // Reverse velocity vector, normalized
            if (mag1 > 0.0) {
                rx = -dx / mag1;
                ry = -dy / mag1;
            }
            else {
                rx = ry = 0.0;
            }
            // Dot product between the two vectors is cosine of angle between them
            float dot = rx * ax + ry * ay,
                rpx = ax * dot,          // Point to reflect across
                rpy = ay * dot;
            rx += (rpx - rx) * 2.0;        // Reflect velocity vector across point
            ry += (rpy - ry) * 2.0;        // (still normalized)

            // New position is the intersection point plus the reflected vector
            // scaled by mag3 (the elasticity-reduced velocity remainder).
            xNew = ix + rx * mag3;
            yNew = iy + ry * mag3;
            // Velocity magnitude is scaled by the elasticity coefficient.
            mag1 *= ELASTICITY;
            vxNew = rx * mag1;
            vyNew = ry * mag1;
        }
        int x1, y1, x2, y2,                        // Bounding rect of screen update area
            px1 = 64 + (int)xNew - PUPIL_SIZE / 2, // Bounding rect of new pupil pos. only
            px2 = 64 + (int)xNew + PUPIL_SIZE / 2 - 1,
            py1 = 64 - (int)yNew - PUPIL_SIZE / 2,
            py2 = 64 - (int)yNew + PUPIL_SIZE / 2 - 1;

        if (firstFrame) {
            x1 = y1 = 0;
            x2 = y2 = 127;
            firstFrame = false;
        }
        else {
            if (xNew >= x) { // Moving right
                x1 = 64 + (int)x - PUPIL_SIZE / 2;
                x2 = 64 + (int)xNew + PUPIL_SIZE / 2 - 1;
            }
            else {       // Moving left
                x1 = 64 + (int)xNew - PUPIL_SIZE / 2;
                x2 = 64 + (int)x + PUPIL_SIZE / 2 - 1;
            }
            if (yNew >= y) { // Moving up (still using +Y Cartesian coords)
                y1 = 64 - (int)yNew - PUPIL_SIZE / 2;
                y2 = 64 - (int)y + PUPIL_SIZE / 2 - 1;
            }
            else {        // Moving down
                y1 = 64 - (int)y - PUPIL_SIZE / 2;
                y2 = 64 - (int)yNew + PUPIL_SIZE / 2 - 1;
            }
        }

        x = xNew;  // Save new position, velocity
        y = yNew;
        vx = vxNew;
        vy = vyNew;

        // Clip update rect.  This shouldn't be necessary, but it looks
        // like very occasionally an off-limits situation may occur, so...
        if (x1 < 0)   x1 = 0;
        if (y1 < 0)   y1 = 0;
        if (x2 > 127) x2 = 127;
        if (y2 > 127) y2 = 127;
    }
};