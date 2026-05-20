#include "GlobalData.h"
#include "Tasks.h"

static status current_status;
static status battery_trigger(status s);
static status battery_read(status s);
static status deep_sleep_mode(status s);

void startTaskBatteria(){
  pinMode(A6, OUTPUT);
  pinMode(A7, INPUT);
  current_status = (status){.run = battery_trigger};
  xTaskCreatePinnedToCore(taskBatteriaLoop, "Batteria", 8192, NULL, 3, NULL, 1);
}



void taskBatteriaLoop(void * pvParameters) {

  for(;;) {
    current_status = current_status.run(current_status);
  }
}

static status battery_trigger(status s){
  digitalWrite(A6, HIGH); // Chiude il partitore
  vTaskDelay(pdMS_TO_TICKS(1000)); // ttende 1 sec
  return (status){.run = battery_read};
}

static status battery_read(status s){
  ParametriConfigurazione p = getParametriConfigurazione();

  int rawValue = analogRead(A7);
  double tensione = (rawValue * 3.3 * p.k_divider) / 4095;
  setTensioneBatteria(tensione);
  vTaskDelay(pdMS_TO_TICKS(300));
  digitalWrite(A6, LOW); //Apre il partitore

  if(tensione <= 3.38) return (status){.run = deep_sleep_mode};
  vTaskDelay(pdMS_TO_TICKS(1700)); // Attende 2 secondi prima del prossimo trigger 
  return (status){.run = battery_trigger};
} 

static status deep_sleep_mode(status s){

  sendDeepSleep();
  vTaskDelay(pdMS_TO_TICKS(3000));
  esp_deep_sleep_start();
  return s;
} 

