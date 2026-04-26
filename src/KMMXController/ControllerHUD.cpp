#include "KMMXController.h"
#include "Network/BLE.h"
#include "Utils/Utils.h"  // Use optimized utility functions

extern BLEManager& bleManager;

// OLED Layout Constants
namespace OLEDLayout {
    // Update settings
    constexpr unsigned long UPDATE_INTERVAL = 50;  // 20Hz

    // Face mirror positions (top half)
    constexpr int FACE_EYE_X = 0;
    constexpr int FACE_EYE_Y = 0;
    constexpr int FACE_NOSE_X = 50;
    constexpr int FACE_NOSE_Y = 7;
    constexpr int FACE_MOUTH_X = 7;
    constexpr int FACE_MOUTH_Y = 18;

    // Accelerometer (bottom left, 28 pixel height)
    constexpr int ACCEL_CENTER_X = 19;
    constexpr int ACCEL_CENTER_Y = 50;
    constexpr int ACCEL_RANGE = 13;
    constexpr int ACCEL_MIN_X = 6;
    constexpr int ACCEL_MAX_X = 32;
    constexpr int ACCEL_MIN_Y = 37;
    constexpr int ACCEL_MAX_Y = 63;
    constexpr int ZBAR_X = 0;
    constexpr int ZBAR_WIDTH = 6;

    // Sensor bars positions (bottom right)
    constexpr int BAR_LABEL_X = 38;
    constexpr int BAR_START_X = 48;
    constexpr int BAR_WIDTH = 80;
    constexpr int BAR_HEIGHT = 6;
    constexpr int PROX_BAR_Y = 37;
    constexpr int PROX_LABEL_Y = 43;
    constexpr int VU_BAR_Y = 46;
    constexpr int VU_LABEL_Y = 52;
    constexpr int VISEME_Y = 60;

    // Bluetooth icon (top right)
    constexpr int BT_X = 118;
    constexpr int BT_Y = 2;
    constexpr int BT_SIZE = 10;

    // State names (top half)
    constexpr int STATE_LABEL_X = 68;
    constexpr int STATE_VALUE_X = 80;
    constexpr int EYE_STATE_Y = 8;
    constexpr int MOUTH_STATE_Y = 18;

    // FPS counter (top right, below states)
    constexpr int FPS_X = 68;
    constexpr int FPS_Y = 28;

    // Sensor value ranges
    constexpr int SENSOR_MIN = 0;
    constexpr int SENSOR_MAX = 1023;
    constexpr float GRAVITY = 9.8f;
}

// State name lookup tables
namespace StateNames {
    const char* getEyeStateName(EyeStateEnum state) {
        switch (state) {
            case EyeStateEnum::IDLE:    return "IDLE";
            case EyeStateEnum::BLINK:   return "BLINK";
            case EyeStateEnum::BOOP:    return "BOOP";
            case EyeStateEnum::ARROW:   return "ARROW";
            case EyeStateEnum::GOOGLY:  return "GOOGLY";
            case EyeStateEnum::OEYE:    return "OEYE";
            case EyeStateEnum::HEART:   return "HEART";
            case EyeStateEnum::SMILE:   return "SMILE";
            case EyeStateEnum::ANGRY:   return "ANGRY";
            case EyeStateEnum::SLEEP:   return "SLEEP";
            case EyeStateEnum::SAD:     return "SAD";
            case EyeStateEnum::CRY:     return "CRY";
            case EyeStateEnum::DOUBTED: return "DOUBT";
            case EyeStateEnum::ROUNDED: return "ROUND";
            case EyeStateEnum::SHARP:   return "SHARP";
            default:                    return "?";
        }
    }

    const char* getMouthStateName(MouthStateEnum state) {
        switch (state) {
            case MouthStateEnum::IDLE:      return "IDLE";
            case MouthStateEnum::BOOP:      return "BOOP";
            case MouthStateEnum::ANGRYBOOP: return "ANGRY";
            case MouthStateEnum::TALKING:   return "TALK";
            case MouthStateEnum::WAH:       return "WAH";
            case MouthStateEnum::EH:        return "EH";
            case MouthStateEnum::POUT:      return "POUT";
            case MouthStateEnum::DROOLING:  return "DROOL";
            default:                        return "?";
        }
    }

    const char* getVisemeName(Viseme::VisemeType viseme) {
        switch (viseme) {
            case Viseme::AH: return "AH";
            case Viseme::EE: return "EE";
            case Viseme::OH: return "OH";
            case Viseme::OO: return "OO";
            case Viseme::TH: return "TH";
            default:         return "n/a";
        }
    }
}

// Helper function to draw a horizontal bar
namespace {
    void drawHorizontalBar(SSD1306& display, int x, int y, int width, int height, int level) {
        display.drawFrame(x, y, width, height);
        if (level > 0) {
            display.drawBox(x + 1, y + 1, level - 2, height - 2);
        }
    }
}

void KMMXController::updateOLED() {
    static unsigned long lastOLEDUpdate = 0;

    unsigned long currentTime = millis();
    if (currentTime - lastOLEDUpdate < OLEDLayout::UPDATE_INTERVAL) {
        return;
    }
    lastOLEDUpdate = currentTime;

    oledDisplay.clear();

    const SensorData& sensors = getSensorData();

    // Draw all HUD components
    drawOLEDFaceMirror(sensors);
    drawOLEDBluetooth();
    drawOLEDStateNames();
    drawOLEDFPS();
    drawOLEDSensorBars(sensors);
    drawOLEDAccelerometer(sensors);

    oledDisplay.update();
}

void KMMXController::drawOLEDFaceMirror(const SensorData& sensors) {
    using namespace OLEDLayout;

    // Get last rendered bitmaps from display (automatically tracked)
    const uint8_t* eyeBitmap = display.getLastEyeBitmap();
    const uint8_t* mouthBitmap = display.getLastMouthBitmap();

    // Draw eye
    if (eyeBitmap) {
        oledDisplay.drawBitmap(FACE_EYE_X, FACE_EYE_Y, eyeBitmap, eyeWidth, eyeHeight);
    }

    // Draw nose
    oledDisplay.drawBitmap(FACE_NOSE_X, FACE_NOSE_Y, noseNew, noseWidth, noseHeight);

    // Draw mouth
    if (mouthBitmap) {
        oledDisplay.drawBitmap(FACE_MOUTH_X, FACE_MOUTH_Y, mouthBitmap, mouthWidth, mouthHeight);
    }
}

void KMMXController::drawOLEDBluetooth() {
    if (!bleManager.isConnected()) {
        return;
    }

    using namespace OLEDLayout;
    auto* u8g2 = oledDisplay.getU8g2();

    // Central vertical line (spine)
    u8g2->drawLine(BT_X, BT_Y, BT_X, BT_Y + BT_SIZE);

    // Top-right diagonal
    u8g2->drawLine(BT_X, BT_Y, BT_X + 4, BT_Y + 3);

    // Middle-right to top diagonal
    u8g2->drawLine(BT_X + 4, BT_Y + 3, BT_X, BT_Y + 5);

    // Middle-right to bottom diagonal
    u8g2->drawLine(BT_X, BT_Y + 5, BT_X + 4, BT_Y + 7);

    // Bottom-right diagonal
    u8g2->drawLine(BT_X + 4, BT_Y + 7, BT_X, BT_Y + BT_SIZE);

    // Left side cross-diagonals
    u8g2->drawLine(BT_X - 4, BT_Y + 3, BT_X, BT_Y + 5);
    u8g2->drawLine(BT_X, BT_Y + 5, BT_X - 4, BT_Y + 7);
}

void KMMXController::drawOLEDStateNames() {
    using namespace OLEDLayout;

    const char* eyeStateName = StateNames::getEyeStateName(eyeState.getState());
    const char* mouthStateName = StateNames::getMouthStateName(mouthState.getState());

    oledDisplay.setFont(u8g2_font_5x8_tr);
    oledDisplay.drawText(STATE_LABEL_X, EYE_STATE_Y, "E:");
    oledDisplay.drawText(STATE_VALUE_X, EYE_STATE_Y, eyeStateName);
    oledDisplay.drawText(STATE_LABEL_X, MOUTH_STATE_Y, "M:");
    oledDisplay.drawText(STATE_VALUE_X, MOUTH_STATE_Y, mouthStateName);
}

void KMMXController::drawOLEDFPS() {
    using namespace OLEDLayout;

    float currentFPS = getFPS();

    oledDisplay.setFont(u8g2_font_5x8_tr);

    // Format: "FPS:240"
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "FPS:%d", (int)currentFPS);
    oledDisplay.drawText(FPS_X, FPS_Y, fpsText);
}

void KMMXController::drawOLEDSensorBars(const SensorData& sensors) {
    using namespace OLEDLayout;

    oledDisplay.setFont(u8g2_font_5x8_tr);

    // Proximity bar
    oledDisplay.drawText(BAR_LABEL_X, PROX_LABEL_Y, "P:");
    if (boopInitialized) {
        // Proximity sensor initialized - show bar
        int proxLevel = fastMap<int>(sensors.proximity, SENSOR_MIN, SENSOR_MAX, 0, BAR_WIDTH);
        drawHorizontalBar(oledDisplay, BAR_START_X, PROX_BAR_Y, BAR_WIDTH, BAR_HEIGHT, proxLevel);
    } else {
        // Proximity sensor not initialized - show "n/a"
        oledDisplay.drawText(BAR_START_X, PROX_LABEL_Y, "n/a");
    }

    // VU meter bar (uses loudness from viseme/microphone)
    oledDisplay.drawText(BAR_LABEL_X, VU_LABEL_Y, "VU");
    if (mouthState.getState() == MouthStateEnum::TALKING) {
        // Microphone is active - show VU meter based on loudness (0-20 range from viseme)
        uint16_t loudness = mouthState.viseme.getLoudness();
        int vuLevel = fastMap<int>(loudness, 0, 20, 0, BAR_WIDTH);
        drawHorizontalBar(oledDisplay, BAR_START_X, VU_BAR_Y, BAR_WIDTH, BAR_HEIGHT, vuLevel);
    } else {
        // Microphone not active - show "n/a"
        oledDisplay.drawText(BAR_START_X, VU_LABEL_Y, "n/a");
    }

    // Viseme text
    const char* visemeText = "n/a";
    if (mouthState.getState() == MouthStateEnum::TALKING) {
        visemeText = StateNames::getVisemeName(mouthState.viseme.getCurrentViseme());
    }

    oledDisplay.drawText(BAR_LABEL_X, VISEME_Y, "V:");
    oledDisplay.drawText(BAR_LABEL_X + 10, VISEME_Y, visemeText);
}

void KMMXController::drawOLEDAccelerometer(const SensorData& sensors) {
    using namespace OLEDLayout;
    auto* u8g2 = oledDisplay.getU8g2();

    // Map accel to screen coordinates (invert Y for intuitive display)
    int crosshairX = ACCEL_CENTER_X + (int)(sensors.accelX * ACCEL_RANGE / GRAVITY);
    int crosshairY = ACCEL_CENTER_Y - (int)(sensors.accelZ * ACCEL_RANGE / GRAVITY);

    // Clamp to screen bounds (optimized fastClamp)
    crosshairX = fastClamp<int>(crosshairX, ACCEL_MIN_X, ACCEL_MAX_X);
    crosshairY = fastClamp<int>(crosshairY, ACCEL_MIN_Y, ACCEL_MAX_Y);

    // Draw crosshair box
    int boxSize = (ACCEL_RANGE + 1) * 2;
    oledDisplay.drawFrame(ACCEL_CENTER_X - ACCEL_RANGE - 1,
                          ACCEL_CENTER_Y - ACCEL_RANGE - 1,
                          boxSize, boxSize);

    // Draw crosshair lines
    u8g2->drawLine(crosshairX - 3, crosshairY, crosshairX + 3, crosshairY);
    u8g2->drawLine(crosshairX, crosshairY - 3, crosshairX, crosshairY + 3);

    // Z-axis vertical bar
    int zBarHeight = boxSize;
    int zBarY = ACCEL_CENTER_Y - ACCEL_RANGE - 1;
    int zBarCenter = zBarHeight / 2;
    int zBarPos = zBarCenter - (int)(sensors.accelZ * zBarCenter / GRAVITY);
    zBarPos = constrain(zBarPos, 0, zBarHeight - 2);

    // Draw Z-bar
    oledDisplay.drawFrame(ZBAR_X, zBarY, ZBAR_WIDTH, zBarHeight);
    u8g2->drawLine(ZBAR_X, zBarY + zBarCenter, ZBAR_X + ZBAR_WIDTH - 1, zBarY + zBarCenter);
    oledDisplay.drawBox(ZBAR_X + 1, zBarY + zBarPos, ZBAR_WIDTH - 2, 3);
}
