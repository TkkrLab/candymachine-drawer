/*
 * Candymachine drawer firmware
 * 
 * MIT License
 *  
 * Copyright (c) 2018 Renze Nicolai
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */


#include <Adafruit_NeoPixel.h>

#define NEOPIXELS_PIN 8
#define NEOPIXELS_AMOUNT 9

#define MOTORS_AMOUNT 3
//#define MOTORS_AMOUNT 6

#define MOTOR1_PIN 2
#define MOTOR2_PIN 3
#define MOTOR3_PIN 4
#define MOTOR4_PIN 5
#define MOTOR5_PIN 6
#define MOTOR6_PIN 7

#define SWITCH1_PIN A0
#define SWITCH2_PIN A1
#define SWITCH3_PIN A2
#define SWITCH4_PIN A3
#define SWITCH5_PIN A4
#define SWITCH6_PIN A5

#define STATE_IDLE 0
#define STATE_STARTING 1
#define STATE_WAIT1 2
#define STATE_VENDING 3
#define STATE_WAIT2 4
#define STATE_CORRECTION 5

const uint8_t motors[] = {MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN, MOTOR5_PIN, MOTOR6_PIN};
const uint8_t switches[] = {SWITCH1_PIN, SWITCH2_PIN, SWITCH3_PIN, SWITCH4_PIN, SWITCH5_PIN, SWITCH6_PIN};

unsigned long previousMillis = 0;
const unsigned long interval = 100; //Interval in milliseconds
uint8_t color = 0;
uint8_t channel = 0;

uint8_t correctioncounter[MOTORS_AMOUNT] = {0};


uint8_t state[MOTORS_AMOUNT] = { 0 };

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXELS_AMOUNT, NEOPIXELS_PIN, NEO_GRBW + NEO_KHZ800);

void moveChannel(uint8_t channel, bool mode = false) {
  digitalWrite(motors[channel], HIGH);
  while(digitalRead(switches[channel]) == mode);
  digitalWrite(motors[channel], LOW);
}

void setup() {
  for (uint8_t i = 0; i<MOTORS_AMOUNT; i++) {
    pinMode(switches[i], INPUT_PULLUP);
    pinMode(motors[i], OUTPUT);
  }
  
  Serial.begin(115200);
  strip.setBrightness(255);
  for (uint8_t i = 0; i<NEOPIXELS_AMOUNT; i++) {
    strip.setPixelColor(i, (uint32_t) 0x00FF0000);
  }
  
  strip.begin();
  
  strip.show();

  for (uint8_t i = 0; i<MOTORS_AMOUNT; i++) {
    moveChannel(i, true);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    for (uint8_t i = 0; i<NEOPIXELS_AMOUNT; i++) {

      uint32_t statuscolor = 0;
      
      int8_t unit = 0;
      #if (MOTORS_AMOUNT>5)
        unit = -1;
        if (i==0) unit = 0;
        if (i==2) unit = 1;
        if (i==3) unit = 2;
        if (i==5) unit = 3;
        if (i==6) unit = 4;
        if (i==8) unit = 5;
      #else
        if (i>2) unit = 1;
        if (i>5) unit = 2;
      #endif

      /*uint8_t led_in_unit = i - (unit*3);
      int8_t progress = -1;
      if (state[unit]==STATE_STARTING) progress = 1;
      if (state[unit]==STATE_WAIT1)    progress = 1;
      if (state[unit]==STATE_VENDING)  progress = 2;
      if (state[unit]==STATE_WAIT2)    progress = 3;

      if (progress>(2-led_in_unit)) statuscolor = 0x0000FF00;*/
      if (unit>=0) {
        if (state[unit]==STATE_STARTING) statuscolor = 0x00FF0000; //Red
        if (state[unit]==STATE_WAIT1)    statuscolor = 0x00FF0000; //Red
        if (state[unit]==STATE_VENDING)  statuscolor = 0x00FF0000; //Red
        if (state[unit]==STATE_WAIT2)    statuscolor = 0x0000FF00; //Green
      }
      strip.setPixelColor(i, (uint32_t) statuscolor);
    }
    color++;
    if (color>32) {
      color = 0;
      channel++;
      if (channel>5) channel = 0;
    }

    bool busy = false;
    for (uint8_t i = 0; i<MOTORS_AMOUNT; i++) {
      if (state[i]!=STATE_IDLE) busy = true;
    }

    if (!busy) {
      for (uint8_t i = 0; i<NEOPIXELS_AMOUNT; i++) {
        strip.setPixelColor(i, ((uint32_t) color << (8*channel)));
        //strip.setPixelColor(i, (uint32_t) 0xFFFFFFFF);
      }
      //Serial.print(color);
      //Serial.print("<< 8*");
      //Serial.print(channel);
      //Serial.print(" = ");
      //Serial.println(((uint32_t)color << (8*channel)), HEX);
    }
    
    strip.show();

    Serial.print(digitalRead(SWITCH1_PIN));
    Serial.print(", ");
    Serial.print(digitalRead(SWITCH2_PIN));
    Serial.print(", ");
    Serial.print(digitalRead(SWITCH3_PIN));
    Serial.print(", ");
    Serial.print(digitalRead(SWITCH4_PIN));
    Serial.print(", ");
    Serial.print(digitalRead(SWITCH5_PIN));
    Serial.print(", ");
    Serial.print(digitalRead(SWITCH6_PIN));
    Serial.print(", ");
    for (uint8_t i = 0; i<MOTORS_AMOUNT; i++) {
      Serial.print(state[i]);
      Serial.print(", ");
    }
    Serial.print(channel);
    Serial.print(", ");
    Serial.println(color);

    while (Serial.available()>0) {
      char c = Serial.read();
      switch(c) {
        case '1':
          if (state[0]==STATE_IDLE) state[0] = STATE_STARTING;
          break;
        case '2':
          if (state[1]==STATE_IDLE) state[1] = STATE_STARTING;
          break;
        case '3':
          if (state[2]==STATE_IDLE) state[2] = STATE_STARTING;
          break;
        #if (MOTORS_AMOUNT>5)
          case '4':
            if (state[3]==STATE_IDLE) state[3] = STATE_STARTING;
            break;
          case '5':
            if (state[4]==STATE_IDLE) state[4] = STATE_STARTING;
            break;
          case '6':
            if (state[5]==STATE_IDLE) state[5] = STATE_STARTING;
            break;
        #endif
        
        case 'q':
          if (state[0]==STATE_IDLE) state[0] = STATE_CORRECTION;
          break;
        case 'w':
          if (state[1]==STATE_IDLE) state[1] = STATE_CORRECTION;
          break;
        case 'e':
          if (state[2]==STATE_IDLE) state[2] = STATE_CORRECTION;
          break;

        #if (MOTORS_AMOUNT>5)
        case 'r':
          if (state[3]==STATE_IDLE) state[3] = STATE_CORRECTION;
          break;
        case 't':
          if (state[4]==STATE_IDLE) state[4] = STATE_CORRECTION;
          break;
        case 'y':
          if (state[5]==STATE_IDLE) state[5] = STATE_CORRECTION;
          break;
        #endif
      }
    }

    for (uint8_t i = 0; i<MOTORS_AMOUNT; i++) {
      switch(state[i]) {
        case STATE_STARTING:
          state[i] = STATE_WAIT1;
          digitalWrite(motors[i], HIGH); //Start motor
          Serial.println("Preparing to vend product "+String(i)+"...");
          break;
        case STATE_WAIT1:
          //Wait until button gets pressed
          if (!digitalRead(switches[i])) {
            state[i] = STATE_VENDING;
            Serial.println("Vending product "+String(i)+"...");
          }
          break;
        case STATE_VENDING:
          //Wait until button gets released
          if (digitalRead(switches[i])) {
            state[i] = STATE_WAIT2;
            Serial.println("Finishing vending process for product "+String(i)+"...");
          }
          break;
        case STATE_WAIT2:
          //Wait until button gets pressed
          if (!digitalRead(switches[i])) {
            state[i] = STATE_IDLE;
            digitalWrite(motors[i], LOW);
            Serial.println("Vending product "+String(i)+" completed.");
          }
          break;
        case STATE_CORRECTION:
          if (correctioncounter[i]>1) {
            state[i] = STATE_IDLE;
            correctioncounter[i] = 0;
            digitalWrite(motors[i], LOW);
            Serial.println("Corrected "+String(i)+".");
          } else {
            Serial.println("Correcting "+String(i)+": "+String(correctioncounter[i])+"....");
            digitalWrite(motors[i], HIGH);
          }
          correctioncounter[i]++;
      }
    }
  }
}
