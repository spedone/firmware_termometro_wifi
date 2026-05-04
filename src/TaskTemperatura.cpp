#include <SPI.h>
#include "GlobalData.h"
#include "Tasks.h"     
#include "Adafruit_MAX31865.h"

#define MAX_CS 8
#define MAX_SDI 9
#define MAX_SDO 10
#define MAX_CLK 11
#define RREF      430.0  // resistenza ref MAX 31865
#define RNOMINAL  100.0 // sonda pt-100 100 Ohm a 0 °C

static Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX_CS, MAX_SDI, MAX_SDO, MAX_CLK);

void startTaskTemperatura(){
    
  thermo.begin(MAX31865_3WIRE); 
  thermo.enable50Hz(true);
  pinMode(D7, INPUT);

  xTaskCreatePinnedToCore(taskTemperaturaLoop, "Temp", 8192, NULL, 3, NULL, 1);
}

void taskTemperaturaLoop(void * pvParameters){

  for(;;) {

    double tempC = thermo.temperature(RNOMINAL, RREF);
    setTemperatura(tempC);
    if(thermo.readFault() == 0){
      setErroreTemperatura(false);
    } else { 
      setErroreTemperatura(true);
      thermo.clearFault();
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  
}