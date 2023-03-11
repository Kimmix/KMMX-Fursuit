#include "main.h"
#include <Arduino.h>

struct Heart {
	float xpos, ypos;
	float velocityx, velocityy;
};
Heart Hearts[numHeart];
static const int numHeart = 5;

// 5x5
static const uint8_t PROGMEM heartS[] = {
	0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00 };

class FlyingHeart {
private:

	void drawHeart(int offsetX, int offsetY) {
		int imageWidth = 5, imageHeight = 5;
		// int offsetX = 0, offsetY = 0;
		int i, j, j2;
		for (i = 0; i < imageHeight; i++) {
			for (j = 0, j2 = 63; j < imageWidth; j++) {
				if (heartS[i * imageWidth + j] > 0) {
					display.writeBUffer(offsetX + j, offsetY + i, (heartS[i * imageWidth + j] * 255) / 255, 0, 0);
					// matrix->drawPixelRGB888(-offsetX + PANEL_WIDTH + j2, offsetY + i,
					//                         (heartS[i * imageWidth + j] * 255) / 255, 0,
					//                         0);
					j2--;
				}
			}
		}
	}

public:
	volatile unsigned long heartSpeed = 0;

	void init() {
		randomSeed(analogRead(0));
		for (int i = 0; i < numHeart; i++) {
			// Hearts[i].xpos = random(0, matrix->width() - 8);
			// Hearts[i].ypos = random(0, matrix->height() - 6);
			Hearts[i].xpos = 0;
			Hearts[i].ypos = random(0, PANEL_HEIGHT - 5);
			Hearts[i].velocityx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			Hearts[i].velocityy = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		}
	}

	void flyingHeart() {
		if (millis() - heartSpeed >= 25) {
			FastLED_Pixel_Buff->dimAll(200);
			for (int i = 0; i < numHeart; i++) {
				// Draw heart and then calculate
				drawHeart(Hearts[i].xpos, Hearts[i].ypos);
				if (8 + Hearts[i].xpos >= PANEL_WIDTH) {
					Hearts[i].velocityx *= -1;
				}
				else if (Hearts[i].xpos <= 0) {
					Hearts[i].velocityx = abs(Hearts[i].velocityx);
				}
				if (6 + Hearts[i].ypos >= PANEL_HEIGHT) {
					Hearts[i].velocityy *= -1;
				}
				else if (Hearts[i].ypos <= 0) {
					Hearts[i].velocityy = abs(Hearts[i].velocityy);
				}
				Hearts[i].xpos += Hearts[i].velocityx;
				Hearts[i].ypos += Hearts[i].velocityy;
				heartSpeed = millis();
			}
			FastLED_Pixel_Buff->show();
		}
	}
};