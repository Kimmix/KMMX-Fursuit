// * SRC: https://github.com/witnessmenow/ESP32-Trinity/blob/master/examples/Projects/TetrisWithNunchuck/TetrisWithNunchuck.ino
#include <Arduino.h>

//--------------------------------
// Game Config Options:
//--------------------------------

#define LEFT_OFFSET -2
#define TOP_OFFSET 0

#define WORLD_TO_PIXEL 2  // each dot on the game world will be represented by these many pixels.

#define DELAY_BETWEEN_FRAMES 50
#define DELAY_ON_LINE_CLEAR 100

#define NUM_PIECES_TIL_SPEED_CHANGE 20

class Teris {
   public:
    Teris(DisplayController* displayPtr = nullptr,
          LIS3DH* lisPtr = nullptr) : display(displayPtr),
                                      lis(lisPtr) {}

    void init() {
        strcpy(tetromino[0], "..X...X...X...X.");  // Tetronimos 4x4
        strcpy(tetromino[1], "..X..XX...X.....");
        strcpy(tetromino[2], ".....XX..XX.....");
        strcpy(tetromino[3], "..X..XX..X......");
        strcpy(tetromino[4], ".X...XX...X.....");
        strcpy(tetromino[5], ".X...X...XX.....");
        strcpy(tetromino[6], "..X...X..XX.....");
        // inits field
        restartGame();
    }

    void play(bool& playTeris) {
        if (!bGameOver) {
            gameInput();
            if (!isPaused) {
                gameTiming();
                gameLogic();
            } else {
                delay(DELAY_BETWEEN_FRAMES);  // stopping pulsing of LEDS
            }
            displayLogic();
        } else {
            delay(DELAY_BETWEEN_FRAMES);
            gameInput();
            displayLogic(true);
            if (!digitalRead(GPIO_NUM_35)) {
                restartGame();
            }
            lis->getTap();
            if (lis->isTap()) {
                display->clearScreen();
                playTeris = false;
            }
        }
    }

   private:
    DisplayController* display;
    LIS3DH* lis;

    uint16_t myRED = display->color565(255, 73, 113);
    uint16_t myGREEN = display->color565(24, 227, 200);
    uint16_t myBLUE = display->color565(85, 111, 255);
    uint16_t myWHITE = display->color565(190, 190, 193);
    uint16_t myYELLOW = display->color565(255, 128, 55);
    uint16_t myCYAN = display->color565(63, 220, 238);
    uint16_t myMAGENTA = display->color565(176, 67, 209);
    uint16_t myBLACK = display->color565(0, 0, 0);

    // [0] is empty space
    // [1-7] are tetromino colours
    // [8] is the colour a completed line changes to before disappearing
    // [9] is the walls of the board.
    uint16_t gameColours[10] = {myBLACK, myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myYELLOW, myMAGENTA};

    bool bGameOver = false;

    bool moveLeft;
    bool moveRight;
    bool dropDown;
    bool rotatePiece;
    bool cButtonPressed;

    bool lastCButtonState;

    bool isPaused;

    int moveThreshold = 30;

    int nNextPiece = 2;
    int nCurrentPiece = 2;
    int nCurrentRotation = 0;
    int nCurrentX = (nFieldWidth / 2) - 2;
    int nCurrentY = 0;

    int nSpeed = 20;
    int nSpeedCount = 0;
    bool bForceDown = false;
    bool bRotateHold = true;

    int nPieceCount = 0;
    int nScore = 0;

    int completedLinesIndex[4];
    int numCompletedLines;

    char tetromino[7][17];
    int nFieldWidth = 22;
    int nFieldHeight = 17;
    char* pField = nullptr;

    void gameLogic() {
        // Handle updating lines cleared last tick
        clearLines();

        nCurrentX += (moveRight && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (moveLeft && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (dropDown && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        if (rotatePiece) {
            nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = false;
        } else
            bRotateHold = true;

        // Force the piece down the playfield if it's time
        if (bForceDown) {
            // Update difficulty every 50 pieces
            nSpeedCount = 0;
            nPieceCount++;
            if (nPieceCount % NUM_PIECES_TIL_SPEED_CHANGE == 0)
                if (nSpeed >= 10) nSpeed--;

            // Test if piece can be moved down
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;  // It can, so do it!
            else {
                // It can't! Lock the piece in place
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != '.')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                // Check for lines
                for (int py = 0; py < 4; py++)
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine) {
                            // Remove Line, set to =
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            // vLines.push_back(nCurrentY + py);
                            completedLinesIndex[numCompletedLines] = nCurrentY + py;
                            numCompletedLines++;
                        }
                    }

                nScore += 25;
                if (numCompletedLines > 0) nScore += (1 << numCompletedLines) * 100;

                // Pick New Piece
                getNewPiece();

                // If piece does not fit straight away, game over!
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
        }
    }

    int Rotate(int px, int py, int r) {
        int pi = 0;
        switch (r % 4) {
            case 0:                // 0 degrees        // 0  1  2  3
                pi = py * 4 + px;  // 4  5  6  7
                break;             // 8  9 10 11
                // 12 13 14 15

            case 1:                       // 90 degrees       //12  8  4  0
                pi = 12 + py - (px * 4);  // 13  9  5  1
                break;                    // 14 10  6  2
                // 15 11  7  3

            case 2:                       // 180 degrees      //15 14 13 12
                pi = 15 - (py * 4) - px;  // 11 10  9  8
                break;                    // 7  6  5  4
                // 3  2  1  0

            case 3:                      // 270 degrees      // 3  7 11 15
                pi = 3 - py + (px * 4);  // 2  6 10 14
                break;                   // 1  5  9 13
        }                                // 0  4  8 12

        return pi;
    }

    bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
        // All Field cells >0 are occupied
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++) {
                // Get index into piece
                int pi = Rotate(px, py, nRotation);

                // Get index into field
                int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

                // Check that test is in bounds. Note out of bounds does
                // not necessarily mean a fail, as the long vertical piece
                // can have cells that lie outside the boundary, so we'll
                // just ignore them
                if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                    if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                        // In Bounds so do collision check
                        if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
                            return false;  // fail on first hit
                    }
                }
            }

        return true;
    }

    void getNewPiece() {
        // Pick New Piece
        nCurrentX = (nFieldWidth / 2) - 2;
        nCurrentY = 0;
        nCurrentRotation = 0;
        nCurrentPiece = nNextPiece;
        nNextPiece = esp_random() % 7;
        // Serial.print("Current:");
        // Serial.print(nCurrentPiece);
        // Serial.print(", Next:");
        // Serial.println(nNextPiece);
    }

    void gameTiming() {
        delay(DELAY_BETWEEN_FRAMES);
        nSpeedCount++;
        bForceDown = (nSpeedCount == nSpeed);
    }

    void clearLines() {
        if (numCompletedLines > 0) {
            delay(DELAY_ON_LINE_CLEAR);
            for (int i = 0; i < numCompletedLines; i++) {
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = completedLinesIndex[i]; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }
            }
            numCompletedLines = 0;
        }
    }

    uint16_t getFieldColour(int index, bool isDeathScreen) {
        if (isDeathScreen && pField[index] != 0) {
            return myRED;
        } else {
            return gameColours[pField[index]];
        }
    }

    void displayLogic(bool isDeathScreen = false) {
        display->render();
        int realX;
        int realY;

        display->clearScreen();

        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++) {
                realX = (x * WORLD_TO_PIXEL) + LEFT_OFFSET;
                realY = (y * WORLD_TO_PIXEL) + TOP_OFFSET;
                uint16_t fieldColour = getFieldColour((y * nFieldWidth + x), isDeathScreen);
                if (fieldColour != myBLACK) {
                    if (WORLD_TO_PIXEL > 1) {
                        display->drawRect(realX, realY, WORLD_TO_PIXEL, WORLD_TO_PIXEL, fieldColour);
                    } else {
                        display->drawPixel(realX, realY, fieldColour);
                    }
                }
            }

        // Draw Current Piece
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != '.') {
                    realX = ((nCurrentX + px) * WORLD_TO_PIXEL) + LEFT_OFFSET;
                    realY = ((nCurrentY + py) * WORLD_TO_PIXEL) + TOP_OFFSET;
                    if (WORLD_TO_PIXEL > 1) {
                        display->drawRect(realX, realY, WORLD_TO_PIXEL, WORLD_TO_PIXEL, gameColours[nCurrentPiece + 1]);
                    } else {
                        display->drawPixel(realX, realY, gameColours[nCurrentPiece + 1]);
                    }
                }
            }
        }

        display->setTextColor(myBLUE);

        if (isPaused) {
            display->setCursor(20, 1);
            display->print("Paws");
        } else {
            // Display the Score

            // As it changes, we need to calculate where
            // the center is
            int16_t x1, y1;
            uint16_t w, h;
            display->getTextBounds(String(nScore), 0, 0, &x1, &y1, &w, &h);
            display->setCursor(nFieldWidth * 2, 5);
            display->print(nScore);
        }

        // display->render();
    }

    void gameInput() {
        // boolean success = nchuk.update();  // Get new data from the controller
        // if (!success) {  // Ruh roh
        //     Serial.println("Controller disconnected!");
        //     delay(1000);
        // }
        // else {
        //     rotatePiece = nchuk.buttonZ();
        //     lastCButtonState = cButtonPressed;
        //     cButtonPressed = nchuk.buttonC();
        //     if (cButtonPressed && !lastCButtonState)
        //     {
        //         isPaused = !isPaused;
        //     }
        //     // Read a joystick axis (0-255, X and Y)
        //     int joyY = nchuk.joyY();
        //     int joyX = nchuk.joyX();
        //     moveRight = (joyX > 127 + moveThreshold);
        //     moveLeft = (joyX < 127 - moveThreshold);
        //     dropDown = (joyY < 127 - moveThreshold);
        // }
    }

    void restartGame() {
        bGameOver = false;
        pField = new char[nFieldWidth * nFieldHeight];  // Create play field buffer
        for (int x = 0; x < nFieldWidth; x++)           // Board Boundary
            for (int y = 0; y < nFieldHeight; y++)
                pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

        // Pick New Piece
        getNewPiece();

        nScore = 0;
    }
};
