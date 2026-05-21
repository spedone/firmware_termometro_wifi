#include <SPI.h>
#include "GlobalData.h"
#include "Tasks.h"     
#include "Adafruit_MAX31865.h"

#define MAX_CS 8
#define MAX_SDI 9
#define MAX_SDO 10
#define MAX_CLK 11
#define RNOMINAL  100.0 // sonda pt-100 100 Ohm a 0 °C

static Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX_CS, MAX_SDI, MAX_SDO, MAX_CLK);

static task_status current_status;
static task_status temp_wait(task_status s);
static task_status temp_read(task_status s);

void startTaskTemperatura(){
    
  thermo.begin(MAX31865_3WIRE); 
  thermo.enable50Hz(true);
  pinMode(D7, INPUT);
  current_status = (task_status){.run = temp_read};
  xTaskCreatePinnedToCore(taskTemperaturaLoop, "Temp", 8192, NULL, 3, NULL, 1);
}

void taskTemperaturaLoop(void * pvParameters){
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  
}


static task_status temp_read(task_status s){
    
    ParametriConfigurazione p = getParametriConfigurazione();
    double tempC = thermo.temperature(RNOMINAL, p.r_ref);
    setTemperatura(tempC);
    
    if(thermo.readFault() == 0){
      setErroreTemperatura(false);
    } else { 
      setErroreTemperatura(true);
      thermo.clearFault();
    }
    
    return s;
}