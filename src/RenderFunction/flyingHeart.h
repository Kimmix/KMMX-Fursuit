//? Still need two screen to complete this feature
// 3x3
static const uint8_t PROGMEM heartBM3x3[] = {
    0xff, 0x00, 0xff,
    0xff, 0xff, 0xff,
    0x00, 0xff, 0x00};
// 5x5
static const uint8_t PROGMEM heartBM5x5[] = {
    0x00, 0xff, 0x00, 0xff, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00};
// 7x7
static const uint8_t PROGMEM heartBM7x7[] = {
    0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00};
class FlyingHeart {
   public:
    FlyingHeart(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {
        resetAll();
    }

    void renderHeart() {
        // Update heart position every X
        if (millis() - heartSpeed >= 5) {
            for (int i = 0; i < sumHeart; i++) {
                moveHeart(i);
            }
            heartSpeed = millis();
        }
        // Non-blocking render
        for (int i = 0; i < sumHeart; i++) {
            if (Hearts[i].velocityx == 0 && Hearts[i].velocityy == 0) {
                continue;
            }
            drawHeart(Hearts[i].xpos, Hearts[i].ypos, Hearts[i].size);
        }
    }

    void reset() {
        for (int i = setIndex * numHeart; i < (setIndex + 1) * numHeart; i++) {
            if (Hearts[i].velocityx == 0 && Hearts[i].velocityy == 0) {
                Hearts[i].xpos = SCREEN_WIDTH / 2;
                Hearts[i].ypos = 3 + (esp_random() % 5);
                // if (esp_random() % 2 == 0) {
                //     Hearts[i].velocityx = -1.0f * (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
                // } else {
                //     Hearts[i].velocityx = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
                // }
                Hearts[i].velocityx = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
                Hearts[i].velocityy = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
                Hearts[i].size = getRandomHeartSize();
            }
        }
        setIndex = (setIndex + 1) % numSets;
    }

    void resetAll() {
        for (int i = 0; i < sumHeart; i++) {
            Hearts[i].xpos = 0;
            Hearts[i].ypos = 0;
            Hearts[i].velocityx = 0;
            Hearts[i].velocityy = 0;
            Hearts[i].size = getRandomHeartSize();
        }
    }

    void setSpeed(float speed) {
        speedModifier = std::max(0.1f, std::min(1.0f, 1.0f - speed));
    }

   private:
    LEDMatrixDisplay* display;
    enum HeartSize { SIZE_3x3,
                     SIZE_5x5,
                     SIZE_7x7 };
    struct Heart {
        float xpos, ypos;
        float velocityx, velocityy;
        HeartSize size;
    };
    short setIndex = 0;
    static const short numSets = 3, numHeart = 23, sumHeart = numSets * numHeart;
    Heart Hearts[sumHeart];
    volatile unsigned long heartSpeed = 0;
    float speedModifier = 0.5;

    HeartSize getRandomHeartSize() {
        int randomNum = esp_random() % 3;
        switch (randomNum) {
            case 0:
                return HeartSize::SIZE_3x3;
            case 1:
                return HeartSize::SIZE_5x5;
            case 2:
            default:
                return HeartSize::SIZE_7x7;
        }
    }

    int imageWidth = 5, imageHeight = 5;
    void drawHeart(int offsetX, int offsetY, HeartSize size) {
        switch (size) {
            case HeartSize::SIZE_3x3:
                display->drawBitmap(heartBM3x3, 3, 3, offsetX, offsetY, 255, 10, 10);
                break;
            case HeartSize::SIZE_5x5:
                display->drawBitmap(heartBM5x5, 5, 5, offsetX, offsetY, 255, 10, 10);
                break;
            case HeartSize::SIZE_7x7:
                display->drawBitmap(heartBM7x7, 7, 7, offsetX, offsetY, 255, 10, 10);
                break;
            default:
                display->drawBitmap(heartBM5x5, 5, 5, offsetX, offsetY, 255, 10, 10);
                break;
        }
    }

    void moveHeart(int i) {
        // if (Hearts[i].xpos >= SCREEN_WIDTH) {
        //     Hearts[i].velocityx *= -1;
        // } else if (Hearts[i].xpos <= 0) {
        //     Hearts[i].velocityx = abs(Hearts[i].velocityx);
        // }
        if (Hearts[i].velocityx == 0 && Hearts[i].velocityy == 0) {
            return;
        }
        if (Hearts[i].ypos >= SCREEN_HEIGHT) {
            Hearts[i].velocityy *= -1;
            Hearts[i].velocityx *= 1.5;
        } else if (Hearts[i].ypos <= 0) {
            Hearts[i].velocityy = abs(Hearts[i].velocityy);
            Hearts[i].velocityx *= 1.5;
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