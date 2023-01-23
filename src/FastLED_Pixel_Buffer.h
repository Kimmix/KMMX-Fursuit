#include <Arduino.h>

#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <FastLED.h>

// Configure for your panel(s) as appropriate!
#define PANEL_RES_X \
  64  // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y \
  32  // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2  // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1  // Number of INDIVIDUAL PANELS per ROW
#define PANEL_CHAIN \
  NUM_ROWS* NUM_COLS  // total number of panels chained one to another

// Change this to your needs, for details on VirtualPanel pls read the PDF!
#define SERPENT true

// placeholder for the matrix object
MatrixPanel_I2S_DMA* p2matrix = nullptr;

// placeholder for the virtual display object
VirtualMatrixPanel* Pixel_Buff = nullptr;

uint8_t brightness;

class VirtualMatrixPanel_FastLED_Pixel_Buffer : public VirtualMatrixPanel {
 public:
  using VirtualMatrixPanel::VirtualMatrixPanel;  // perform VirtualMatrixPanel
                                                 // class constructor
  void Initialize() {
    HUB75_I2S_CFG mxconfig(PANEL_RES_X,  // module width
                           PANEL_RES_Y,  // module height
                           PANEL_CHAIN   // chain length
    );

    // OK, now we can create our matrix object
    p2matrix = new MatrixPanel_I2S_DMA(mxconfig);

    // let's adjust default brightness to about 75%
    p2matrix->setBrightness8(128);  // range is 0-255, 0 - 0%, 255 - 100%

    // Allocate memory and start DMA display
    if (!p2matrix->begin()) {
      Serial.println("****** I2S memory allocation failed ***********");
    }

    // create VirtualDisplay object based on our newly created p2matrix
    // object
    Pixel_Buff = new VirtualMatrixPanel((*p2matrix), NUM_ROWS, NUM_COLS,
                                        PANEL_RES_X, PANEL_RES_Y, SERPENT);

    p2matrix->clearScreen();
  }

  bool allocateMemory()  // allocate memory
  {
    // https://www.geeksforgeeks.org/how-to-declare-a-2d-array-dynamically-in-c-using-new-operator/
    buffer = new CRGB[virtualResX *
                      virtualResY];  // These are defined in the underliny

    if (!buffer) {
      return false;
    }

    Serial.printf("Allocated %d bytes of memory for pixel buffer.\r\n",
                  sizeof(CRGB) * ((virtualResX * virtualResY) + 1));
    this->clear();

    return true;

  }  // end Buffer

  // For adafruit
  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Serial.println("calling our drawpixel!");

    // 565 color conversion
    uint8_t r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
    uint8_t g = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
    uint8_t b = (((color & 0x1F) * 527) + 23) >> 6;

    this->drawPixel(x, y, CRGB(r, g, b));
  }

  void drawPixel(int16_t x, int16_t y, int r, int g, int b) {
    this->drawPixel(x, y, CRGB(r, g, b));
  }

  // We actually just draw to ourselves... to our buffer
  void drawPixel(int16_t x, int16_t y, CRGB color) {
    // Serial.printf("updated x y : %d %d", x, y);
    buffer[XY16(x, y)] = color;
  }

  CRGB getPixel(int16_t x, int16_t y) { return buffer[XY16(x, y)]; }

  /**
   * Dim all the pixels on the layer.
   */
  void dimAll(byte value) {
    // Serial.println("performing dimall");
    //  nscale8 max value is 255, or it'll flip back to 0
    //  (documentation is wrong when it says x/256), it's actually x/255
    /*
    for (int y = 0; y < LAYER_HEIGHT; y++) {
        for (int x = 0; x < LAYER_WIDTH; x++) {
            pixels->data[y][x].nscale8(value);
    }}
    */
    dimRect(0, 0, virtualResX, virtualResY, value);
  }

  /**
   * Dim all the pixels in a rectangular option of the layer the layer.
   */
  void dimRect(int16_t x, int16_t y, int16_t w, int16_t h, byte value) {
    for (int16_t i = x; i < x + w; i++) {
      for (int16_t j = y; j < y + h; j++) {
        buffer[XY16(i, j)].nscale8(value);
      }
    }
  }

  void clear() { memset(buffer, CRGB(0, 0, 0), (virtualResX * virtualResY)); }

  /**
   * Actually Send the CRGB FastLED buffer to the DMA engine / Physical
   * Panels! Do this via the underlying 'VirtualMatrixPanel' that does all
   * the pixel-remapping for all sorts of chained panels, and panel scan
   * types.
   */
  void show() {
    // Serial.println("Doing Show");

    CRGB _pixel = 0;
    for (int16_t y = 0; y < virtualResY; y++) {
      for (int16_t x = 0; x < virtualResX; x++) {
        // VirtualMatrixPanel::getCoords(x, y); // call to base to update
        // coords for chaining approach
        _pixel = buffer[XY16(x, y)];
        drawPixelRGB888(
            x, y, _pixel.r, _pixel.g,
            _pixel.b);  // call VirtualMatrixPanel::drawPixelRGB888(...)
                        // drawPixelRGB888( x, y, 0, 0, 128); // call
        // VirtualMatrixPanel::drawPixelRGB888(...)
      }  // end loop to copy fast led to the dma matrix
    }

  }  // show

  // Release Memory
  ~VirtualMatrixPanel_FastLED_Pixel_Buffer(void) { delete (buffer); }

 protected:
  uint16_t XY16(uint16_t x, uint16_t y) {
    if (x >= virtualResX)
      return 0;
    if (y >= virtualResY)
      return 0;

    return (y * virtualResX) + x +
           1;  // everything offset by one to compute out of bounds stuff -
               // never displayed by ShowFrame()
  }

 private:
  CRGB* buffer = nullptr;
};