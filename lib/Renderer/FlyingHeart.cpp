#include "FlyingHeart.h"

FlyingHeart::FlyingHeart(LEDMatrixDisplay* displayPtr) : display(displayPtr) {
    resetAll();
}

void FlyingHeart::renderHeart() {
    // Update heart position
    if (millis() - heartSpeed >= 7) {
        for (int i = 0; i < sumHeart; i++) {
            moveHeart(i);
        }
        heartSpeed = millis();
    }
    // Render all heart in Non-blocking render
    for (int i = 0; i < sumHeart; i++) {
        if (Hearts[i].velocityx == 0 && Hearts[i].velocityy == 0) {
            continue;
        }
        drawHeart(Hearts[i].xpos, Hearts[i].ypos, Hearts[i].size);
    }
}

void FlyingHeart::reset() {
    for (int i = setIndex * numHeart; i < (setIndex + 1) * numHeart; i++) {
        if (Hearts[i].velocityx == 0 && Hearts[i].velocityy == 0) {
            Hearts[i].xpos = SCREEN_WIDTH / 2;
            Hearts[i].ypos = 3 + (esp_random() % 5);
            if (esp_random() % 2 == 0) {
                Hearts[i].velocityx = -1.0f * (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
            } else {
                Hearts[i].velocityx = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
            }
            Hearts[i].velocityy = (static_cast<float>(esp_random()) / static_cast<float>(RAND_MAX) * speedModifier);
            Hearts[i].size = getRandomHeartSize();
        }
    }
    setIndex = (setIndex + 1) % numSets;
}

void FlyingHeart::resetAll() {
    for (int i = 0; i < sumHeart; i++) {
        Hearts[i].xpos = 0;
        Hearts[i].ypos = 0;
        Hearts[i].velocityx = 0;
        Hearts[i].velocityy = 0;
        Hearts[i].size = getRandomHeartSize();
    }
}

void FlyingHeart::setSpeed(float speed) {
    speedModifier = std::max(0.1f, std::min(1.0f, 1.0f - speed));
}

FlyingHeart::HeartSize FlyingHeart::getRandomHeartSize() {
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

void FlyingHeart::drawHeart(int offsetX, int offsetY, HeartSize size) {
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

void FlyingHeart::moveHeart(int i) {
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
    if (Hearts[i].xpos >= SCREEN_WIDTH || Hearts[i].xpos <= 0) {
        Hearts[i].xpos = SCREEN_WIDTH;
        Hearts[i].velocityx = 0;
        Hearts[i].velocityy = 0;
    }
    Hearts[i].xpos += Hearts[i].velocityx;
    Hearts[i].ypos += Hearts[i].velocityy;
}
