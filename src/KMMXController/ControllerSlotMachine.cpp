#include "KMMXController.h"

namespace {
constexpr int hornDimBrightness = 5;
constexpr int hornFlashBrightness = 100;
constexpr int hornFadeSpeed = 20;
constexpr unsigned long hornFlashIntervalMs = 200;
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

    if (outcome != slotHornOutcome) {
        if (slotHornOutcome == SlotMachine::Outcome::NONE && outcome != SlotMachine::Outcome::NONE) {
            slotHornBrightness = hornLED.getBrightness();
        }
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

    if (outcome == SlotMachine::Outcome::WIN && now - lastSlotHornFlash >= hornFlashIntervalMs) {
        lastSlotHornFlash = now;
        slotHornFlashOn = !slotHornFlashOn;
        hornLED.setBrightness(slotHornFlashOn ? hornFlashBrightness : 0, hornFadeSpeed);
    }
}
