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


#define overlap(amt,low,high) ((amt < low) ? high : (( amt > high) ? low : amt))
#define invert(cond, val) ((cond) ? (val) : !(val))

TaskHandle_t mountTask;

App::App(){
    mount = new NexStarAux(27, 18);
    ui = new UserInterface();
    input = new Input(39, 36);
    prefs = new Preferences();
}
App::~App() {
    //delete mount;
    delete ui;
    delete input;
    delete prefs;
    mount = NULL;
    ui = NULL;
    input = NULL;
    prefs = NULL;
}

MountTaskParams params;
void MountTask( void * pvParameters ){
    Serial.print("MountTask running on core ");
    Serial.println(xPortGetCoreID());
    MountTaskParams* params = (MountTaskParams*)pvParameters;
    delay(5000);
    params->mount->init();
    params->model->mountReady = true;
    while(1){
        params->mount->run();
        delay(1);
    }
}

void App::init()
{
    input->init();
    ui->init();

    model.mountReady = false;
    model.mode = ModeType::CONTROL;
    model.menu.idx = 0;
    if (!loadSettings())
    {
        settings.speed = 0;
        settings.dirALT = false;
        settings.dirAZM = false;
    }

    mount->setPosition(DEV_AZ, 0);
    mount->setPosition(DEV_ALT, 0);
    mount->move(DEV_AZ, true, 0);
    mount->move(DEV_ALT, true, 0);

    model.positionALT = model.positionAZM = 0;

    params.mount = mount;
    params.model = &model;
    xTaskCreatePinnedToCore(
                MountTask,   /* Task function. */
                "MountTask",     /* name of task. */
                10000,       /* Stack size of task */
                &params,        /* parameter of the task */
                1,           /* priority of the task */
                &mountTask,      /* Task handle to keep track of created task */
                0);          /* pin task to core 0 */   
                
    stopWatch = millis();
}
void App::run()
{
    model.input = input->get();

    if (model.input.released & InputReleased::Button)
    {
        if (model.mode == ModeType::MENU)
        {
            saveSettings();
        }
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
    
    ui->draw(model, settings);
}

void App::processMenu()
{
    if (model.input.x != 0)
    {
        switch (model.menu.idx)
        {
        case MENU_ITEM_SPEED:
            {
                if (model.input.x<0) settings.speed--;
                else if (model.input.x>0) settings.speed++;
                settings.speed = constrain(settings.speed, 0, 9);
            }
            break;
        case MENU_ITEM_INV_ALT:
               settings.dirALT = (model.input.x<0) ? false : ((model.input.x>0) ? true : settings.dirALT);
            break;
        case MENU_ITEM_INV_AZM:
               settings.dirAZM = (model.input.x<0) ? false : ((model.input.x>0) ? true : settings.dirAZM);
            break;
        }
        delay(200);
    } 
    else if (model.input.released)
    {
        if (model.input.released == InputReleased::UpStick) model.menu.idx--;
        else if (model.input.released == InputReleased::DownStick) model.menu.idx++;
        model.menu.idx = overlap(model.menu.idx, 0, MENU_ITEM_COUNT - 1);
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
        if (settings.speed == 0 || ax > 0)
        {
            mount->move(DEV_AZ, invert(settings.dirAZM, model.input.x > 0), settings.speed > 0 ? settings.speed : ax);
        }
        if (settings.speed == 0 || ay > 0)
        {
            mount->move(DEV_ALT, invert(settings.dirALT, model.input.y < 0), settings.speed > 0 ? settings.speed : ay);
        }
    }

    if (model.input.ccAvailable)
    {
        switch (model.input.cc) {
            case 'q':
                Serial.println("Stop Axis");
                mount->move(DEV_ALT, true, 0);
                delay(50);
                mount->move(DEV_AZ, true, 0);
                break;
            break;
            case '?':
                Serial.println("OK");
            break;
        }
    }

    if ((millis()-stopWatch) > 500)
    {
        mount->requestPosition(DEV_ALT);
        delay(25);
        mount->requestPosition(DEV_AZ);
        stopWatch=millis();
    }

    model.positionALT = mount->getPosition(DEV_ALT);
    model.positionAZM = mount->getPosition(DEV_AZ);

    delay(20);
}

bool App::saveSettings()
{
    if (prefs->begin("control"))
    {
        prefs->putBytes("settings", (uint8_t*)&settings, sizeof(ControlSettings));
        prefs->end();
        return true;
    }
    return false;
}

bool App::loadSettings()
{
    if (prefs->begin("control"))
    {
        size_t  schLen = prefs->getBytesLength("settings");
        prefs->getBytes("settings", (uint8_t*)&settings, schLen);
        prefs->end();
        return true;
    }
    return false;
}