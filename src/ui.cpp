#include "ui.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

UserInterface::UserInterface() {
  lcd = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
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

void UserInterface::draw(Model model)
{
  lcd->fillScreen(SSD1306_BLACK);
  lcd->setCursor(0,0);

  if (model.mode == ModeType::MENU)
  {
    lcd->println("MENU:");
    lcd->printf("%s Speed: %d", ">", model.speed); 
    lcd->println();
  }
  else
  {
    lcd->printf("X: %4d", model.input.x);
    lcd->println();
    lcd->printf("Y: %4d", model.input.y); 
    lcd->println();
    if (model.speed>=0) {
      lcd->printf("Speed: %d", model.speed); 
      lcd->println();
    }
  }

  lcd->display();
}