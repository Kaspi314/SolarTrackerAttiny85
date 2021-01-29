#include <stdlib.h>
#include <util/delay.h>



struct Motor
{
  struct SolarPanel *PANEL;
  uint8_t *CW; // switch pin assignment for H bridge clockwise
  uint8_t *CCW; // counterclockwise
};

struct SolarSensor
{
  struct SolarPanel *PANEL;
  uint8_t *EAST; // switch pin assignment for adc input
  uint8_t *WEST;
};

struct SolarPanel
{
  struct Motor *MOTOR;
  struct SolarSensor *SOLARSENSOR;
  uint8_t *POSITION;
};

extern SolarPanel panels[];

/// INSTANTIATE
Motor motors[] =
{
  {&panels[0], 14, 15}
};

SolarSensor sensors[] =
{
  {&panels[0], 2, 3}
};

SolarPanel panels[] =
{
  {&motors[0], &sensors[0], NULL}
};

// Delay milliseconds
#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 8000000UL // 8MHz ATTiny85

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

/*
 *  This delay uses the avr builtin _delay_ms() which offers a resolution of 1/10th of a ms
 *  when the delay is above 262.14 ms / F_CPU in MHz. Up to 6.5535 seconds (at which point we have
 *  the overhead of another loop.)
 */
void delay_ms(uint8_t ms)
{
  if (ms > 6553.5) {
    while(ms >= 6553.5)
    {
      _delay_ms((double)6553.5);
      ms -= 6553.5;
    }
  }
  _delay_ms((double)ms);
}

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  switch_channel(0);
  motors[0].CW = 14;
  motors[0].CCW = 15;
}

void loop() {

  turn_motor(motors[0], ROT_CW, 1000);
  delay_ms(500);
  turn_motor(motors[0], ROT_CCW, 1000);
  delay_ms(500);

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
  delay_ms(duration);
  digitalWrite(SIG0, LOW);
}

uint8_t get_motor_channel(struct Motor motor, uint8_t rotation)
{
  if (rotation == ROT_CW)
  {
    return motor.CW;
  }
  return motor.CCW;
}
