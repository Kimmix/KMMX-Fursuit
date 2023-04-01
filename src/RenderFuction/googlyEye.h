// Original from: https://github.com/gamblor21/Googly_Eye/blob/main/googlyeye.py#L31
#include <Arduino.h>

class GooglyEye {
public:
    GooglyEye(float x = 0, float y = 0,
        float drag = 0.996,   // The amount of drag to apply to the eye movement. 
        // A higher value will cause the eye to slow down faster.
        float g_scale = 4,    // The scale factor to apply to the acceleration.
        // A higher value will cause the eye to move more in response to acceleration.
        float eye_radius = 3, // The radius of the eye.
        float screen_radius = 8, // The radius of the screen area.
        float elastic = 0.90  // The elasticity of the eye. A value of 1.0 means no elasticity, 
        // while a lower value will cause the eye to bounce back more slowly.
    ):
        x(x), y(y), vx(0.0), vy(0.0), last_update(0), drag(drag), g_scale(g_scale),
        eye_radius(eye_radius),
        _eye_radius2(eye_radius* eye_radius),
        screen_radius(screen_radius),
        _screen_radius2(screen_radius* screen_radius),
        _inner_radius(screen_radius - eye_radius),
        _inner_radius2(_inner_radius* _inner_radius),
        elastic(elastic) {}

    float x, y;

    void update(float ax, float ay) {
        unsigned long now = millis() / 1000;
        unsigned long dt = now - last_update;
        last_update = now;

        ax *= dt * g_scale;
        ay *= dt * g_scale;
        vx = (vx + ax) * drag;  // y direction is display -x
        vy = (vy + ay) * drag;  // x direction is display +y

        // runaway velocity check
        float v = vx * vx + vy * vy;
        if (v > _eye_radius2) {
            v = eye_radius / sqrt(v);
            vx *= v;
            vy *= v;
        }

        // this is where the eye should move to
        float new_x = x + vx;
        float new_y = y + vy;

        // But check if the eye will pass the screen border
        float d = new_x * new_x + new_y * new_y;
        if (d > _inner_radius2) {
            // Find the vector from current to new position that crosses screen border
            float dx = new_x - x;
            float dy = new_y - y;

            // find intersection point from the vector to the circle (screen)
            float n1 = 0.0, n2 = 0.0;
            float x2 = x * x;
            float y2 = y * y;
            float a2 = dx * dx;
            float b2 = dy * dy;
            float a2b2 = a2 + b2;
            float n = a2 * _inner_radius2 - a2 * y2 + 2.0 * dx * dy * x * y + b2 * _inner_radius2 - b2 * x2;
            if (n > 0.0 && a2b2 > 0.0) {
                n = sqrt(n);
                n1 = (n - dx * x - dy * y) / a2b2;
                n2 = -(n + dx * x + dy * y) / a2b2;
            }
            // use larger intersection point (there are two)
            if (n2 > n1) {
                n1 = n2;
            }

            // The single intersection point of movement vector and circle
            // That is where the eye will hit the circle
            float ix = x + dx * n1;
            float iy = y + dy * n1;

            // Calculate the bounce from the edge, which is the remainder of our velocity
            // and the opposite angle at which we intersected the circle
            float mag1 = sqrt(dx * dx + dy * dy);
            float dx1 = ix - x; // vector from prior pos
            float dy1 = iy - y; // to edge of circle
            float mag2 = sqrt(dx1 * dx1 + dy1 * dy1); // mag of that previous vector

            // Lose some energy in the bounce
            float mag3 = (mag1 - mag2) * elastic;

            float ax = -ix / _inner_radius;
            float ay = -iy / _inner_radius;
            float rx = 0.0, ry = 0.0;
            if (mag1 > 0.0) {
                rx = -dx / mag1;
                ry = -dy / mag1;
            }

            float dot = rx * ax + ry * ay;
            float rpx = ax * dot;
            float rpy = ay * dot;
            rx += (rpx - rx) * 2.0;  // reversed vector leaving the circle
            ry += (rpy - ry) * 2.0;

            // New position is where we hit the circle plus the reflected vector
            // scaled down by mag3 (which lowers velocity due to bounce)
            new_x = ix + rx * mag3;
            new_y = iy + ry * mag3;

            // Set our new velocity values for the next move
            mag1 *= elastic;
            vx = rx * mag1;
            vy = ry * mag1;
        }

        x = new_x;
        y = new_y;
    }

private:
    float vx, vy, last_update, drag, g_scale, eye_radius, _eye_radius2, screen_radius, _screen_radius2, _inner_radius, _inner_radius2, elastic;
};