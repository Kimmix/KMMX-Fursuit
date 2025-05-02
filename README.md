<div align="left">

# ✨ KMMX-Fursuit Controller ✨

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![IDE: PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-orange.svg)](https://platformio.org/)

</div>

## 🌟 Features

- **Animated LED Matrix Displays** for eyes and mouth expressions
- **Real-time facial animations** including blinking, emoting, and mouth movements
- **Audio-reactive visemes** that respond to speech
- **Proximity sensing** for interactive "booping" responses
- **Accelerometer integration** for motion-based animations and responses
- **Bluetooth connectivity** for remote control and configuration
- **Customizable expressions** with easy bitmap conversion tools

## 🦊 Demo & Gallery

<div align="center">
  <i>[ Future gallery of photos/videos showing the fursuit in action ]</i>
</div>

## 🛠️ Hardware

<details>
<summary>Click to expand hardware details</summary>

This project is designed to run on an ESP32-based custom board (specifically the ESP32-S3) and includes:

- **HUB75 LED Matrix panels** (64x32 pixel resolution)
- **APDS9930** proximity sensor for booping interaction
- **LIS3DH** accelerometer for motion detection
- **I2S Microphone** for audio input (viseme detection)
- **WS2812 RGB LEDs** for cheek panels and status indicators
- **PWM-controlled LEDs** for horn illumination

</details>

## 📋 Project Structure

<details>
<summary>Click to expand project structure</summary>

- **`src/`** - Main source code
  - **`Bitmaps/`** - Bitmap assets for eye and mouth animations
  - **`Devices/`** - Hardware driver implementations
  - **`FacialStates/`** - Facial animation state machines
  - **`KMMXController/`** - Main controller logic
  - **`Network/`** - Bluetooth connectivity
  - **`Renderer/`** - Animation and rendering code
  - **`Utils/`** - Helper functions
- **`bitmapTool/`** - Tools for converting images to bitmaps
- **`include/`** - Header files
- **`lib/`** - External libraries
- **`boards/`** - Custom board definitions

</details>

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 development board (preferably ESP32-S3)
- LED matrix panels and other hardware components as specified in the code

### Setup

<details>
<summary>Click to expand setup instructions</summary>

1. Clone the repository:

   ```
   git clone https://github.com/yourusername/KMMX-Fursuit.git
   ```

2. Open the project in PlatformIO or Arduino IDE

3. Install the required dependencies (listed in platformio.ini):
   - ArduinoBLE
   - Adafruit GFX Library
   - Adafruit MPR121
   - arduinoFFT
   - Adafruit LIS3DH
   - ESP32 HUB75 LED MATRIX PANEL DMA Display
   - APDS-9930 Ambient Light and Proximity Sensor
   - Adafruit NeoPixel

4. Configure your hardware connections in `src/config.h`

5. Build and upload to your ESP32 board

</details>

## ⚙️ Configuration

The project is highly configurable. Main settings can be adjusted in `src/config.h`, including:

- Pin assignments for all hardware components
- LED matrix resolution and brightness settings
- Animation timing parameters
- Sensor sensitivity thresholds
- BLE connectivity settings

## 🎮 Usage

Once programmed, the controller will:

1. Initialize all hardware components
2. Start displaying default eye and mouth animations
3. Respond to "boops" via the proximity sensor
4. React to speech and sounds with viseme animations
5. Respond to physical movement via the accelerometer
6. Accept Bluetooth commands for changing expressions and settings

## ✨ Animation Creation

Create your own custom animations using the bitmap tool:

1. Design your animations in Adobe Photoshop or After Effects
2. Use the provided converter tool in `bitmapTool/` to convert to the proper format
3. Add the generated header files to your project
4. Configure the new animations in the controller

## 📄 License

This project is licensed under the terms specified in the LICENSE file.

## 📱 Contact

<div align="center">

  <a href="https://github.com/yourusername/KMMX-Fursuit/issues">
    <img src="https://img.shields.io/badge/GitHub-Issues-red.svg">
  </a>
  <a href="https://discord.com/">
    <img src="https://img.shields.io/badge/Discord-kimmix-7289DA.svg">
  </a>

  <p>For questions or support, please open an issue in the GitHub repository or reach out on Discord: kimmix</p>

</div>

---