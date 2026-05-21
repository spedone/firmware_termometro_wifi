

#include "GlobalData.h"
#include "Tasks.h"

void setup() {

  loadParametriConfigurazione();
  Serial.begin(115200);
  setCpuFrequencyMhz(80); 

  startTaskNetwork();
  startTaskMqtt();
  startTaskWeb();
  startTaskOTA();
  startTaskTemperatura();
  startTaskBatteria();
  startTaskDisplay();

}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
