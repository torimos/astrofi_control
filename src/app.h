#pragma once
#include <Arduino.h>
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

public:
    App();
    void init();
    void run();
private:
    Model model;
    void process();
};