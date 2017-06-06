/*
  RotaryEncoder.cpp
  Very simple class for 2 bit gray code rotary encoders
  
  Please see https://idyl.io for complete tutorial on how this works.
  
  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 4th March 2017
  by Tony Pottier
  
*/

#include <stdint.h>
#include <arduino.h>
#include "Button.h"
#include "RotaryEncoder.h"


RotaryEncoder::RotaryEncoder(){

}

Button* RotaryEncoder::getButton(){
  return _btn;
}

void RotaryEncoder::setup(uint8_t pinA, uint8_t pinB, uint8_t pinButton){
  this->_a = pinA;
  this->_b = pinB;
  _btn = new Button();
  _btn->setup(pinButton, INPUT_PULLUP, true);
	pinMode(_a, INPUT_PULLUP);
	pinMode(_b, INPUT_PULLUP);
	_aLastValue = digitalRead(_a);
	_relativeValue = 0;
}

void RotaryEncoder::poll(){
	
	uint8_t aNow = digitalRead(_a);
	if(_aLastValue == LOW &&  aNow == HIGH){
		if(digitalRead(_b) == LOW){
			_relativeValue--;
		}
		else{
			_relativeValue++;
		}
	}
	_aLastValue = aNow;
	
}

int8_t RotaryEncoder::read(){
	int8_t tmp = _relativeValue;
	_relativeValue = 0; //reset value at every reading
	return tmp;
}
