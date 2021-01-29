#include <stdlib.h>

// Delay milliseconds
#define F_CPU 8000000UL


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

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  switch_channel(0);
}

void loop() {

  turn_motor(motors[0], ROT_CW, (double)1000);
  delay_ms((double)500);
  turn_motor(motors[0], ROT_CCW, (double)1000);
  delay_ms((double)500);

}

void turn_motor(struct Motor motor, uint8_t rotation, double duration) {
  pinMode(SIG0, OUTPUT);
  digitalWrite(SIG0, LOW);
  if (rotation == ROT_CW)
  {
    switch_channel(motor.CW);
  } else {
    switch_channel(motor.CCW);
  }
  digitalWrite(SIG0, HIGH);
  delay_ms((double)duration);
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



void swap(int *x, int *y) {
  if (x == y)
    return;
  *x ^= *y;
  *y ^= *x;
  *x ^= *y;
}

void cocktailsort(int *a, size_t n) {

  while (1) {
    // packing two similar loops into one
    char flag;
    size_t start[2] = {1, n - 1},
                      end[2] = {n, 0},
                               inc[2] = {1, -1};
    for (int it = 0; it < 2; ++it) {
      flag = 1;
      for (int i = start[it]; i != end[it]; i += inc[it])
        if (a[i - 1] > a[i]) {
          swap(a + i - 1, a + i);
          flag = 0;
        }
      if (flag)
        return;
    }
  }
}

double mean(uint8_t* values, uint8_t n)
{
  cocktailsort(*values, n);
  if (!(n % 2)) {
    return (double)values[n / 2];
  }
  return (double)((values[n - 1 / 2] + values[(n - 1 / 2) + 1]) / 2);
}

size_t sizeof_t(uint8_t* values)
{
  return sizeof(values) / sizeof(values[0]);
}

double avg(uint8_t* values, uint8_t n)
{
  uint8_t i;
  double s = 0;

  for ( i = 0; i < n; i++ )
    s += values[i];
  return s / n;
}


double stddev(uint8_t* values, uint8_t n)
{
  uint8_t i;
  double average = avg(values, n);
  double s = 0;

  for ( i = 0; i < n; i++ )
    s += (values[i] - average) * (values[i] - average);
  return sqrt(s / (n - 1));
}
