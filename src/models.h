#pragma once

enum ModeType {
    MENU = 0,
    CONTROL = 1
};

struct InputState
{
  int x;
  bool x_released;
  int y;
  bool y_released;
  bool b_pressed;
  bool b_released;
  bool ccAvailable;
  char cc;
};

struct Model
{
    InputState input;
    ModeType mode;
    int speed;
    bool running;
    int menu_hdir;
    int menu_vdir;
};
