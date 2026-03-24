#pragma once
#include <Arduino.h>

#include "config.h"
#include "Utils/Utils.h"

#include "FacialStates/EyeState/EyeState.h"
#include "FacialStates/MouthState/MouthState.h"
#include "FacialStates/FXState/FXState.h"

#include "Renderer/Boop.h"

#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Devices/HornLED/HornLED.h"
#include "Devices/Accelerometer/LIS3DH.h"
#include "Devices/Proximity/APDS9930Sensor.h"
#include "Devices/Ws2812/RGBStatus.h"
#include "Devices/Ws2812/CheekPanel.h"
#include "Types/SensorData.h"

class KMMXController {
   public:
    void setupSensors();
    void update();
    void setEye(int i);
    void setMouth(int i);
    void setViseme(int b);
    int getViseme();
    int getDisplayBrightness();
    void setDisplayBrightness(int i);
    int getHornBrightness();
    void setHornBrightness(int i);
    int getCheekBrightness();
    void setCheekBrightness(int i);
    void setCheekBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
    void setCheekFadeColor(uint8_t r, uint8_t g, uint8_t b);
    uint32_t getCheekBackgroundColor();
    uint32_t getCheekFadeColor();
    void reboot();

   private:
    // Devices
    Hub75DMA display;
    RGBStatus statusLED = RGBStatus(RGB_STATUS_PIN);
    CheekPanel cheekPanel = CheekPanel(argbCount, ARGB_PIN);
    HornLED hornLED;
    LIS3DH accelerometer;
    APDS9930Sensor proximitySensor;
    // Double-buffer for thread-safe sensor access
    SensorData sensorBuffer[2];
    volatile uint8_t activeBuffer = 0;
    TaskHandle_t sensorTaskHandle;
    TaskHandle_t renderTaskHandle;
    // Renderer states
    EyeState eyeState = EyeState(&display);
    MouthState mouthState = MouthState(&display);
    FXState fxState = FXState(&display);
    Boop boop;
    int16_t pixelPos = 0;

    void renderFace();
    void handleBoop();
    void resetIdleTime(KMMXController *controller);
    void resetIdleTime();
    void enterSleep(KMMXController *controller);
    void checkIdleAndSleep(KMMXController *controller, unsigned long currentTime);
    static void readSensorTask(void *parameter);
    static void renderTask(void *parameter);
    const SensorData& getSensorData() const;

    // Motion detection methods
    void checkMotionFeatures(KMMXController *controller);
    // detectShake - REMOVED
    void detectTilt(const SensorData& current);
    // detectBounce - REMOVED
    // detectSpin - REMOVED
    void detectPetting(const SensorData& current);
    // triggerShakeResponse - REMOVED
    void triggerTiltResponse(float angle, bool isLeftRight);
    // triggerBounceResponse - REMOVED
    // triggerSpinResponse - REMOVED
    void triggerPettingResponse(bool sustained);

    // Previous sensor values for idle detection
    SensorData prevSensorData;
    SensorData baselineAccel;  // Baseline acceleration when motion was last detected
    unsigned long stillTime = 0;  // Time when the accelerometer became still
    uint8_t motionCounter = 0;  // Counter for hysteresis (sustained motion detection)
    bool baselineInitialized = false;  // Whether baseline has been set
    unsigned long nextFrame;
    unsigned long nextBoop = 0;
    bool isSleeping = false;
    bool boopInitialized = false, inBoopRange = false, isBooping = false, isContinuousBoop = false, isAngry = false;
    unsigned short prevHornBright = hornInitBrightness;
    float boopSpeed = 0.0f;
    unsigned long motionDetectionStartTime = 0;  // Time when motion detection should start (after startup delay)

    // Motion detection state structures
    // ShakeDetector - REMOVED

    struct TiltDetector {
        float tiltAngleX = 0.0f;  // Forward/back tilt
        float tiltAngleZ = 0.0f;  // Left/right tilt
        unsigned long tiltStartTime = 0;
        unsigned long lastTiltChangeTime = 0;
        unsigned long lastForwardBackTime = 0;  // Track last forward/back tilt time
        unsigned long lastNeutralReturnTime = 0;  // Track when tilt returns to neutral (for petting cooldown)
        bool isTilted = false;
        bool isLeftRight = false;  // true = left/right, false = forward/back
        EyeStateEnum previousEyeState = EyeStateEnum::IDLE;
        MouthStateEnum previousMouthState = MouthStateEnum::IDLE;
    } tiltDetector;

    // BounceDetector - REMOVED
    // SpinDetector - REMOVED

    struct PettingDetector {
        float oscillationHistory[20];  // Track oscillation pattern
        uint8_t historyIndex = 0;
        unsigned long pettingStartTime = 0;
        unsigned long lastPettingTime = 0;
        bool isPetting = false;
        bool isSustained = false;  // Has been petting for extended time
        float averageFrequency = 0.0f;
        EyeStateEnum previousEyeState = EyeStateEnum::IDLE;
        MouthStateEnum previousMouthState = MouthStateEnum::IDLE;
    } pettingDetector;
};