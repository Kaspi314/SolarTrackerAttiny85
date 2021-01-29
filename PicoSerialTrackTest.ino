#include "picoUART.h"
#include "pu_print.h"
#include "pu_config.h"
#include <stdlib.h>

// Delay milliseconds
#define F_CPU 8000000UL


struct Motor
{
  struct SolarPanel *PANEL;
  uint8_t *CW; // switch pin assignment for H bridge clockwise
  uint8_t *CCW; // counterclockwise
};

struct Sensor
{
  struct SolarPanel *PANEL;
  uint8_t *CW; // switch pin assignment for adc input
  uint8_t *CCW;
};

struct SolarPanel
{
  struct Motor *MOTOR;
  struct Sensor *SENSOR;
  uint8_t *POSITION;
};

extern SolarPanel panels[];

/// INSTANTIATE
Motor motors[] =
{
  {&panels[0], 14, 15}
};

Sensor sensors[] =
{
  {&panels[0], 2, 3}
};

SolarPanel panels[] =
{
  {&motors[0], &sensors[0], NULL}
};




// Signal switch registers ATTiny85 - CD74HC4067
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define SIG0 4

#define ROT_CW 0x00
#define ROT_CCW 0x01
uint8_t current_channel;

const int buflen = 20;
char linebuf[buflen];

uint8_t int_i;
uint8_t int_aa;
uint8_t int_ab;

char itoa_i[8];

int readline()
{
  int count = 0;
  char c;
  c = pu_read(); // waste
  do {
    while ( ! purx_dataready() );   // wait for data
    c = pu_read();
    linebuf[count++] = c;
  } while ( (c != '\n') && (count < buflen - 1) );
  linebuf[count] = '\0';
  return count;
}

void setup()
{
  // Setup pins for multiplexer CD74HC4067
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  // Switch multiplexer to TX output 0001b
  digitalWrite(S0, HIGH);
  prints("\nrxISR line echo\n");
  digitalWrite(S0, LOW); // back to rx
}

void loop()
{
  /*
  if ( purx_dataready() )
  {
    readline();
    // Switch multiplexer to TX output 0001b
    switch_channel(1);

    prints("got: ");
    prints(linebuf);

    switch_channel(0);
  }
  */
  // Switch multiplexer to analog in - 0010b
  switch_channel(2);
  delay(50);
  int_i = analogRead(A2);
  int_aa = int_i;


  // Switch to TX out from 0010b to 0001b
  switch_channel(1);
  itoa(int_i, itoa_i, 10);
  for (int_i = 0; int_i < 7; int_i++) {
    if (itoa_i[int_i]) {
      
      putx(itoa_i[int_i]);
      
    }
    itoa_i[int_i] = false;
  }
  putx(',');
  switch_channel(3);
  delay(50);
  int_i = analogRead(A2);
  int_ab = int_i;

  // Switch to TX out from 0010b to 0001b
  switch_channel(1);
  itoa(int_i, itoa_i, 10);
  for (int_i = 0; int_i < 7; int_i++) {
    if (itoa_i[int_i]) {
      
      putx(itoa_i[int_i]);
      
    }
    itoa_i[int_i] = false;
  }
  prints("\r\n");
  switch_channel(0); // back to rx - 0000b
  if(int_aa + 15 <= int_ab)
  {
    turn_motor(panels[0].MOTOR, ROT_CW, (double)1000);
    
    switch_channel(1);

    prints("mot: cw");
  } else if(int_aa >= int_ab + 15)
  {
    turn_motor(panels[0].MOTOR, ROT_CCW, (double)1000);
    
    switch_channel(1);

    prints("mot: ccw");
  } else {
    switch_channel(1);
    prints("mot: xx");
  }
  switch_channel(0); // back to rx - 0000b
  int_aa = 0;
  int_ab = 0;
}

/*
    This delay uses the avr builtin _delay_ms() which offers a resolution of 1/10th of a ms
    when the delay is above 262.14 ms / F_CPU in MHz. Up to 6.5535 seconds (at which point we have
    the overhead of another loop.)
*/
void delay_ms(double __ms)
{
  uint16_t __ticks;
  double __tmp ;
  __tmp = ((F_CPU) / 4e3) * __ms;
  if (__tmp < 1.0)
    __ticks = 1;
  else if (__tmp > 65535)
  {
    //  __ticks = requested delay in 1/10 ms
    __ticks = (uint16_t) (__ms * 10.0);
    while (__ticks)
    {
      // wait 1/10 ms
      _delay_loop_2(((F_CPU) / 4e3) / 10);
      __ticks --;
    }
    return;
  }
  else
    __ticks = (uint16_t)__tmp;
  _delay_loop_2(__ticks);
}

void delay_us(double __us)
{
  uint16_t __ticks;
  double __tmp ;
  __tmp = ((F_CPU) / 3e6) * __us;
  if (__tmp < 1.0)
    __ticks = 1;
  else if (__tmp > 255)
  {
    _delay_ms(__us / 1000.0);
    return;
  }
  else
    __ticks = (uint8_t)__tmp;
  _delay_loop_1(__ticks);
}

uint8_t switch_channel(uint8_t channel)
{
  channel &= 0x0F; // bitmask limit to 16 channels.
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

void read_sensor(Sensor *sensor, uint8_t dir) {
  pinMode(A2, INPUT);
  if ( dir == ROT_CW )
  {
    switch_channel(sensor->CW);
    int_aa = analogRead(A2);
    return;
  } else {
    switch_channel(sensor->CCW);
    int_ab = analogRead(A2);
    return;
  }
  return;
}

void turn_motor(Motor *motor, uint8_t rotation, double duration) {
  pinMode(SIG0, OUTPUT);
  digitalWrite(SIG0, LOW);
  if (rotation == ROT_CW)
  {
    switch_channel(motor->CW);
  } else {
    //switch_channel(motor->CCW);
    switch_channel(motor->CCW);
  }
  digitalWrite(SIG0, HIGH);
  delay_ms((double)duration);
  digitalWrite(SIG0, LOW);
  rotation = NULL;
}

uint8_t get_motor_channel(Motor *motor, uint8_t rotation)
{
  if (rotation == ROT_CW)
  {
    return motor->CW;
  }
  return motor->CCW;
}
