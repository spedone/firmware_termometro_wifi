/* TWIFI-ARDESP32 - Copyright 2026 Santino Pedone - see LICENSE for details */
#include <WiFi.h>
#include "GlobalData.h"
#include "Tasks.h"     

static task_status current_status;
static unsigned long timeout_wifi;


// Prototipi funzioni di stato
static task_status wifi_start(task_status s);
static task_status wifi_wait(task_status s);
static task_status wifi_idle(task_status s);
static task_status ap_mode_start(task_status s);
static task_status ap_mode_idle(task_status s);
static task_status mqtt_start(task_status s);
static task_status mqtt_wait(task_status s);

void startTaskNetwork(){
  current_status = (task_status){.run = wifi_start};
  xTaskCreatePinnedToCore(taskNetworkLoop, "Network", 8192, NULL, 3, NULL, 0);
}

void taskNetworkLoop(void * pvParameters) {
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static task_status wifi_start(task_status s) {
  
  if(checkResetWeb() || checkResetMqtt()) return s;
 
  if(WiFi.getMode() == WIFI_STA){
    WiFi.disconnect(true);
    return s;
  }

  if(WiFi.softAPgetStationNum() > 0){
    WiFi.softAPdisconnect(true);
    return s;
  }

  if(catchResetNetwork()){
     WiFi.mode(WIFI_OFF);
     return s;
  }

  setStatoWifi(WiFi.macAddress(), false, WiFi.localIP().toString(), false);
  ParametriConfigurazione p = getParametriConfigurazione();
  if(p.wifi_ssid == "") return (task_status){.run = ap_mode_start};
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  WiFi.begin(p.wifi_ssid.c_str(), p.wifi_password.c_str());
  timeout_wifi = millis();
  return (task_status){.run = wifi_wait};
}

static task_status wifi_wait(task_status s) {

  if(WiFi.status() == WL_CONNECTED) 
    s = (task_status){.run = wifi_idle};

  if(millis() - timeout_wifi > 10000)
    s = (task_status){.run = ap_mode_start};
  
  return s;
}

static task_status wifi_idle(task_status s) {

  setStatoWifi(WiFi.macAddress(), false, WiFi.localIP().toString(), true);
  if(WiFi.status() != WL_CONNECTED || catchResetNetwork()){
    sendResetWeb();
    sendResetMqtt();
    s = (task_status){.run = wifi_start};
  }

  return s;
}

static task_status ap_mode_start(task_status s) {
  setStatoWifi(WiFi.macAddress(), false, WiFi.softAPIP().toString(), false);
  int canaliPuliti[] = {1, 6, 11};
  int indiceRandom = esp_random() % 3;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  WiFi.softAP(MODEL_NAME "-" SERIAL_NR, "12345678", canaliPuliti[indiceRandom], 0, 4);
  return (task_status){.run = ap_mode_idle};
}

static task_status ap_mode_idle(task_status s){
  setStatoWifi(WiFi.macAddress(), true, WiFi.softAPIP().toString(), false);
  if(catchResetNetwork()){
    sendResetWeb();
    sendResetMqtt();
    s = (task_status){.run = wifi_start};
  } 
  return s;
}


