#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

struct status_s;
typedef struct status_s (*status_fn)(struct status_s);
typedef struct status_s {
  status_fn run;
} status;

void startTaskBatteria();
void startTaskTemperatura();
void startTaskNetwork();
void startTaskWeb();
void startTaskDisplay();
void startTaskMqtt();

void taskBatteriaLoop(void * pvParameters);
void taskTemperaturaLoop(void * pvParameters);
void taskNetworkLoop(void * pvParameters);
void taskWebLoop(void * pvParameters);
void taskDisplayLoop(void * pvParameters);
void taskMqttLoop(void * pvParameters);


#endif