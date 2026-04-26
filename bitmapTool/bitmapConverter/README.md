# Bitmap Converter Tool

A Node.js utility for converting GIF animations into C++ header files containing grayscale bitmap arrays for the KMMX-Fursuit LED matrix displays.

## 📋 Overview

This tool takes animated GIF files and converts each frame into a grayscale bitmap array that can be used directly in the KMMX-Fursuit firmware. The output is a C++ header file with PROGMEM arrays that can be imported into the project to display custom animations on the LED matrix panels.

## 🔧 Prerequisites

- [Node.js](https://nodejs.org/) (v14.0.0 or higher recommended)
- NPM (Node Package Manager)

## 🚀 Installation

1. Navigate to the bitmapConverter directory:

   ```sh
   cd bitmapTool/bitmapConverter
   ```

1. Install the required dependencies:

   ```sh
   npm install
   ```

## 📁 Directory Structure

- `input/` - Place your GIF files here
- `output/` - Generated header files will be saved here

## 🎮 Usage

1. Place your GIF animation frames in the `input/` directory
   - Each frame should be saved as a separate GIF file
   - Files will be processed in alphabetical order, so name them accordingly (e.g., frame01.gif, frame02.gif, etc.)

1. Run the converter with your desired animation name and output type:

   **For standard greyscale output:**

   ```sh
   node bmpToGreyscale.js yourAnimationName raw
   ```

   or simply

   ```sh
   node bmpToGreyscale.js yourAnimationName
   ```

   If no name is provided, it will default to "defaultBitmap":

   ```sh
   node bmpToGreyscale.js
   ```

1. The generated header file will be saved to `output/yourAnimationName.h`

## 🖼️ Input Image Requirements

- GIF format
- Images should match the dimensions expected by the KMMX-Fursuit firmware
  - For eyes: 32×18 pixels
  - For mouth: 50×14 pixels
- For best results, ensure your images have appropriate contrast for grayscale conversion

## 📄 Output Format

The generated header file contains PROGMEM arrays for each frame of the animation in the following format:

```cpp
// Generated output
static const uint8_t PROGMEM yourAnimationName0[576] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... more rows of pixel data
};

static const uint8_t PROGMEM yourAnimationName1[576] = {
    // ... next frame data
};
// ... more frames
```

## 🔄 Integration

After generating the header file, you need to:

1. Copy the generated `.h` file to the appropriate directory in your firmware project:
   - For eyes: `src/Bitmaps/eyes/`
   - For mouth: `src/Bitmaps/mouth/`

1. Update the corresponding bitmap include file:
   - For eyes: `src/Bitmaps/eyes/eyeBitmaps.h`
   - For mouth: `src/Bitmaps/mouth/mouthBitmaps.h`

## 📝 Examples

### Basic Usage

```sh
node bmpToGreyscale.js eyeBlink
```

Result: `output/eyeBlink.h` with arrays named eyeBlink0, eyeBlink1, eyeBlink2, etc.

### Using Default Name

```sh
node bmpToGreyscale.js
```

Result: `output/defaultBitmap.h` with arrays named defaultBitmap0, defaultBitmap1, defaultBitmap2, etc.

## ⚠️ Troubleshooting

- If you encounter errors related to the Sharp library, ensure you have the required system dependencies installed according to [Sharp's installation guide](https://sharp.pixelplumbing.com/install).
- Make sure your input directory contains valid GIF files.
- Check that you have write permissions for the output directory.

## 📚 Additional Resources

- [Sharp Documentation](https://sharp.pixelplumbing.com/)
- See the KMMX-Fursuit main README for more information on how bitmap animations are used in the project
