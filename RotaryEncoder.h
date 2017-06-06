/*
  RotaryEncoder.cpp
  Very simple class for 2 bit gray code rotary encoders
  
  Please see https://idyl.io for complete tutorial on how this works.
  
  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 4th March 2017
  by Tony Pottier
  
*/



class RotaryEncoder{

	private:
		uint8_t _a;
		uint8_t _b;
		uint8_t _aLastValue;
		int8_t _relativeValue;
    Button* _btn;
	public:
		RotaryEncoder();
		void setup(uint8_t pinA, uint8_t pinB, uint8_t pinButton);
		void poll();
    Button* getButton();
		int8_t read();
};
