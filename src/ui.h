#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include "models.h"

class UserInterface
{
private:
    Adafruit_SSD1306* lcd;
public:
    UserInterface();
    ~UserInterface();
    void init();
    void draw(Model model, ControlSettings settings);
};