#include "GlobalData.h"
#include "Tasks.h"

static status current_status;

static status battery_idle(status s);
static status deep_sleep_mode(status s);

void startTaskBatteria(){
  pinMode(A7, INPUT);
  current_status = (status){.run = battery_idle};
  xTaskCreatePinnedToCore(taskBatteriaLoop, "Batteria", 8192, NULL, 3, NULL, 1);
}



void taskBatteriaLoop(void * pvParameters) {

  for(;;) {

    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


static status battery_idle(status s){

  int rawValue = analogRead(A7);
  double tensione = rawValue * (3.3 / 4095.0) * 2.13;
  setTensioneBatteria(tensione);
  if(tensione <= 3.4) return (status){.run = deep_sleep_mode};
  return s;
} 

static status deep_sleep_mode(status s){

  deepSleep();
  vTaskDelay(pdMS_TO_TICKS(3000));
  esp_deep_sleep_start();
  return s;
} 

