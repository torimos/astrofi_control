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
  lcd->setTextSize(2);
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
    lcd->setTextSize(1);

    lcd->printf("%sSpeed:", model.menu.idx == 0 ? ">" : " ");
    if (settings.speed > 0) {
      lcd->printf("%d", settings.speed);lcd->println();
    }
    else {
      lcd->println("auto");
    }
    
    lcd->printf("%sInv AZM:%s", model.menu.idx == 1 ? ">" : " ", settings.invAZM ? "yes" : " no");lcd->println();
    lcd->printf("%sInv ALT:%s", model.menu.idx == 2 ? ">" : " ", settings.invALT ? "yes" : " no");lcd->println();
    lcd->println();
    lcd->printf("X:%3d - Y:%3d", model.input.x, model.input.y);lcd->println();
  }
  else
  {
    lcd->setTextSize(2);
    lcd->printf("X:%3d", model.input.x);lcd->println();
    lcd->printf("Y:%3d", model.input.y);lcd->println();
    lcd->print("Speed:");
    if (model.speed > 0) {
      lcd->printf("%d", model.speed);lcd->println();
    }
    else {
      lcd->println("auto");
    }
  }

  lcd->display();
}