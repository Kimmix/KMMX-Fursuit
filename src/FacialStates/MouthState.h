#include "Bitmaps/mouthBitmap.h"
#include "RenderFunction/viseme.h"

class MouthState {
   public:
    MouthState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        switch (currentState) {
            case MouthStateEnum::IDLE:
                display->drawMouth(mouthDefault);
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
                display->drawMouth(visemeFrame);
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
