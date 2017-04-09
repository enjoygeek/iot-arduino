/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef Status_h
#define Status_h

#include "Arduino.h"

class Status
{
  public:
    Status(int pin);
    bool getStatus();
  private:
    int _pin;
};

#endif
