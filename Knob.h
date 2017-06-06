/*
  Knob.h
  Very simple class for 2 bit gray code rotary encoders with a button.
  The button is hardware debounced and doesn't need any further software debounce as such

  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 19th March 2017
  by Tony Pottier
  https://idyl.io
  
*/

class Knob{

	private:
		uint8_t _a;
		uint8_t _b;
		uint8_t _pinButton;
		uint8_t _aLastValue;
		int8_t _relativeValue;
		uint8_t _lastButtonValue;
		uint8_t _falling;
	public:
		Knob();
		void setup(uint8_t pinA, uint8_t pinB, uint8_t pinButton);
    void reset();
		void poll();
		int8_t read();
		uint8_t falling();
};
