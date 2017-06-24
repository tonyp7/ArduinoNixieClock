
/*
  NixieClock
  Open source code for Nixie Clock driven by Arduino Mega

  Please visit https://idyl.io to see a Nixie clock running this code in action!

  This example code is licensed under CC BY 4.0.
  Please see https://creativecommons.org/licenses/by/4.0/

  modified 1st of June 2017
  by Tony Pottier
  https://idyl.io
  
 *
 *   ARDUINO NIXIE CLOCK
 * 
 *   NIXIE1 NIXIE2   NIXIE3 NIXIE4   NIXIE5 NIXIE6
 *     __    __        __    __        __    __
 *    /  \  /  \      /  \  /  \      /  \  /  \
 *    | 2|  | 3|      | 5|  | 9|      | 5|  | 9|
 * ___|__|__|__|______|__|__|__|______|__|__|__|___
 * | LEFT DISPLAY | CENTER DISPLAY | RIGHT DISPLAY |
 * |     O        |      o         |      o        |
 * | LEFT KNOB    | CENTER KNOB    | RIGHT KNOB    |
 * |_______________________________________________|
 * 
 * MAPPING OF THE ARDUINO MEGA
 * 
 * PORT K
 * K7  K6  K5  K4  K3  K2  K1 K0 - PORT PIN
 * A15 A14 A13 A12 A11 A10 A9 A8 - ARDUINO IDE PIN #
 * D   C   B   A   D   C   B  A  - CONNECTION TO BCD DECODER
 * 
 * PORT L
 * L7  L6  L5  L4  L3  L2  L1  L0
 * 42  43  44  45  46  47  48  49
 * D   C   B   A   D   C   B   A
 * 
 * PORT C
 * C7  C6  C5  C4  C3  C2  C1  C0
 * 30  31  32  33  34  35  36  37
 * D   C   B   A   D   C   B   A  
 * 
 * PORT D
 * 20   21   19   18 
 * SDA  SCL  RX   TX
 * RTC  RTC
 * 
 * PORT A
 * A7  A6  A5  A4  A3  A2  A1  A0
 * 29  28  27  26  25  24  23  22
 * H_S H_A H_B M_S M_A M_B 
 * 
 * PORT F
 * F7  F6  F5  F4  F3  F2  F1  F0
 * A7  A6  A5  A4  A3  A2  A1  A0
 * S_S S_A S_B SW  BPR PWR 
 */

#include "Button.h"
#include "Knob.h"
#include "TinyGPS++.h"
#include <SoftwareSerial.h>
#include "RTClib.h"

RTC_DS3231 rtc;

#define DEBUG

#define PORT_DISPLAY_LEFT PORTK
#define PORT_DISPLAY_LEFT_REGISTER DDRK

#define PORT_DISPLAY_CENTER PORTL
#define PORT_DISPLAY_CENTER_REGISTER DDRL

#define PORT_DISPLAY_RIGHT PORTC 
#define PORT_DISPLAY_RIGHT_REGISTER DDRC

#define STATE_BOOTING_UP 0x00
#define STATE_DISPLAY_TIME 0x01
#define STATE_DISPLAY_DATE 0x02

#define PIN_POWER_SWITCH (uint8_t)A2
#define PIN_BUTTON_POWER_SWITCH (uint8_t)A3  
#define PIN_BUTTON_DATE_DISPLAY (uint8_t)A4

#define PIN_ROTARY_ENCODER_H_A (uint8_t)27
#define PIN_ROTARY_ENCODER_H_B (uint8_t)28
#define PIN_ROTARY_ENCODER_H_BUTTON (uint8_t)29

#define PIN_ROTARY_ENCODER_M_A (uint8_t)24
#define PIN_ROTARY_ENCODER_M_B (uint8_t)25
#define PIN_ROTARY_ENCODER_M_BUTTON (uint8_t)26

#define PIN_ROTARY_ENCODER_S_A (uint8_t)A5
#define PIN_ROTARY_ENCODER_S_B (uint8_t)A6
#define PIN_ROTARY_ENCODER_S_BUTTON (uint8_t)A7

#define BLINK_SPEED (uint16_t)400


uint8_t portLeftValue, portCenterValue, portRightValue;

uint8_t state;

uint8_t blinker;
uint16_t blinkerAccumulator;

int8_t h, m, s, dd, mm, yy;
uint8_t setLeft, setCenter, setRight;
DateTime now, previous;


uint32_t currentMillis = 0UL;
uint32_t previousMillis = 0UL;
uint32_t elapsedCurrentState = 0UL;
uint16_t elapsed = 0;

Knob knobLeft, knobCenter, knobRight;
Button btnDateDisplay, btnPowerSwitch;

void changeState(uint8_t newState){
  state = newState;
  elapsedCurrentState = 0UL;
}

void setup() {

#ifdef DEBUG
  Serial.begin(57600);
#endif

  //turn OFF the display ASAP!
  pinMode(PIN_POWER_SWITCH, OUTPUT);
  digitalWrite(PIN_POWER_SWITCH, LOW);
  
  //Set port as output (all 8 pins)
  PORT_DISPLAY_LEFT_REGISTER = 0xFF;
  PORT_DISPLAY_CENTER_REGISTER = 0xFF;
  PORT_DISPLAY_RIGHT_REGISTER = 0xFF;

  blinker = false;
  blinkerAccumulator = 0;
  previousMillis = currentMillis = 0UL;
  
  //set initial variables
  state = STATE_BOOTING_UP;
  h = m = s = dd = mm = yy = 0;
  setLeft = setCenter = setRight = false;
  portLeftValue = portCenterValue = portRightValue = 0;

  //register the rotary encoders
  knobLeft.setup(PIN_ROTARY_ENCODER_H_A, PIN_ROTARY_ENCODER_H_B, PIN_ROTARY_ENCODER_H_BUTTON);
  knobCenter.setup(PIN_ROTARY_ENCODER_M_A, PIN_ROTARY_ENCODER_M_B, PIN_ROTARY_ENCODER_M_BUTTON);
  knobRight.setup(PIN_ROTARY_ENCODER_S_A, PIN_ROTARY_ENCODER_S_B, PIN_ROTARY_ENCODER_S_BUTTON);

  //register the button to switch power and display
  btnDateDisplay.setup(PIN_BUTTON_DATE_DISPLAY, INPUT_PULLUP, true);
  btnPowerSwitch.setup(PIN_BUTTON_POWER_SWITCH, INPUT_PULLUP, true);

    
  //All buttons are hardware debounced with resistor/capacitors
  //This causes incorrect edge detections on the first power up
  //This reset all user buttons
  delay(100);
  btnPowerSwitch.reset();
  knobLeft.reset();
  knobCenter.reset();
  knobRight.reset();
  btnDateDisplay.reset();
  btnPowerSwitch.reset();

  //Setup the display according to the power status at startup
  btnPowerSwitch.poll();
  digitalWrite(PIN_POWER_SWITCH, btnPowerSwitch.getState());
 
}

void loop() {
  //timing related stuff, to timeout if the clock
  //stays in the same state for too long
  previousMillis = currentMillis;
  currentMillis = millis();
  elapsed = (uint16_t)(currentMillis - previousMillis);
  elapsedCurrentState += elapsed;


  

  
  if(state == STATE_BOOTING_UP){

    //start the real time clock!
    rtc.begin();
    if(rtc.lostPower()){
      //set to build time
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    //short animation at startup
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x00; delay(100);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x11; delay(50);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x22; delay(50);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x33; delay(50);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x44; delay(50);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x55; delay(50);  
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x66; delay(50);  
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x77; delay(50);  
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x88; delay(50);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x99; delay(100);
    PORT_DISPLAY_LEFT = PORT_DISPLAY_CENTER = PORT_DISPLAY_RIGHT = 0x00; delay(200);

    //flush the display with current time
    previous = now = rtc.now();
    updateTime();
    updateDate(false);
    flushDisplay();
    changeState(STATE_DISPLAY_TIME);
  }
  else if(state == STATE_DISPLAY_TIME){

    //poll knob and button
    knobLeft.poll();
    knobCenter.poll();
    knobRight.poll();
    btnDateDisplay.poll();

    //blinker
    blinkerAccumulator += elapsed;
    while(blinkerAccumulator >= BLINK_SPEED){
      blinkerAccumulator -= BLINK_SPEED;
      blinker = !blinker;

      if(blinker){
        if(setLeft){
          numberToPort(&portLeftValue,0xff);
          flushDisplay(true, false, false);
        }
        if(setCenter){
          numberToPort(&portCenterValue,0xff);
          flushDisplay(false, true, false);
        }
        if(setRight){
          numberToPort(&portRightValue,0xff);
          flushDisplay(false, false, true);
        }
      }
      else{
        if(setLeft){
          numberToPort(&portLeftValue,h);
          flushDisplay(true, false, false);
        }
        if(setCenter){
          numberToPort(&portCenterValue,m);
          flushDisplay(false, true, false);
        }
        if(setRight){
          numberToPort(&portRightValue,s);
          flushDisplay(false, false, true);
        }
      }
      
    }

    //query real time clock & update display if time has changed.
    now = rtc.now();
    if(previous.second() != now.second()){
      updateTime();
      previous = now;
    }
    
    //update left (=Hours)
    if(setLeft){
      int8_t tmp = knobLeft.read();
      if(tmp != 0){
        h += tmp;
        clampValue(&h, (int8_t)0, (int8_t)23);
        //flush display
        numberToPort(&portLeftValue,h);
        flushDisplay(true, false, false);
      }
    }

    //update center (=Minutes)
    if(setCenter){
      int8_t tmp = knobCenter.read();
      if(tmp != 0){
        m += tmp;
        clampValue(&m, (int8_t)0, (int8_t)59);
        //flush display
        numberToPort(&portCenterValue,m);
        flushDisplay(false, true, false);
      }
    }

    //update right (=Seconds)
    if(setRight){
      int8_t tmp = knobRight.read();
      if(tmp != 0){
        s += tmp;
        clampValue(&s, (int8_t)0, (int8_t)59);
        //flush display
        numberToPort(&portRightValue,s);
        flushDisplay(false, false, true);
      }
    }

    //user wants to set time?
    if(knobLeft.falling()){
      if(setLeft){
        setLeft = false;
        //save HOUR set by the user
        rtc.adjust(DateTime(2000 + now.year(), now.month(), now.day(), h, now.minute(), now.second()));
        numberToPort(&portLeftValue,h);
        flushDisplay(true, false, false);
      }
      else{
        setLeft = true;
        knobLeft.read(); //flush stored value if any
      }
    }

    if(knobCenter.falling()){
      if(setCenter){
        setCenter = false;
        //save MINUTE set by the user
        rtc.adjust(DateTime(2000 + now.year(), now.month(), now.day(), now.hour(), m, now.second()));
        numberToPort(&portCenterValue,m);
        flushDisplay(false, true, false);
      }
      else{
        setCenter = true;
        knobCenter.read(); //flush any stored value if any
      }
    }

    if(knobRight.falling()){
      if(setRight){
        setRight = false;
        //save SECOND set by the user
        rtc.adjust(DateTime(2000 + now.year(), now.month(), now.day(), now.hour(), now.minute(),s));
        numberToPort(&portRightValue,s);
        flushDisplay(false, false, true);
      }
      else{
        setRight = true;
        knobRight.read(); //flush stored value if any
      }
    }


    //change state to display the date
    if(btnDateDisplay.falling()){  
      //user was currently setting the time and requested to display the date?
      //this is an edge case that shouldn't matter so we just discard what was
      //the user setting up. Sort of like a "cancel" button
      setLeft = setCenter = setRight = false;
      blinker = false;
      updateDate();
      flushDisplay(true, true, true); //force refresh of display
      changeState(STATE_DISPLAY_DATE);
    }
    
  
  }
  else if(state == STATE_DISPLAY_DATE){


    //buttons
    knobLeft.poll();
    knobCenter.poll();
    knobRight.poll();
    btnDateDisplay.poll();
    
    //blinker
    blinkerAccumulator += elapsed;
    while(blinkerAccumulator >= BLINK_SPEED){
      blinkerAccumulator -= BLINK_SPEED;
      blinker = !blinker;

      if(blinker){
        if(setLeft){
          numberToPort(&portLeftValue,0xff);
          flushDisplay(true, false, false);
        }
        if(setCenter){
          numberToPort(&portCenterValue,0xff);
          flushDisplay(false, true, false);
        }
        if(setRight){
          numberToPort(&portRightValue,0xff);
          flushDisplay(false, false, true);
        }
      }
      else{
        if(setLeft){
          numberToPort(&portLeftValue,dd);
          flushDisplay(true, false, false);
        }
        if(setCenter){
          numberToPort(&portCenterValue,mm);
          flushDisplay(false, true, false);
        }
        if(setRight){
          numberToPort(&portRightValue,yy);
          flushDisplay(false, false, true);
        }
      }
    }

    //query real time clock and update the date if it has changed
    now = rtc.now();
    if(previous.day() != now.day()){
      updateDate();
      previous = now;
    }


    //update left (=Days)
    if(setLeft){
      int8_t tmp = knobLeft.read();
      if(tmp != 0){
        dd += tmp;
        clampValue(&dd, (int8_t)1, (int8_t)31);
        //flush display
        numberToPort(&portLeftValue,dd);
        flushDisplay(true, false, false);
      }
    }

    //update center (=Month)
    if(setCenter){
      int8_t tmp = knobCenter.read();
      if(tmp != 0){
        mm += tmp;
        clampValue(&mm, (int8_t)1, (int8_t)12);
        //flush display
        numberToPort(&portCenterValue,mm);
        flushDisplay(false, true, false);
      }
    }

    //update right (=Year)
    if(setRight){
      int8_t tmp = knobRight.read();
      if(tmp != 0){
        yy += tmp;
        clampValue(&yy, (int8_t)0, (int8_t)99);
        //flush display
        numberToPort(&portRightValue,yy);
        flushDisplay(false, false, true);
      }
    }


    //user wants to set date?
    if(knobLeft.falling()){
      if(setLeft){
        setLeft = false;
        //save DAY set by the user
        if(isValidDate(dd,mm,yy)){
          rtc.adjust(DateTime( (uint16_t)2000 + (uint16_t)now.year2(), now.month(), dd, now.hour(), now.minute(), now.second()));
          numberToPort(&portLeftValue,dd);
          flushDisplay(true, false, false);
        }
        else{
          //restore date a known valid value
          numberToPort(&portLeftValue,now.day());
          flushDisplay(true, false, false);
        }
      }
      else{
        setLeft = true;
        knobLeft.read(); //flush stored value if any
      }
    }
    
    if(knobCenter.falling()){
      if(setCenter){
        setCenter = false;
        //save MONTH set by the user
        if(isValidDate(dd,mm,yy)){
          rtc.adjust(DateTime( (uint16_t)2000 + (uint16_t)now.year2(), mm, now.day(), now.hour(), now.minute(), now.second()));
          numberToPort(&portCenterValue,mm);
          flushDisplay(false, true, false);
        }
        else{
          //restore date a known valid value
          numberToPort(&portCenterValue,now.month());
          flushDisplay(false, true, false);
        }
      }
      else{
        setCenter = true;
        knobCenter.read(); //flush stored value if any
      }
    }

    if(knobRight.falling()){
      if(setRight){
        setRight = false;
        //save YEAR set by the user
        if(isValidDate(dd,mm,yy)){
          rtc.adjust(DateTime( (uint16_t)2000 + (uint16_t)yy, now.month(), now.day(), now.hour(), now.minute(), now.second()));
          numberToPort(&portRightValue,yy);
          flushDisplay(false, false, true);
        }
        else{
          //restore date a known valid value
          numberToPort(&portRightValue,now.year2());
          flushDisplay(false, false, true);
        }
      }
      else{
        setRight = true;
        knobRight.read(); //flush stored value if any
      }
    }

    //change state to display the time
    if(btnDateDisplay.falling()){  
      //user was currently setting the date and requested to display the time?
      //this is an edge case that shouldn't matter so we just discard what was
      //the user setting up. Sort of like a "cancel" button
      setLeft = setCenter = setRight = false;
      blinker = false;
      updateTime();
      flushDisplay(true, true, true); //force refresh of display
      changeState(STATE_DISPLAY_TIME);
    }

    
  }


  //The power switch does not belong to a specific state and is always polled
  btnPowerSwitch.poll();
  if(btnPowerSwitch.rising() || btnPowerSwitch.falling()){
    digitalWrite(PIN_POWER_SWITCH, btnPowerSwitch.getState());
  }

}


///Clamp a value between min and max, with proper looping
/// ex: if min=1, max=12, value=5 ==> output value = 5
/// ex: if min=1, max=12, value=13 ==> output value = 1
void clampValue(int8_t* value, int8_t min, int8_t max) {

  while (*value > max) {
    *value -=  (max-min+ (int8_t)1);
  }
  while (*value < min) {
    *value += (max - min + (int8_t)1);
  }
}


void updateDate(){updateDate(true);}
void updateDate(uint8_t flush){

  if(!setLeft){
    dd = now.day();
  }

  if(!setCenter){
    mm = now.month();
  }

  if(!setRight){
    yy = now.year2();
  }
  


  if(flush){
    numberToPort(&portLeftValue, dd);
    numberToPort(&portCenterValue, mm);
    numberToPort(&portRightValue, yy);
    flushDisplay(now.day() != previous.day(), now.month() != previous.month(), now.year2() != previous.year2());
  }



}



/// Convert a 2 digit base 10 number to individual 4 bits component
/// creating a single 8 bit number
/// ex: 12 => 0x12 (1)0001 (2)0010
//  ex: 53 => 0x53 (5)0101 (3)0011
void numberToPort(uint8_t* port, int8_t value){
  int8_t tH = value / 10;
  int8_t dH = value - tH*10;
  *port = (tH << 4) | dH;
}

void flushDisplay(){flushDisplay(true,true,true);}
void flushDisplay(uint8_t flushH, uint8_t flushM, uint8_t flushS){

    if(flushH){
      PORT_DISPLAY_LEFT = portLeftValue;
      #ifdef DEBUG
      Serial.print("H:");
      Serial.println(portLeftValue, HEX);
      #endif
    }
    if(flushM){
      PORT_DISPLAY_CENTER = portCenterValue;
      #ifdef DEBUG
      Serial.print("M:");
      Serial.println(portCenterValue, HEX);
      #endif
    }
    if(flushS){
      PORT_DISPLAY_RIGHT = portRightValue;
      #ifdef DEBUG
      Serial.print("S:");
      Serial.println(portRightValue, HEX);
      #endif
    }
}




void updateTime(){updateTime(true);}
void updateTime(uint8_t flush){

  if(!setLeft){
    h = now.hour();
  }
  if(!setCenter){
    m = now.minute();
  }
  if(!setRight){
    s = now.second();
  }
  

  if(flush){
    numberToPort(&portLeftValue, h);
    numberToPort(&portCenterValue, m);
    numberToPort(&portRightValue, s);
    flushDisplay(now.hour() != previous.hour(), now.minute() != previous.minute(), now.second() != previous.second());
  }

}





//Helper function for dates
// Returns true if given year is valid or not.
bool isValidDate(uint8_t d, uint8_t m, uint8_t y)
{
    if (m < 1 || m > 12)
      return false;
    if (d < 1 || d > 31)
      return false;
 
    // Handle February month with leap year
    if (m == 2)
    {
        if (isLeap(y))
           return (d <= 29);
        else
           return (d <= 28);
    }
 
    // Months of April, June, Sept and Nov
    // must have number of days less than
    // or equal to 30.
    if (m==4 || m==6 || m==9 || m==11)
        return (d <= 30);
 
    return true;
}
// Returns true if given year is valid.
bool isLeap(uint8_t y)
{

  //convert to 20xx
  int y2k = (int)2000 + (int)y;
    
  // Return true if year is a multiple pf 4 and
  // not multiple of 100.
  // OR year is multiple of 400. 
  return (((y2k%4==0) && (y2k%100!=0)) ||
           (y2k%400==0));
}



