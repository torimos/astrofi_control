#include "input.h"

const int minZ = 10;
const int maxZ = 99;
const int scaleZ1 = 105;
const int scaleZ2 = 1000;

Input::Input(int xpin, int ypin)
{
    x_pin = xpin;
    y_pin = ypin;
}

void Input::init()
{
    Serial.begin(115200);
    _inputState.x=_inputState.y=_prevInputState.x=_prevInputState.y=0;
    _inputState.b_pressed=_prevInputState.b_pressed=false;
    _inputState.b_released=_prevInputState.b_released=false;
    _inputState.cc = 0;
    _inputState.ccAvailable = false;
}

InputState Input::get()
{
    int val_X = analogRead(x_pin) - 1665;// -950..950 or >1000 if btn pressed
    int val_Y = analogRead(y_pin) - 1700;// -950..950
    _inputState.b_pressed = val_X > 1000;
    _inputState.x = _inputState.b_pressed ? 0 : map_pos(val_X)/10;
    _inputState.y = map_pos(val_Y)/10;
    _inputState.b_released =  (!_inputState.b_pressed && _inputState.b_pressed != _prevInputState.b_pressed);
    _prevInputState.x = _inputState.x;
    _prevInputState.y = _inputState.y;
    _prevInputState.b_pressed = _inputState.b_pressed;
    _inputState.ccAvailable = Serial.available();
    _inputState.cc =  Serial.read();

    return _inputState;
}

int Input::map_pos(int v)
{
    v = v * scaleZ1 / scaleZ2;
    if (abs(v) < minZ) v = 0;
    else if (v > maxZ) v = maxZ;
    else if (v < -maxZ) v =-maxZ;
    return v;
}