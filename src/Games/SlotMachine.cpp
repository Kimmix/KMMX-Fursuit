#include "SlotMachine.h"

#include <cassert>
#include <esp_system.h>

namespace {
const uint8_t symbols[][5] PROGMEM = {
    {0b00100, 0b00110, 0b00100, 0b01010, 0b01110},  // Cherry
    {0b01010, 0b11111, 0b11111, 0b01110, 0b00100},  // Heart
    {0b00100, 0b10101, 0b01110, 0b10101, 0b00100},  // Star
    {0b00100, 0b01110, 0b11111, 0b01110, 0b00100},  // Diamond
};

const uint8_t glyphB[] PROGMEM = {0b110, 0b101, 0b110, 0b101, 0b110};
const uint8_t glyphO[] PROGMEM = {0b111, 0b101, 0b101, 0b101, 0b111};
const uint8_t glyphP[] PROGMEM = {0b110, 0b101, 0b110, 0b100, 0b100};
const uint8_t glyphW[] PROGMEM = {0b101, 0b101, 0b101, 0b111, 0b101};
const uint8_t glyphI[] PROGMEM = {0b111, 0b010, 0b010, 0b010, 0b111};
const uint8_t glyphN[] PROGMEM = {0b101, 0b111, 0b111, 0b111, 0b101};

const uint8_t* const boopWord[] = {glyphB, glyphO, glyphO, glyphP};
const uint8_t* const winWord[] = {glyphW, glyphI, glyphN};
const uint32_t symbolColors[] = {0xFF3030, 0xFF4C8B, 0xFFD83D, 0x52C7FF};
const unsigned long reelStopMs[] = {700, 1000, 1300};
constexpr int slotScreenOffsetX = 14;
}  // namespace

SlotMachine::SlotMachine(Hub75DMA* display) : display(display) {
#ifndef NDEBUG
    const uint8_t win[] = {1, 1, 1};
    const uint8_t loss[] = {1, 2, 1};
    assert(isWinning(win));
    assert(!isWinning(loss));
#endif
}

void SlotMachine::setEnabled(bool value) {
    enabled.store(value);
    if (!value) {
        spinRequested.store(false);
        outcome.store(Outcome::NONE);
    }
}

bool SlotMachine::isEnabled() const {
    return enabled.load();
}

SlotMachine::Outcome SlotMachine::getOutcome() const {
    return outcome.load();
}

void SlotMachine::update(uint16_t proximity, unsigned long now) {
    const bool active = isEnabled();
    if (active != inputWasEnabled) {
        boop.reset();
        inputWasEnabled = active;
    }
    if (active && boop.update(proximity, now).event == BoopEvent::COMPLETED) {
        spinRequested.store(true);
    }
}

void SlotMachine::render(unsigned long now) {
    const bool active = isEnabled();
    if (!active) {
        renderWasEnabled = false;
        return;
    }
    if (!renderWasEnabled) {
        resetGame();
        renderWasEnabled = true;
    }
    if (spinRequested.exchange(false) && phase != Phase::SPINNING) startSpin(now);
    if (phase == Phase::SPINNING) updateSpin(now);
    if (phase == Phase::RESULT && now - phaseStartedAt >= resultDurationMs) {
        phase = Phase::READY;
        outcome.store(Outcome::NONE);
    }
    drawPanel(now);
}

void SlotMachine::startSpin(unsigned long now) {
    for (uint8_t& value : result) value = esp_random() % symbolCount;
    phase = Phase::SPINNING;
    phaseStartedAt = now;
    lastReelStepAt = now;
    winning = false;
    outcome.store(Outcome::NONE);
}

void SlotMachine::updateSpin(unsigned long now) {
    const unsigned long elapsed = now - phaseStartedAt;
    if (now - lastReelStepAt >= reelStepMs) {
        lastReelStepAt = now;
        for (uint8_t i = 0; i < reelCount; ++i) {
            reels[i] = elapsed >= reelStopMs[i] ? result[i] : esp_random() % symbolCount;
        }
    }
    if (elapsed >= reelStopMs[reelCount - 1]) {
        for (uint8_t i = 0; i < reelCount; ++i) reels[i] = result[i];
        winning = isWinning(reels);
        outcome.store(winning ? Outcome::WIN : Outcome::LOSE);
        phase = Phase::RESULT;
        phaseStartedAt = now;
    }
}

void SlotMachine::resetGame() {
    phase = Phase::READY;
    winning = false;
    outcome.store(Outcome::NONE);
    reels[0] = 0;
    reels[1] = 1;
    reels[2] = 2;
    spinRequested.store(false);
}

void SlotMachine::drawPanel(unsigned long now) {
    const uint16_t white = display->color565(210, 210, 210);
    const uint16_t yellow = display->color565(255, 210, 35);
    const uint16_t red = display->color565(180, 30, 30);

    if (phase == Phase::READY) drawWord(boopWord, 4, 24, 1, white);
    if (phase == Phase::RESULT && winning) drawWord(winWord, 3, 26, 1, yellow);

    const uint16_t frameColor = phase == Phase::RESULT && winning
                                    ? ((now / 150) % 2 ? yellow : white)
                                    : (phase == Phase::RESULT ? red : white);
    for (uint8_t i = 0; i < reelCount; ++i) {
        const int x = i * 17;
        drawRect(x, 8, 16, 18, frameColor);
        drawSymbol(reels[i], x + 3, 12);
    }
}

void SlotMachine::drawSymbol(uint8_t symbol, int x, int y) {
    const uint32_t rgb = symbolColors[symbol % symbolCount];
    const uint16_t color = display->color565(rgb >> 16, rgb >> 8, rgb);
    for (uint8_t row = 0; row < 5; ++row) {
        const uint8_t bits = pgm_read_byte(&symbols[symbol % symbolCount][row]);
        for (uint8_t col = 0; col < 5; ++col) {
            if (!(bits & (1 << (4 - col)))) continue;
            drawPixelBoth(x + col * 2, y + row * 2, color);
            drawPixelBoth(x + col * 2 + 1, y + row * 2, color);
            drawPixelBoth(x + col * 2, y + row * 2 + 1, color);
            drawPixelBoth(x + col * 2 + 1, y + row * 2 + 1, color);
        }
    }
}

void SlotMachine::drawWord(const uint8_t* const* glyphs, uint8_t count, int x, int y, uint16_t color) {
    for (uint8_t i = 0; i < count; ++i) drawGlyph(glyphs[i], x + i * 4, y, color);
}

void SlotMachine::drawGlyph(const uint8_t* glyph, int x, int y, uint16_t color) {
    for (uint8_t row = 0; row < 5; ++row) {
        const uint8_t bits = pgm_read_byte(glyph + row);
        for (uint8_t col = 0; col < 3; ++col) {
            if (bits & (1 << (2 - col))) drawPixelBoth(x + col, y + row, color);
        }
    }
}

void SlotMachine::drawRect(int x, int y, int width, int height, uint16_t color) {
    for (int px = x; px < x + width; ++px) {
        drawPixelBoth(px, y, color);
        drawPixelBoth(px, y + height - 1, color);
    }
    for (int py = y + 1; py < y + height - 1; ++py) {
        drawPixelBoth(x, py, color);
        drawPixelBoth(x + width - 1, py, color);
    }
}

void SlotMachine::drawPixelBoth(int x, int y, uint16_t color) {
    const int shiftedX = x + slotScreenOffsetX;
    display->drawPixel(shiftedX, y, color);
    display->drawPixel(shiftedX + panelResX, y, color);
}

bool SlotMachine::isWinning(const uint8_t values[reelCount]) {
    return values[0] == values[1] && values[1] == values[2];
}
