#include "input.h"

const int minZ = 10;
const int maxZ = 99;
const int scaleZ1 = 105;
const int scaleZ2 = 1000;

Input::Input(int xpin, int ypin)
{
    x_pin = xpin;
    y_pin = ypin;

    _prevInputState.x = _prevInputState.y = 0;
    _prevInputState.b_pressed = false;
    _prevInputState.released = InputReleased::None;
    _prevInputState.ccAvailable = false;
    _prevInputState.cc = 0;
}

void Input::init()
{
    Serial.begin(115200);
}

InputState Input::get()
{
    int val_X = analogRead(x_pin) - 1665;// -950..950 or >1000 if btn pressed
    int val_Y = analogRead(y_pin) - 1700;// -950..950

    InputState state;
    state.released = InputReleased::None;
    
    state.ccAvailable = Serial.available();
    state.cc =  Serial.read();

    state.b_pressed = val_X > 1000;
    state.x = state.b_pressed ? 0 : map_pos(val_X)/10;
    state.y = map_pos(val_Y)/10;
    state.last_x = _prevInputState.x;
    state.last_y = _prevInputState.y;
    
    state.released |= (state.b_pressed == false && state.b_pressed != _prevInputState.b_pressed) ? InputReleased::Button : InputReleased::None;
    state.released |= (state.x == 0 && state.x != _prevInputState.x) ? ( state.x < _prevInputState.x ? InputReleased::RightStick : (state.x > _prevInputState.x ? InputReleased::LeftStick : InputReleased::None ) ) : InputReleased::None;
    state.released |= (state.y == 0 && state.y != _prevInputState.y) ? ( state.y < _prevInputState.y ? InputReleased::UpStick : (state.y > _prevInputState.y ? InputReleased::DownStick : InputReleased::None ) ) : InputReleased::None;
    
    _prevInputState.x = state.x;
    _prevInputState.y = state.y;
    _prevInputState.b_pressed = state.b_pressed;
    _prevInputState.cc = state.cc;

    return state;
}

int Input::map_pos(int v)
{
    v = v * scaleZ1 / scaleZ2;
    if (abs(v) < minZ) v = 0;
    else if (v > maxZ) v = maxZ;
    else if (v < -maxZ) v =-maxZ;
    return v;
}