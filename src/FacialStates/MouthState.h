#include "Bitmaps/mouthBitmapNew.h"
#include "RenderFunction/viseme.h"

enum class MouthStateEnum { IDLE,
                            BOOP,
                            TALKING };
// Define a task handle for the viseme rendering task
TaskHandle_t visemeTaskHandle = NULL;
class MouthState : public FacialState {
   public:
    MouthState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}

    void update() {
        switch (currentState) {
            case MouthStateEnum::IDLE:
                display->drawMouth(mouthDefault);
                break;
            case MouthStateEnum::BOOP:
                display->drawMouth(mouthOpen);
                if (millis() - resetBoop >= 100) {
                    currentState = MouthStateEnum::TALKING;
                }
                break;
            case MouthStateEnum::TALKING:
                if (!visemeTaskRunning) {
                    startVisemeTask();
                }
                display->drawMouth(visemeFrame);
                break;
            default:
                break;
        }
    }

    void setState(MouthStateEnum newState) {
        currentState = newState;
    }

    MouthStateEnum getState() const {
        return currentState;
    }

   private:
    MouthStateEnum currentState = MouthStateEnum::IDLE;
    LEDMatrixDisplay* display;
    Viseme viseme;
    TaskHandle_t visemeTaskHandle = NULL;

    bool visemeTaskRunning = false;
    const uint8_t* visemeFrame = mouthDefault;
    unsigned long resetBoop;

    // Start the viseme rendering task on the second core
    void startVisemeTask() {
        Serial.println("Create task");
        xTaskCreatePinnedToCore(visemeRenderingTask, "VisemeTask", 2048, this, 1, &visemeTaskHandle, 0);
    }

    void stopVisemeTask() {
        vTaskDelete(visemeTaskHandle);
        visemeTaskRunning = false;
    }

    // The viseme rendering task function
    static void visemeRenderingTask(void* parameter) {
        MouthState* mouthState = reinterpret_cast<MouthState*>(parameter);
        mouthState->visemeTaskRunning = true;
        Serial.println("TASK Runinng");
        while (mouthState->currentState == MouthStateEnum::TALKING) {
            mouthState->visemeFrame = mouthState->viseme.renderViseme();
        }
        mouthState->visemeTaskRunning = false;
        Serial.println("TASK Ended");
        vTaskDelete(NULL);
    }
};
