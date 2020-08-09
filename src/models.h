#pragma once

enum ModeType {
    MENU = 0,
    CONTROL = 1
};

enum InputReleased {
    None        = 0x0,
    Button      = 1UL << 1,
    UpStick     = 1UL << 2,
    DownStick   = 1UL << 3,
    LeftStick   = 1UL << 4,
    RightStick  = 1UL << 5,
};

struct InputState
{
    int x;
    int y;
    int last_x;
    int last_y;
    bool b_pressed;
    int released;
    bool ccAvailable;
    char cc;
};

struct MenuModel
{
    int idx;
};

struct Model
{
    InputState input;
    ModeType mode;
    MenuModel menu;
    int speed;
    bool invAZM;
    bool invALT;
};
