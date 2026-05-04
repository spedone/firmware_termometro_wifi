

#include "GlobalData.h"
#include "Tasks.h"

void setup() {

  loadParametriConfigurazione();
  Serial.begin(115200);

  startTaskNetwork();
  startTaskMqtt();
  startTaskWeb();

  startTaskTemperatura();
  startTaskBatteria();
  startTaskDisplay();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
