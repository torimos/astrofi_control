#pragma once

#include "nexstar_aux.h"

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

struct ControlSettings
{
    int speed;
    bool dirAZM;
    bool dirALT;
};

struct Model
{
    InputState input;
    ModeType mode;
    MenuModel menu;
    
    bool mountReady;
    uint32_t positionAZM;
    uint32_t positionALT;
};

struct MountTaskParams {
    NexStarAux* mount;
    Model* model;
};
