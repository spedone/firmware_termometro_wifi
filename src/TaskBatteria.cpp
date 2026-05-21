#include "GlobalData.h"
#include "Tasks.h"

static task_status current_status;
static task_status battery_idle(task_status s);
static task_status battery_read(task_status s);
static task_status deep_sleep_mode(task_status s);
static double tensione;
static int timeout_reading;
static int read_counter;

void startTaskBatteria(){
  pinMode(A6, OUTPUT);
  pinMode(A7, INPUT);
  timeout_reading = millis();
  read_counter = 1;
  current_status = (task_status){.run = battery_idle};
  xTaskCreatePinnedToCore(taskBatteriaLoop, "Batteria", 8192, NULL, 3, NULL, 1);
}



void taskBatteriaLoop(void * pvParameters) {

  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

static task_status battery_idle(task_status s){
  
  if(millis() - timeout_reading > 15000 || read_counter > 0){
    read_counter = 0;
    tensione = 0;
    digitalWrite(A6, HIGH); // Chiude il partitore
    s = (task_status){.run = battery_read};
  } 
  return s;
}

static task_status battery_read(task_status s){
  
  if(read_counter < 7){
    tensione += (double) analogRead(A7) / 4095.0;
    read_counter++;
    return s;
  }else {
    ParametriConfigurazione p = getParametriConfigurazione();
    tensione = (tensione * 3.3 * p.k_divider) / 7;
    setTensioneBatteria(tensione);
    digitalWrite(A6, LOW); //Apre il partitore
    timeout_reading = millis();
    read_counter = 0;
    if(tensione <= 3.39)
      s = (task_status){.run = deep_sleep_mode};
    else
      s = (task_status){.run = battery_idle};
  }
  
  return s;
} 

static task_status deep_sleep_mode(task_status s){

  sendDeepSleep();
  vTaskDelay(pdMS_TO_TICKS(3000)); 
  esp_deep_sleep_start();
  return s;
} 

