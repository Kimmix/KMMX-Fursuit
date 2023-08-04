//? Still need two screen to complete this feature
// 5x5
static const uint8_t PROGMEM heartBM[] = {
    0x00, 0xff, 0x00, 0xff, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00};
class FlyingHeart {
   public:
    FlyingHeart(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {
        reset();
    }

    void renderHeart() {
        // Update heart position every X
        if (millis() - heartSpeed >= 5) {
            for (int i = 0; i < numHeart; i++) {
                moveHeart(i);
            }
            heartSpeed = millis();
        }
        // Non-blocking render
        for (int i = 0; i < numHeart; i++) {
            drawHeart(Hearts[i].xpos, Hearts[i].ypos);
        }
    }

    void reset() {
        for (int i = 0; i < numHeart; i++) {
            Hearts[i].xpos = SCREEN_WIDTH / 2;
            Hearts[i].ypos = 3 + (esp_random() % 5);
            Hearts[i].velocityx = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
            Hearts[i].velocityy = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
        }
    }

   private:
    LEDMatrixDisplay* display;
    struct Heart {
        float xpos, ypos;
        float velocityx, velocityy;
    };
    static const int numHeart = 25;
    Heart Hearts[numHeart];
    volatile unsigned long heartSpeed = 0;
    float speedModifier = 0.5;

    int imageWidth = 5, imageHeight = 5;
    void drawHeart(int offsetX, int offsetY) {
        display->drawBitmap(heartBM, imageWidth, imageHeight, offsetX, offsetY, 255, 10, 10);
    }

    void moveHeart(int i) {
        // if (Hearts[i].xpos >= SCREEN_WIDTH) {
        //     Hearts[i].velocityx *= -1;
        // } else if (Hearts[i].xpos <= 0) {
        //     Hearts[i].velocityx = abs(Hearts[i].velocityx);
        // }
        if (Hearts[i].ypos >= SCREEN_HEIGHT) {
            Hearts[i].velocityy *= -1;
            Hearts[i].velocityx *= 1.3;
        } else if (Hearts[i].ypos <= 0) {
            Hearts[i].velocityy = abs(Hearts[i].velocityy);
            Hearts[i].velocityx *= 1.3;
        }
        // Destroy
        if (Hearts[i].xpos >= SCREEN_WIDTH || Hearts[i].xpos <= 0) {
            Hearts[i].xpos = SCREEN_WIDTH;
            Hearts[i].velocityx = 0;
            Hearts[i].velocityy = 0;
        }
        Hearts[i].xpos += Hearts[i].velocityx;
        Hearts[i].ypos += Hearts[i].velocityy;
    }
};