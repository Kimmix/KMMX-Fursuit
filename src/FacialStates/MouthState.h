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
        // Check if currentState is not TALKING, and if the viseme task is running, suspend it.
        if (currentState != MouthStateEnum::TALKING && visemeTaskRunning) {
            visemeTaskRunning = false;
            // viseme.microphoneEnable = false;
            vTaskSuspend(visemeTaskHandle);
            Serial.println("Killed task");
            return;
        }
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
                // Check if the viseme task is already running, if not start it
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

   private:
    LEDMatrixDisplay* display;
    Viseme viseme;
    const uint8_t* visemeFrame = mouthDefault;
    MouthStateEnum currentState = MouthStateEnum::IDLE;

    unsigned long resetBoop;
    bool visemeTaskRunning = false;

    // Task handle for the viseme rendering task
    TaskHandle_t visemeTaskHandle = NULL;

    // Start the viseme rendering task on the second core
    void startVisemeTask() {
        Serial.println("Start task");
        xTaskCreatePinnedToCore(
            visemeRenderingTask,
            "VisemeTask",
            2048,  // Stack size (adjust this according to your needs)
            this,  // Pass the MouthState object as a parameter to the task
            1,     // Priority of the task
            &visemeTaskHandle,
            0  // Core to run the task on
        );
    }

    // The viseme rendering task function
    static void visemeRenderingTask(void* parameter) {
        MouthState* mouthState = reinterpret_cast<MouthState*>(parameter);
        mouthState->visemeTaskRunning = true;
        while (mouthState->currentState == MouthStateEnum::TALKING) {
            mouthState->visemeFrame = mouthState->viseme.renderViseme();
        }
        mouthState->visemeTaskRunning = false;
        vTaskDelete(NULL);  // Terminate the task explicitly
    }
};
