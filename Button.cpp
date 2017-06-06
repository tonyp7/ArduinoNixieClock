/*
  Button.cpp
  Class to handle a hardware debounced button on Arduino
  
  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 1st of June February 2017
  by Tony Pottier
  https://idyl.io
  
*/

#include <stdint.h>
#include <arduino.h>
#include "Button.h"


Button::Button(){
  
}



void Button::setup(const uint8_t pin, const uint8_t pinModeValue, const uint8_t invert){
  pinMode(pin, pinModeValue);
  this->pinNumber = pin;
  this->invertMode = invert;
  this->edge = UNKNOWN;
  this->state = this->invertMode?!digitalRead(this->pinNumber):digitalRead(this->pinNumber);
  functionRising = NULL;
  functionFalling = NULL;
  
}

void Button::reset(){
  this->edge = UNKNOWN;
  this->state = this->invertMode?!digitalRead(this->pinNumber):digitalRead(this->pinNumber);
}



void Button::attachFunction(void (*function)(), uint8_t edgeType){
  if(edgeType == RISING){
    this->functionRising = function;
  }
  else{
    this->functionFalling = function;
  }
}


/// Read the input and perform edge detection if necessary
void Button::poll(){
  uint8_t reading = this->invertMode ? !digitalRead(this->pinNumber):digitalRead(this->pinNumber);

  if (reading != this->state) {
    this->state = reading;
    if(reading){
      if(functionRising){ functionRising();}
      else{this->edge = RISING;}
    }
    else{
      if(functionFalling){ functionFalling();}
      else{this->edge = FALLING;}
    }
  }
}



/// Returns true if a rising edge is detected. Edge must be processed after a succesful call. calling this function reset the edge to unknown state.
uint8_t Button::rising(){
  if(edge == RISING){
    edge = UNKNOWN;
    return true;
  }
  return false;
}

/// Returns true if a falling edge is detected. Edge must be processed after a succesful call. calling this function reset the edge to unknown state.
uint8_t Button::falling(){
  if(edge == FALLING){
    edge = UNKNOWN;
    return true;
  }
  return false;
}

uint8_t Button::getState(){
	return this->state;
}
