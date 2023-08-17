#include "Bitmaps/mouthBitmap.h"
#include "Bitmaps/Viseme/visemeAH.h"
#include "Bitmaps/Viseme/visemeEE.h"
#include "Bitmaps/Viseme/visemeOH.h"
#include "Bitmaps/Viseme/visemeOO.h"
#include "Bitmaps/Viseme/visemeTH.h"
#include "Bitmaps/mouthAnimated.h"
#include "RenderFunction/viseme.h"

class MouthState {
   public:
    MouthState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        updateAnimation();
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

   private:
    LEDMatrixDisplay* display;
    Viseme viseme;
    const uint8_t* visemeFrame = mouthDefault;
    MouthStateEnum prevState, currentState = MouthStateEnum::TALKING;
    unsigned long mouthInterval;

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
    short defaultAnimationIndex = 0;
    void drawDefault() {
        display->drawMouth(defaultAnimation[defaultAnimationIndex]);
    }

    unsigned long previousMillis = 0;
    const unsigned long interval = 5;
    bool increasingIndex = true;
    void updateAnimation() {
        if (millis() >= mouthInterval) {
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
            mouthInterval = millis() + 80;
        }
    }

    unsigned long resetBoop;
    bool visemeTaskRunning = false;

    // Task handle for the viseme rendering task
    TaskHandle_t visemeTaskHandle = NULL;

    // Start the viseme rendering task on the second core
    void startVisemeTask() {
        xTaskCreatePinnedToCore(visemeRenderingTask, "VisemeTask", 2048, this, 1, &visemeTaskHandle, 0);
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
