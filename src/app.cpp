#include "app.h"

// SEL/RTS              - GRAY  / GRAY
// GND                  - RED   / BLACK
// SER1 (from DEVICE)   - YELOW / ORANGE
// 12V                  - GREEN / BLUE
// SER2 (from MAIN/INT) - BROWN / BROWN
// NC/CTS               - WHITE / WHITE

#define MENU_ITEM_COUNT     3
#define MENU_ITEM_SPEED     0
#define MENU_ITEM_INV_AZM   1
#define MENU_ITEM_INV_ALT   2


#define overlap(amt,low,high) ((amt)<(low)?(high):((amt)>(high)?(low):(amt)))

App::App(){
    mount = new NexStarAux(27, 4);
    ui = new UserInterface();
    input = new Input(39, 36);
}

void App::init()
{
    input->init();
    ui->init();
    mount->init();

    model.mode = ModeType::CONTROL;
    model.menu.idx = 0;
    
    model.speed = -1;
    model.invALT = model.invAZM = false;
}

void App::run()
{
    mount->run();
    model.input = input->get();

    if (model.input.released & InputReleased::Button)
    {
        model.mode = model.mode == ModeType::MENU ? ModeType::CONTROL : ModeType::MENU;
        model.menu.idx = 0;
    }
    switch (model.mode)
    {
    case ModeType::MENU:
        processMenu();
        break;
    case ModeType::CONTROL:
        processCtrl();
        break;
    }
    
    ui->draw(model);
}

void App::processMenu()
{ 
    if (model.input.released)
    {
        switch (model.menu.idx)
        {
        case MENU_ITEM_SPEED:
            {
                if (model.input.released == InputReleased::LeftStick) model.speed--;
                else if (model.input.released == InputReleased::RightStick) model.speed++;
                constrain(model.speed, 0, 9);
            }
            break;
        }
        if (model.input.released == InputReleased::UpStick) model.menu.idx--;
        else if (model.input.released == InputReleased::DownStick) model.menu.idx++;
        overlap(model.menu.idx, 0, MENU_ITEM_COUNT - 1);
    }
}

void App::processCtrl()
{
    if (model.input.released)
    {
        mount->move(DEV_AZ, true, 0);
        mount->move(DEV_ALT, true, 0);
    }
    else if (model.input.x != 0 || model.input.y != 0)
    {
        int ax = abs(model.input.x);
        int ay = abs(model.input.y);
        if (model.speed == 0 || ax > 0)
        {
            mount->move(DEV_AZ, model.input.x > 0, model.speed > 0 ? model.speed : ax);
        }
        if (model.speed == 0 || ay > 0)
        {
            mount->move(DEV_ALT, model.input.y < 0, model.speed > 0 ? model.speed : ay);
        }
    }
    delay(20);
}


// uint8_t   maxSpd = 0x09;
// uint8_t   stopSpd = 0x00;

// uint32_t  position[2];
// bool  dir[2];
// bool  moving[2];
// char ver[2];
// uint8_t   selAxis;// = DEV_ALT;

// bool menu = false;
// bool running = true;
// if (input.ccAvailable) {
//     switch (input.cc) {
//     case '1':
//     case '2':
//     case '3':
//     case '4':
//     case '5':
//     case '6':
//     case '7':
//     case '8':
//     case '9':
//     case '0':
//         maxSpd = input.cc - (unsigned char)'0';
//         Serial.printf("Rate set to %d", maxSpd);
//         Serial.println();
//         break;
//     case '[':
//         selAxis = DEV_ALT;
//         Serial.println("ALT axis selected");
//         break;
//     case ']':
//         selAxis = DEV_AZ;
//         Serial.println("AZM axis selected");
//         break;
//     case 'q':
//         Serial.printf("Stop Slew of %x Axis", selAxis);
//         Serial.println();
//         mount.move(selAxis, true, 0);
//         break;
//     case 'c':
//         Serial.printf("Slew %x Axis in negative direction", selAxis);
//         Serial.println();
//         mount.move(selAxis, false, maxSpd);
//         break;
//     case 'd':
//         Serial.printf("Slew %x Axis in positive direction", selAxis);
//         Serial.println();
//         mount.move(selAxis, true, maxSpd);
//         break;
//     case 'z':
//         position[selAxis & 1] = (position[selAxis & 1] - 0x100000) & 0xFFFFFFF;
//         Serial.printf("Rotate %x in negative direction by 0x100000 (1/16 of a revolution) to %x", selAxis, position[selAxis & 1]);
//         Serial.println();
//         mount.gotoPosition(selAxis, true, position[selAxis & 1]);
//         break;
//     case 'a':
//         position[selAxis & 1] = (position[selAxis & 1] + 0x100000) & 0xFFFFFFF;
//         Serial.printf("Rotate %x in positive direction by 0x100000 (1/16 of a revolution) to %x", selAxis, position[selAxis & 1]);
//         Serial.println();
//         mount.gotoPosition(selAxis, true, position[selAxis & 1]);
//         break;
//     case 's':
//         Serial.printf("Set %x Axis current position to 0x000000", selAxis);
//         Serial.println();
//         mount.setPosition(selAxis, 0x000000);
//         position[selAxis & 1] = 0x000000;
//         break;
//     case 'x':
//         Serial.printf("Move %x Axis to position 0x000000", selAxis);
//         Serial.println();
//         mount.gotoPosition(selAxis, false, 0x000000);
//         break;
//     case 'g':
//         Serial.printf("Axis %x Position = ", selAxis);
//         mount.getPosition(selAxis, &position[selAxis & 1]);
//         Serial.printf("Pos=%d", position[selAxis & 1]);
//         Serial.println();
//         break;
//     case 'v':
//         Serial.print("Firmware Version = ");
//         mount.getVersion(selAxis, &ver[0],&ver[1]);
//         Serial.printf("%d.%d", ver[0], ver[1]);  
//         Serial.println();
//         break;
//     case '?':
//         Serial.println("OK");
//         break;
//     }
//     // Delay before processing next command to avoid overrun
//     delay(20);
// }