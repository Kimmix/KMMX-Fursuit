#pragma once

#include <Arduino.h>
#include <atomic>

#include "Devices/LEDMatrixDisplay/Hub75DMA.h"
#include "Renderer/Boop.h"

class SlotMachine {
   public:
    enum class Outcome : uint8_t { NONE, WIN, LOSE };
    enum class State : uint8_t { READY, SPINNING, REVEAL, RESULT };

    explicit SlotMachine(Hub75DMA* display);

    void setEnabled(bool value);
    bool isEnabled() const;
    Outcome getOutcome() const;
    State getState() const;
    uint8_t getCharge() const;
    uint8_t getStoppedReels() const;
    uint8_t getMatchingReels() const;
    bool isAnticipating() const;
    uint8_t getResultSymbol() const;
    void update(uint16_t proximity, unsigned long now = millis());
    void render(unsigned long now = millis());

   private:
    static constexpr uint8_t reelCount = 3;
    static constexpr uint8_t symbolCount = 4;
    static constexpr unsigned long revealDurationMs = 450;
    static constexpr unsigned long resultDurationMs = 3000;
    static constexpr unsigned long megaResultDurationMs = 5000;
    static constexpr unsigned long chargeSmoothingIntervalMs = 20;

    Hub75DMA* display;
    Boop boop;
    std::atomic<bool> enabled{false};
    std::atomic<bool> spinRequested{false};
    std::atomic<Outcome> outcome{Outcome::NONE};
    std::atomic<uint8_t> charge{0};
    std::atomic<uint8_t> displayedCharge{0};
    std::atomic<uint8_t> stoppedReels{0};
    std::atomic<uint8_t> matchingReels{0};
    std::atomic<bool> anticipating{false};
    std::atomic<uint8_t> resultSymbol{0};
    bool inputWasEnabled = false;
    bool renderWasEnabled = false;
    bool winning = false;
    std::atomic<State> phase{State::READY};
    unsigned long phaseStartedAt = 0;
    unsigned long lastChargeUpdateAt = 0;
    unsigned long lastReelStepAt[reelCount] = {0, 0, 0};
    uint8_t reels[reelCount] = {0, 1, 2};
    uint8_t previousReels[reelCount] = {0, 1, 2};
    uint8_t result[reelCount] = {0, 1, 2};

    void startSpin(unsigned long now);
    void updateSpin(unsigned long now);
    void resetGame();
    void drawPanel(unsigned long now);
    void drawSymbol(uint8_t symbol, int x, int y, uint8_t brightness = 255,
                    int highlightColumn = -1, bool squashed = false);
    void drawChargeMeter(uint8_t value, uint16_t color);
    void drawMarquee(unsigned long now, uint16_t first, uint16_t second);
    void drawRainbowMarquee(unsigned long now);
    void drawWinParticles(unsigned long now, uint16_t first, uint16_t second);
    void drawWord(const uint8_t* const* glyphs, uint8_t count, int x, int y, uint16_t color);
    void drawGlyph(const uint8_t* glyph, int x, int y, uint16_t color);
    void drawRect(int x, int y, int width, int height, uint16_t color);
    void drawPixelBoth(int x, int y, uint16_t color);
    static bool isWinning(const uint8_t values[reelCount]);
};
