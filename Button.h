/*
  Button.h
  Class to handle a hardware debounced button on Arduino
  
  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 1st of June February 2017
  by Tony Pottier
  https://idyl.io
  
*/

#define UNKNOWN B10101010

class Button{
  private:
    uint8_t state;
    uint8_t pinNumber;
    uint8_t edge;
    uint8_t invertMode;
	void (*functionRising)();
	void (*functionFalling)();
	void init();
  public: 
    void setup(const uint8_t pin, const uint8_t pinModeValue, const uint8_t invert);
    void poll();
    void reset();
    uint8_t rising();
    uint8_t falling();
    Button();
	  void attachFunction(void (*function)(), uint8_t edgeType);
	  uint8_t getState();
};
