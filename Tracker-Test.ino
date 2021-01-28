#include <stdlib.h>

struct Motor
{
  uint8_t CW; // switch pin assignment for H bridge clockwise
  uint8_t CCW; // counterclockwise
} motora;

// Signal switch registers ATTiny85 - CD74HC4067

#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define SIG0 4
#define ASIG0 A2

#define ROT_CW 0x00
#define ROT_CCW 0x01

uint8_t current_channel = 0;

uint8_t switch_channel(uint8_t channel)
{
  channel &= 0x0F;
  if (channel > 15) {
    return; // Exit if pin is out of scope
  }
  for (int pin = 0; pin < 4; pin++)
  {
    if (channel & (1 << pin))
    {
      switch (pin)
      {
        case 0:
          {
            digitalWrite(0, HIGH);
          }
        case 1:
          {
            digitalWrite(1, HIGH);
          }
        case 2:
          {
            digitalWrite(2, HIGH);
          }
        case 3:
          {
            digitalWrite(3, HIGH);
          }
      }
    }
    else
    {
      switch (pin)
      {
        case 0:
          {
            digitalWrite(0, LOW);
          }
        case 1:
          {
            digitalWrite(1, LOW);
          }
        case 2:
          {
            digitalWrite(2, LOW);
          }
        case 3:
          {
            digitalWrite(3, LOW);
          }
      }
    }
  }
  current_channel = channel;
  return channel;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  switch_channel(0);
  motora.CW = 14;
  motora.CCW = 15;
}

void loop() {
  // put your main code here, to run repeatedly:

  turn_motor(motora, ROT_CW, 1000);
  delay(500);
  turn_motor(motora, ROT_CCW, 1000);
  delay(500);

}

void turn_motor(struct Motor motor, uint8_t rotation, uint8_t duration) {
  pinMode(SIG0, OUTPUT);
  digitalWrite(SIG0, LOW);
  if (rotation == ROT_CW)
  {
    switch_channel(motor.CW);
  } else {
    switch_channel(motor.CCW);
  }
  digitalWrite(SIG0, HIGH);
  delay(duration);
  digitalWrite(SIG0, LOW);
}

uint8_t get_motor_channel(struct Motor motor, uint8_t rotation)
{
  if (rotation = ROT_CW)
  {
    return motor.CW;
  }
  return motor.CCW;
}
