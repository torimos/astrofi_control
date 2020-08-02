#include <Arduino.h>
#include "nexstar_aux.h"

#define SEL_IN  22
#define SEL_OUT 23
// SEL/RTS              - GRAY  / GRAY
// GND                  - RED   / BLACK
// SER1 (from DEVICE)   - YELOW / ORANGE
// 12V                  - GREEN / BLUE
// SER2 (from MAIN/INT) - BROWN / BROWN
// NC/CTS               - WHITE / WHITE

NexStarAux mount(SEL_IN,SEL_OUT);

uint8_t   maxSpd = 0x09;
uint8_t   stopSpd = 0x00;

uint32_t  position[2];
bool  dir[2];
bool  moving[2];
char ver[2];
uint8_t   selAxis = DEV_ALT;

void setup() {
  mount.init();
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  mount.run();

  if (Serial.available()) {
    unsigned char cc = Serial.read();
    switch (cc) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
        maxSpd = cc - (unsigned char)'0';
        Serial.printf("Rate set to %d", maxSpd);
        Serial.println();
        break;
      case '[':
        selAxis = DEV_ALT;
        Serial.println("ALT axis selected");
        break;
      case ']':
        selAxis = DEV_AZ;
        Serial.println("AZM axis selected");
        break;
      case 'q':
        Serial.printf("Stop Slew of %x Axis", selAxis);
        Serial.println();
        mount.move(selAxis, true, 0);
        break;
      case 'c':
        Serial.printf("Slew %x Axis in negative direction", selAxis);
        Serial.println();
        mount.move(selAxis, false, maxSpd);
        break;
      case 'd':
        Serial.printf("Slew %x Axis in positive direction", selAxis);
        Serial.println();
        mount.move(selAxis, true, maxSpd);
        break;
      case 'z':
        position[selAxis & 1] = (position[selAxis & 1] - 0x100000) & 0xFFFFFFF;
        Serial.printf("Rotate %x in negative direction by 0x100000 (1/16 of a revolution) to %x", selAxis, position[selAxis & 1]);
        Serial.println();
        mount.gotoPosition(selAxis, true, position[selAxis & 1]);
        break;
      case 'a':
        position[selAxis & 1] = (position[selAxis & 1] + 0x100000) & 0xFFFFFFF;
        Serial.printf("Rotate %x in positive direction by 0x100000 (1/16 of a revolution) to %x", selAxis, position[selAxis & 1]);
        Serial.println();
        mount.gotoPosition(selAxis, true, position[selAxis & 1]);
        break;
      case 's':
        Serial.printf("Set %x Axis current position to 0x000000", selAxis);
        Serial.println();
        mount.setPosition(selAxis, 0x000000);
        position[selAxis & 1] = 0x000000;
        break;
      case 'x':
        Serial.printf("Move %x Axis to position 0x000000", selAxis);
        Serial.println();
        mount.gotoPosition(selAxis, false, 0x000000);
        break;
      case 'g':
        Serial.printf("Axis %x Position = ", selAxis);
        mount.getPosition(selAxis, &position[selAxis & 1]);
        Serial.printf("Pos=%d", position[selAxis & 1]);
        Serial.println();
        break;
      case 'v':
        Serial.print("Firmware Version = ");
        mount.getVersion(selAxis, &ver[0],&ver[1]);
        Serial.printf("%d.%d", ver[0], ver[1]);  
        Serial.println();
        break;
      case '?':
        Serial.println("OK");
        break;
    }
    // Delay before processing next command to avoid overrun
    delay(20);
  }
}