#include "KMMXController.h"

namespace {
constexpr int hornDimBrightness = 5;
constexpr int hornFlashBrightness = 100;
constexpr int hornFadeSpeed = 20;
const unsigned long hornWinIntervals[] = {220, 300, 120, 75};
constexpr unsigned long hornReelTickMs = 80;
constexpr unsigned long hornAnticipationIntervalMs = 110;
}

void KMMXController::setSlotMachineEnabled(int enabled) {
    slotMachine.setEnabled(enabled == 1);
}

int KMMXController::getSlotMachineEnabled() const {
    return slotMachine.isEnabled();
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
