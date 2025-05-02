# KMMX-Fursuit Controller

An advanced electronic fursuit controller system that powers LED matrix displays and sensors for interactive and animated fursuit faces.

## 🌟 Features

- **Animated LED Matrix Displays** for eyes and mouth expressions
- **Real-time facial animations** including blinking, emoting, and mouth movements
- **Audio-reactive visemes** that respond to speech
- **Proximity sensing** for interactive "booping" responses
- **Accelerometer integration** for motion-based animations and responses
- **Bluetooth connectivity** for remote control and configuration
- **Power-efficient** sleep mode and smart idle detection

## 🛠️ Hardware

This project is designed to run on an ESP32-based custom board (specifically the ESP32-S3) and includes:

- **HUB75 LED Matrix panels** (64x32 pixel resolution)
- **APDS9930** proximity sensor for booping interaction
- **LIS3DH** accelerometer for motion detection
- **I2S Microphone** for audio input (viseme detection)
- **WS2812 RGB LEDs** for cheek panels and status indicators
- **PWM-controlled LEDs** for horn illumination

## 📋 Project Structure

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

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 development board (preferably ESP32-S3)
- LED matrix panels and other hardware components as specified in the code

### Setup

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

## 👥 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the terms specified in the LICENSE file.

## 📱 Contact

For questions or support, please open an issue in the GitHub repository.