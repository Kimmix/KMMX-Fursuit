#include "GooglyEye.h"
#include <Arduino.h>

void GooglyEye::update(float inputX, float inputY) {
    unsigned long t = millis();
    float elapsed = (float)(t - lastTime) / 1000.0;
    lastTime = t;

    float scale = G_SCALE * elapsed;
    float ax = inputY * scale, ay = inputX * scale;

    float vxNew = (vx + ax) * DRAG, vyNew = (vy + ay) * DRAG;

    float v = vxNew * vxNew + vyNew * vyNew;
    if (v > (PUPIL_SIZE * PUPIL_SIZE)) {
        v = PUPIL_SIZE / sqrt(v);
        vxNew *= v;
        vyNew *= v;
    }

    float xNew = x + vxNew, yNew = y + vyNew;

    float d = xNew * xNew + yNew * yNew;
    float r2 = INNER_RADIUS * INNER_RADIUS;
    if (d >= r2) {
        float dx = xNew - x, dy = yNew - y;

        float x2 = x * x, y2 = y * y, a2 = dx * dx, b2 = dy * dy, a2b2 = a2 + b2,
              n1, n2, n = a2 * r2 - a2 * y2 + 2.0 * dx * dy * x * y + b2 * r2 - b2 * x2;
        if ((n >= 0.0) & (a2b2 > 0.0)) {
            n = sqrt(n);
            n1 = (n - dx * x - dy * y) / a2b2;
            n2 = -(n + dx * x + dy * y) / a2b2;
        } else {
            n1 = n2 = 0.0;
        }
        if (n2 > n1) n1 = n2;
        float ix = x + dx * n1, iy = y + dy * n1;

        float mag1 = sqrt(dx * dx + dy * dy), dx1 = (ix - x), dy1 = (iy - y), mag2 = sqrt(dx1 * dx1 + dy1 * dy1);
        float mag3 = (mag1 - mag2) * ELASTICITY;

        float ax = -ix / INNER_RADIUS, ay = -iy / INNER_RADIUS, rx, ry;
        if (mag1 > 0.0) {
            rx = -dx / mag1;
            ry = -dy / mag1;
        } else {
            rx = ry = 0.0;
        }
        float dot = rx * ax + ry * ay, rpx = ax * dot, rpy = ay * dot;
        rx += (rpx - rx) * 2.0;
        ry += (rpy - ry) * 2.0;

        xNew = ix + rx * mag3;
        yNew = iy + ry * mag3;
        mag1 *= ELASTICITY;
        vxNew = rx * mag1;
        vyNew = ry * mag1;
    }
    x = xNew;
    y = yNew;
    vx = vxNew;
    vy = vyNew;
}
