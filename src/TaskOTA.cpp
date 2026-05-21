#include <ArduinoOTA.h>
#include "GlobalData.h"
#include "Tasks.h"    

static task_status current_status;
static task_status ota_wait(task_status s);
static task_status ota_idle(task_status s);
static int timeoutOTA = 300;

void startTaskOTA(){
  
  current_status = (task_status){.run = ota_wait};
  xTaskCreatePinnedToCore(taskOTALoop, "OTA", 8192, NULL, 1, NULL, 0);
}

void taskOTALoop(void * pvParameters){

  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(timeoutOTA));
  }
  
}

static task_status ota_wait(task_status s){
    
    StatoRete sr = getStatoRete();

    if(sr.isWifiConnected || sr.isStationMode)
        s = (task_status){.run = ota_idle};

    ArduinoOTA.begin();
    
    return s;
}

static task_status ota_idle(task_status s){

    StatoRete sr = getStatoRete();

    if(sr.isWifiConnected || sr.isStationMode){
       ArduinoOTA.handle();
       timeoutOTA = 1;
    }else{
        ArduinoOTA.end();
        s = (task_status){.run = ota_wait};
        timeoutOTA = 300;
    }
    return s;
}

