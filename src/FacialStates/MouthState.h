#include "Bitmaps/mouthBitmap.h"
#include "Bitmaps/mouthUp.h"
#include "Bitmaps/mouthDown.h"
#include "Bitmaps/mouthAnimated.h"
#include "RenderFunction/Viseme.h"
enum class MouthStateEnum { IDLE,
                            BOOP,
                            TALKING };
class MouthState {
   public:
    MouthState(LEDMatrixDisplay* display) : display(display) {}
    Viseme viseme;

    void startMic() {
        viseme.initMic();
    }

    void update() {
        updateAnimation();
        movingMouth();
        switch (currentState) {
            case MouthStateEnum::IDLE:
                drawDefault();
                break;
            case MouthStateEnum::BOOP:
                display->drawMouth(mouthOpen);
                if (millis() - resetBoop >= 700) {
                    currentState = prevState;
                }
                break;
            case MouthStateEnum::TALKING:
                if (!visemeTaskRunning) {
                    startVisemeTask();
                }
                if (visemeFrame == mouthDefault) {
                    drawDefault();
                } else {
                    display->drawMouth(visemeFrame);
                }
                break;
            default:
                currentState = MouthStateEnum::IDLE;
                break;
        }
    }

    void setState(MouthStateEnum newState) {
        savePrevState(currentState);
        if (newState == MouthStateEnum::BOOP) {
            resetBoop = millis();
        }
        currentState = newState;
    }

    void savePrevState(MouthStateEnum newState) {
        if (newState == MouthStateEnum::BOOP) {
            return;
        }
        prevState = newState;
    }

    MouthStateEnum getState() const {
        return currentState;
    }

    void getListEvent(const sensors_event_t& eventData) {
        event = eventData;
    }

   private:
    LEDMatrixDisplay* display;
    sensors_event_t event;
    MouthStateEnum prevState, currentState = MouthStateEnum::TALKING;
    unsigned long mouthInterval;
    const uint8_t *visemeFrame = mouthDefault, *mouthFrame = mouthDefault;

    const uint8_t* defaultAnimation[20] = {
        mouthDefault1,
        mouthDefault2,
        mouthDefault3,
        mouthDefault4,
        mouthDefault5,
        mouthDefault6,
        mouthDefault7,
        mouthDefault8,
        mouthDefault9,
        mouthDefault10,
        mouthDefault11,
        mouthDefault12,
        mouthDefault13,
        mouthDefault14,
        mouthDefault15,
        mouthDefault16,
        mouthDefault17,
        mouthDefault18,
        mouthDefault19,
        mouthDefault20,
    };
    void drawDefault() {
        display->drawMouth(mouthFrame);
    }

    const uint8_t* mouthUp[20] = {
        mouthUp1,
        mouthUp2,
        mouthUp3,
        mouthUp4,
        mouthUp5,
        mouthUp6,
        mouthUp7,
        mouthUp8,
        mouthUp9,
        mouthUp10,
        mouthUp11,
        mouthUp12,
        mouthUp13,
        mouthUp14,
        mouthUp15,
        mouthUp16,
        mouthUp17,
        mouthUp18,
        mouthUp19,
        mouthUp20,
    };
    const uint8_t* mouthDown[20] = {
        mouthDown1,
        mouthDown2,
        mouthDown3,
        mouthDown4,
        mouthDown5,
        mouthDown6,
        mouthDown7,
        mouthDown8,
        mouthDown9,
        mouthDown10,
        mouthDown11,
        mouthDown12,
        mouthDown13,
        mouthDown14,
        mouthDown15,
        mouthDown16,
        mouthDown17,
        mouthDown18,
        mouthDown19,
        mouthDown20,
    };
    void movingMouth() {
        float yAcc = event.acceleration.y;

        // Hysteresis thresholds for movement detection
        const float upThreshold = -2.00, downThreshold = 3.00,
                    upMaxThreshold = -7.00, downMaxThreshold = 8.00;

        // Check if head is moving up or down
        if (yAcc < upThreshold) {
            int level = mapFloat(yAcc, upThreshold, upMaxThreshold, 0, 19);
            mouthFrame = mouthUp[level];
        } else if (yAcc > downThreshold) {
            int level = mapFloat(yAcc, downThreshold, downMaxThreshold, 0, 19);
            mouthFrame = mouthDown[level];
        } else {
            mouthFrame = defaultAnimation[defaultAnimationIndex];
        }
    }

    unsigned long previousMillis = 0;
    const unsigned long interval = 5;
    bool increasingIndex = true;
    void updateAnimation() {
        if (millis() >= mouthInterval) {
            updateIndex();
            mouthInterval = millis() + 80;
        }
    }

    short defaultAnimationIndex = 0;
    void updateIndex() {
        if (increasingIndex) {
            defaultAnimationIndex++;
            if (defaultAnimationIndex >= 19) {
                defaultAnimationIndex = 19;
                increasingIndex = false;
            }
        } else {
            defaultAnimationIndex--;
            if (defaultAnimationIndex <= 0) {
                defaultAnimationIndex = 0;
                increasingIndex = true;
            }
        }
    }

    unsigned long resetBoop;
    bool visemeTaskRunning = false;

    // Task handle for the viseme rendering task
    TaskHandle_t visemeTaskHandle = NULL;

    // Start the viseme rendering task on the second core
    void startVisemeTask() {
        xTaskCreatePinnedToCore(visemeRenderingTask, "VisemeTask", 2048, this, 2, &visemeTaskHandle, 0);
    }

    // The viseme rendering task function
    static void visemeRenderingTask(void* parameter) {
        MouthState* mouthState = reinterpret_cast<MouthState*>(parameter);
        mouthState->visemeTaskRunning = true;
        while (mouthState->currentState != MouthStateEnum::IDLE) {
            mouthState->visemeFrame = mouthState->viseme.renderViseme();
        }
        mouthState->visemeTaskRunning = false;
        vTaskDelete(NULL);
    }
};
