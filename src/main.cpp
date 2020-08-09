#include <Arduino.h>
#include "app.h"

App app;

void setup() {
  app.init();
}

void loop() {
  app.run();
}