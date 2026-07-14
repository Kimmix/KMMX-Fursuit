#include "KMMXController.h"

namespace {
constexpr int hornDimBrightness = 5;
constexpr int hornFlashBrightness = 100;
constexpr int hornFadeSpeed = 20;
const unsigned long hornWinIntervals[] = {220, 300, 120, 75};
constexpr unsigned long hornReelTickMs = 80;
constexpr unsigned long hornAnticipationIntervalMs = 110;
const uint32_t slotSymbolColors[] = {0x45E06F, 0xFF4C8B, 0xFFD83D, 0x52C7FF};
const uint32_t slotRainbowColors[] = {0xFF3030, 0xFFE13D, 0x45E06F, 0x52C7FF, 0xB060FF};
constexpr unsigned long cheekUpdateIntervalMs = 50;
}

void KMMXController::setSlotMachineEnabled(int enabled) {
    slotMachine.setEnabled(enabled == 1);
}

int KMMXController::getSlotMachineEnabled() const {
    return slotMachine.isEnabled();
}

void KMMXController::updateSlotMachineCheeks() {
    static bool wasActive = false;
    static unsigned long lastUpdate = 0;
    const bool active = slotMachine.isEnabled();

    if (!active) {
        if (wasActive) cheekPanel.resume();
        wasActive = false;
        return;
    }

    const unsigned long now = millis();
    if (wasActive && now - lastUpdate < cheekUpdateIntervalMs) return;
    wasActive = true;
    lastUpdate = now;

    const uint8_t configuredBrightness = cheekPanel.getBrightness();
    const SlotMachine::State state = slotMachine.getState();
    uint32_t first = 0x652080;
    uint32_t second = 0xFF4C8B;
    uint8_t offset = now / 180;
    uint8_t brightness = configuredBrightness;

    if (state == SlotMachine::State::READY) {
        const uint8_t charge = slotMachine.getCharge();
        second = charge ? 0xFFD83D : 0xFF4C8B;
        brightness = configuredBrightness * (30 + charge * 70 / 100) / 100;
    } else if (state == SlotMachine::State::SPINNING) {
        offset = now / (slotMachine.isAnticipating() ? 45 : 75);
        if (slotMachine.isAnticipating()) {
            first = 0xFFD83D;
            second = 0xFF30A0;
            brightness = (now / 110) % 2 ? configuredBrightness : configuredBrightness / 3;
        } else {
            first = 0x52C7FF;
            second = 0xB060FF;
        }
    } else if (state == SlotMachine::State::REVEAL) {
        first = 0xFFFFFF;
        second = slotSymbolColors[slotMachine.getResultSymbol() % 4];
        offset = now / 45;
    } else if (slotMachine.getOutcome() == SlotMachine::Outcome::WIN) {
        const uint8_t symbol = slotMachine.getResultSymbol() % 4;
        if (symbol == 3) {
            const uint8_t rainbow = (now / 90) % 5;
            first = slotRainbowColors[rainbow];
            second = slotRainbowColors[(rainbow + 1) % 5];
        } else {
            first = slotSymbolColors[symbol];
            second = 0xFFFFFF;
        }
        offset = now / 55;
        brightness = (now / 120) % 2 ? configuredBrightness : configuredBrightness * 2 / 3;
    } else {
        first = 0x500000;
        second = 0xFF3030;
        offset = 0;
        brightness = configuredBrightness / 3;
    }

    cheekPanel.showPattern(first, second, offset, brightness);
}

void KMMXController::updateSlotMachineHorn() {
    const SlotMachine::Outcome outcome = slotMachine.getOutcome();
    const unsigned long now = millis();
    const bool active = slotMachine.isEnabled();

    if (active != slotHornWasEnabled) {
        if (active) slotHornBrightness = hornLED.getBrightness();
        else hornLED.setBrightness(slotHornBrightness, hornFadeSpeed);
        slotHornOutcome = SlotMachine::Outcome::NONE;
        slotHornStoppedReels = 0;
        slotHornAnticipating = false;
        slotHornWasEnabled = active;
    }
    if (!active) return;

    const uint8_t stoppedReels = slotMachine.getStoppedReels();
    const bool anticipating = slotMachine.isAnticipating();
    if (stoppedReels > slotHornStoppedReels) {
        hornLED.setBrightness(hornFlashBrightness, hornFadeSpeed);
        slotHornCueUntil = now + hornReelTickMs;
    }
    slotHornStoppedReels = stoppedReels;

    if (outcome == SlotMachine::Outcome::NONE) {
        if (anticipating && now - lastSlotHornFlash >= hornAnticipationIntervalMs) {
            lastSlotHornFlash = now;
            slotHornFlashOn = !slotHornFlashOn;
            hornLED.setBrightness(slotHornFlashOn ? 75 : 20, hornFadeSpeed);
        } else if (!anticipating && slotHornCueUntil) {
            if (now >= slotHornCueUntil) {
                slotHornCueUntil = 0;
                hornLED.setBrightness(slotHornBrightness, hornFadeSpeed);
            }
        } else if (!anticipating && slotHornAnticipating) {
            hornLED.setBrightness(slotHornBrightness, hornFadeSpeed);
        }
    }
    slotHornAnticipating = anticipating;

    if (outcome != slotHornOutcome) {
        if (outcome == SlotMachine::Outcome::WIN) {
            slotHornFlashOn = true;
            lastSlotHornFlash = now;
            hornLED.setBrightness(hornFlashBrightness, hornFadeSpeed);
        } else if (outcome == SlotMachine::Outcome::LOSE) {
            hornLED.setBrightness(hornDimBrightness, hornFadeSpeed);
        } else {
            hornLED.setBrightness(slotHornBrightness, hornFadeSpeed);
        }
        slotHornOutcome = outcome;
    }

    const unsigned long winInterval = hornWinIntervals[slotMachine.getResultSymbol() % 4];
    if (outcome == SlotMachine::Outcome::WIN && now - lastSlotHornFlash >= winInterval) {
        lastSlotHornFlash = now;
        slotHornFlashOn = !slotHornFlashOn;
        hornLED.setBrightness(slotHornFlashOn ? hornFlashBrightness : 0, hornFadeSpeed);
    }
}
