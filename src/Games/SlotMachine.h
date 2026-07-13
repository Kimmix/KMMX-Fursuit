#pragma once

#include <Arduino.h>
#include <atomic>

#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Renderer/Boop.h"

class SlotMachine {
   public:
    enum class Outcome : uint8_t { NONE, WIN, LOSE };

    explicit SlotMachine(Hub75DMA* display);

    void setEnabled(bool value);
    bool isEnabled() const;
    Outcome getOutcome() const;
    void update(uint16_t proximity, unsigned long now = millis());
    void render(unsigned long now = millis());

   private:
    enum class Phase : uint8_t { READY, SPINNING, RESULT };

    static constexpr uint8_t reelCount = 3;
    static constexpr uint8_t symbolCount = 4;
    static constexpr unsigned long reelStepMs = 80;
    static constexpr unsigned long resultDurationMs = 3000;

    Hub75DMA* display;
    Boop boop;
    std::atomic<bool> enabled{false};
    std::atomic<bool> spinRequested{false};
    std::atomic<Outcome> outcome{Outcome::NONE};
    bool inputWasEnabled = false;
    bool renderWasEnabled = false;
    bool winning = false;
    Phase phase = Phase::READY;
    unsigned long phaseStartedAt = 0;
    unsigned long lastReelStepAt = 0;
    uint8_t reels[reelCount] = {0, 1, 2};
    uint8_t previousReels[reelCount] = {0, 1, 2};
    uint8_t result[reelCount] = {0, 1, 2};

    void startSpin(unsigned long now);
    void updateSpin(unsigned long now);
    void resetGame();
    void drawPanel(unsigned long now);
    void drawSymbol(uint8_t symbol, int x, int y, uint8_t brightness = 255);
    void drawMarquee(unsigned long now, uint16_t first, uint16_t second);
    void drawWord(const uint8_t* const* glyphs, uint8_t count, int x, int y, uint16_t color);
    void drawGlyph(const uint8_t* glyph, int x, int y, uint16_t color);
    void drawRect(int x, int y, int width, int height, uint16_t color);
    void drawPixelBoth(int x, int y, uint16_t color);
    static bool isWinning(const uint8_t values[reelCount]);
};
