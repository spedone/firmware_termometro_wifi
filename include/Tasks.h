/* TWIFI-ARDESP32 - Copyright 2026 Santino Pedone - see LICENSE for details */
#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

struct status_s;
typedef struct status_s (*status_fn)(struct status_s);
typedef struct status_s {
  status_fn run;
} task_status;

void startTaskBatteria();
void startTaskTemperatura();
void startTaskNetwork();
void startTaskWeb();
void startTaskDisplay();
void startTaskMqtt();
void startTaskOTA();

void taskBatteriaLoop(void * pvParameters);
void taskTemperaturaLoop(void * pvParameters);
void taskNetworkLoop(void * pvParameters);
void taskWebLoop(void * pvParameters);
void taskDisplayLoop(void * pvParameters);
void taskMqttLoop(void * pvParameters);
void taskOTALoop(void * pvParameters);


#endif