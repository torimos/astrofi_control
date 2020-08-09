#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "nexstar_aux.h"
#include "models.h"
#include "input.h"
#include "ui.h"

class App
{
private:
    Input* input;
    UserInterface* ui;
    NexStarAux* mount;
    Preferences* prefs;

public:
    App();
    ~App();
    void init();
    void run();
private:
    Model model;
    ControlSettings settings;
    void processMenu();
    void processCtrl();
    bool saveSettings();
    bool loadSettings();
};