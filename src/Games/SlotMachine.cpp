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
const uint8_t glyphA[] PROGMEM = {0b010, 0b101, 0b111, 0b101, 0b101};
const uint8_t glyphC[] PROGMEM = {0b011, 0b100, 0b100, 0b100, 0b011};
const uint8_t glyphE[] PROGMEM = {0b111, 0b100, 0b110, 0b100, 0b111};
const uint8_t glyphJ[] PROGMEM = {0b001, 0b001, 0b001, 0b101, 0b010};
const uint8_t glyphK[] PROGMEM = {0b101, 0b110, 0b100, 0b110, 0b101};
const uint8_t glyphO[] PROGMEM = {0b111, 0b101, 0b101, 0b101, 0b111};
const uint8_t glyphP[] PROGMEM = {0b110, 0b101, 0b110, 0b100, 0b100};
const uint8_t glyphT[] PROGMEM = {0b111, 0b010, 0b010, 0b010, 0b010};
const uint8_t glyphN[] PROGMEM = {0b101, 0b111, 0b111, 0b111, 0b101};
const uint8_t glyphBang[] PROGMEM = {0b010, 0b010, 0b010, 0b000, 0b010};

const uint8_t* const boopWord[] = {glyphB, glyphO, glyphO, glyphP, glyphBang};
const uint8_t* const jackpotWord[] = {glyphJ, glyphA, glyphC, glyphK, glyphP, glyphO, glyphT, glyphBang};
const uint8_t* const nopeWord[] = {glyphN, glyphO, glyphP, glyphE, glyphBang};
const uint32_t symbolColors[] = {0xFF3030, 0xFF4C8B, 0xFFD83D, 0x52C7FF};
const unsigned long reelStopMs[] = {700, 1000, 1300};
constexpr unsigned long reelBounceMs = 160;
constexpr int slotScreenOffsetX = 14;

constexpr int reelBounceOffset(unsigned long elapsed) {
    return elapsed >= reelBounceMs ? 0 : elapsed < reelBounceMs / 2
                                               ? elapsed * 2 / (reelBounceMs / 2)
                                               : (reelBounceMs - elapsed) * 2 / (reelBounceMs / 2);
}
}  // namespace

SlotMachine::SlotMachine(Hub75DMA* display) : display(display) {
#ifndef NDEBUG
    const uint8_t win[] = {1, 1, 1};
    const uint8_t loss[] = {1, 2, 1};
    assert(isWinning(win));
    assert(!isWinning(loss));
    assert(reelBounceOffset(0) == 0 && reelBounceOffset(reelBounceMs / 2) == 2 && reelBounceOffset(reelBounceMs) == 0);
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
    for (uint8_t i = 0; i < reelCount; ++i) {
        previousReels[i] = reels[i];
        reels[i] = esp_random() % symbolCount;
    }
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
            previousReels[i] = reels[i];
            reels[i] = elapsed >= reelStopMs[i] ? result[i] : esp_random() % symbolCount;
        }
    }
    if (elapsed >= reelStopMs[reelCount - 1] + reelBounceMs) {
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
    for (uint8_t i = 0; i < reelCount; ++i) previousReels[i] = reels[i];
    spinRequested.store(false);
}

void SlotMachine::drawPanel(unsigned long now) {
    const uint16_t white = display->color565(210, 210, 210);
    const uint16_t yellow = display->color565(255, 210, 35);
    const uint16_t red = display->color565(180, 30, 30);

    if (phase == Phase::READY) drawWord(boopWord, 5, 16, 1, white);
    if (phase == Phase::RESULT) {
        if (winning) drawWord(jackpotWord, 8, 10, 1, yellow);
        else drawWord(nopeWord, 5, 16, 1, red);
    }

    const uint16_t frameColor = phase == Phase::RESULT && winning
                                    ? ((now / 150) % 2 ? yellow : white)
                                    : (phase == Phase::RESULT ? red : white);
    for (uint8_t i = 0; i < reelCount; ++i) {
        const int x = i * 17;
        drawRect(x, 8, 16, 18, frameColor);
        if (phase == Phase::SPINNING) {
            const unsigned long elapsed = now - phaseStartedAt;
            if (elapsed < reelStopMs[i]) {
                const unsigned long stepElapsed = now - lastReelStepAt;
                const int offset = (stepElapsed < reelStepMs ? stepElapsed : reelStepMs - 1) * 10 / reelStepMs;
                drawSymbol(previousReels[i], x + 3, 12 + offset, 64);
                drawSymbol(reels[i], x + 3, 2 + offset);
            } else {
                drawSymbol(result[i], x + 3, 12 + reelBounceOffset(elapsed - reelStopMs[i]));
            }
        } else {
            drawSymbol(reels[i], x + 3, 12);
        }
    }
    if (phase == Phase::RESULT && winning) drawMarquee(now, yellow, white);
}

void SlotMachine::drawSymbol(uint8_t symbol, int x, int y, uint8_t brightness) {
    const uint32_t rgb = symbolColors[symbol % symbolCount];
    const uint16_t color = display->color565(((rgb >> 16) & 0xFF) * brightness / 255,
                                             ((rgb >> 8) & 0xFF) * brightness / 255,
                                             (rgb & 0xFF) * brightness / 255);
    for (uint8_t row = 0; row < 5; ++row) {
        const uint8_t bits = pgm_read_byte(&symbols[symbol % symbolCount][row]);
        for (uint8_t col = 0; col < 5; ++col) {
            if (!(bits & (1 << (4 - col)))) continue;
            for (uint8_t py = 0; py < 2; ++py) {
                const int pixelY = y + row * 2 + py;
                if (pixelY < 9 || pixelY > 24) continue;
                drawPixelBoth(x + col * 2, pixelY, color);
                drawPixelBoth(x + col * 2 + 1, pixelY, color);
            }
        }
    }
}

void SlotMachine::drawMarquee(unsigned long now, uint16_t first, uint16_t second) {
    const bool phase = (now / 120) % 2;
    for (int x = 1; x < 49; x += 4) {
        const uint16_t color = ((x / 4) % 2) == phase ? first : second;
        drawPixelBoth(x, 8, color);
        drawPixelBoth(x, 25, color);
    }
    for (int y = 9; y < 25; y += 4) {
        const uint16_t color = ((y / 4) % 2) == phase ? second : first;
        drawPixelBoth(0, y, color);
        drawPixelBoth(49, y, color);
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
