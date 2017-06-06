/*
  Knob.cpp
  Very simple class for 2 bit gray code rotary encoders with a button.
  The button is hardware debounced and doesn't need any further software debounce as such.

  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 19th March 2017
  by Tony Pottier
  https://idyl.io
  
*/

#include <stdint.h>
#include <arduino.h>
#include "Knob.h"


Knob::Knob(){}

void Knob::setup(uint8_t pinA, uint8_t pinB, uint8_t pinButton){
	this->_a = pinA;
	this->_b = pinB;
	this->_pinButton = pinButton;

	pinMode(this->_a, INPUT_PULLUP);
	pinMode(this->_b, INPUT_PULLUP);
	pinMode(this->_pinButton, INPUT_PULLUP);  

	this->reset();
}

void Knob::reset(){
  this->_aLastValue = digitalRead(this->_a);
  this->_lastButtonValue = digitalRead(this->_pinButton);
  this->_relativeValue = 0;
  this->_falling = false;
}

void Knob::poll(){
	
	//poll the button
	uint8_t btnNow = digitalRead(_pinButton);
	if(btnNow != _lastButtonValue){
		if(btnNow == HIGH){
			//falling edge --remind the logic is inverted due to INPUT_PULLUP
			_falling = true;
		}
	}
	_lastButtonValue = btnNow;
	
	//poll the rotary encoder
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

int8_t Knob::read(){
	int8_t tmp = _relativeValue;
	_relativeValue = 0; //reset value at every reading
	return tmp;
}

uint8_t Knob::falling(){
	if(_falling){
		_falling = false;
		return true;
	}
	return false;
}
	
