class FacialState {
   protected:
    LEDMatrixDisplay* display;

   public:
    FacialState(LEDMatrixDisplay* displayPtr = nullptr) : display(displayPtr) {}
    virtual void update() = 0;
};