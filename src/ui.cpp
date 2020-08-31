#include "ui.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

UserInterface::UserInterface() {
  lcd = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
}
UserInterface::~UserInterface() {
  delete lcd;
  lcd = NULL;
}

void UserInterface::init()
{
  if(!lcd->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  lcd->setTextSize(1);
  lcd->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  lcd->clearDisplay();
  lcd->display();
}

void UserInterface::draw(Model model, ControlSettings settings)
{
  lcd->fillScreen(SSD1306_BLACK);
  lcd->setCursor(0,0);

  if (model.mode == ModeType::MENU)
  {
    lcd->printf("%sSpeed:", model.menu.idx == 0 ? ">" : " ");
    if (settings.speed > 0) {
      lcd->printf("%d", settings.speed);lcd->println();
    }
    else {
      lcd->println("auto");
    }
    
    lcd->printf("%sAZM:%s", model.menu.idx == 1 ? ">" : " ", settings.dirAZM ? "+" : "-");lcd->println();
    lcd->printf("%sALT:%s", model.menu.idx == 2 ? ">" : " ", settings.dirALT ? "+" : "-");lcd->println();
    lcd->println();
    lcd->printf("X:%3d - Y:%3d", model.input.x, model.input.y);lcd->println();
  }
  else
  {
    lcd->printf("X:%3d", model.input.x);lcd->println();
    lcd->printf("Y:%3d", model.input.y);lcd->println();
    lcd->print("Speed:");
    if (settings.speed > 0) {
      lcd->printf("%d", settings.speed);lcd->println();
    }
    else {
      lcd->println("auto");
    }
    if (model.mountReady)
    {
      double azm = model.positionAZM * 360.0 / (double)0x1000000;
      int degAZM = int(azm);
      int minAZM = int((azm - degAZM) * 60);
      double secAZM = (azm - degAZM  - minAZM / 60.0) * 3600;
      double alt = model.positionALT * 360.0 / (double)0x1000000;
      int degALT = int(alt);
      int minALT = int((alt - degALT) * 60);
      double secALT = (alt - degALT  - minALT / 60.0) * 3600;
      lcd->printf("AZM:%3d %2d' %2.1f\"", degAZM, minAZM, secAZM);lcd->println();
      lcd->printf("ALT:%3d %2d' %2.1f\"", degALT, minALT, secALT);lcd->println();
    }
    else
    {
      lcd->println("Waiting for mount...");
    }
  }

  lcd->display();
}