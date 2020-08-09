#pragma once

#include <Arduino.h>
#include "models.h"

class Input
{
public:
  Input(int xpin, int ypin);
  void init();
  InputState get();
private:
  int x_pin;
  int y_pin;
  InputState _prevInputState;
  
  int map_pos(int v);
};
