#include <WiFi.h>
#include "GlobalData.h"
#include "Tasks.h"     

static status current_status;
static unsigned long timeout_wifi;


// Prototipi funzioni di stato
static status wifi_start(status s);
static status wifi_wait(status s);
static status wifi_idle(status s);
static status ap_mode_start(status s);
static status ap_mode_idle(status s);
static status mqtt_start(status s);
static status mqtt_wait(status s);

void startTaskNetwork(){
  current_status = (status){.run = wifi_start};
  xTaskCreatePinnedToCore(taskNetworkLoop, "Network", 8192, NULL, 3, NULL, 0);
}

void taskNetworkLoop(void * pvParameters) {
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static status wifi_start(status s) {
  
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
  if(p.wifi_ssid == "") return (status){.run = ap_mode_start};
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  WiFi.begin(p.wifi_ssid.c_str(), p.wifi_password.c_str());
  timeout_wifi = millis();
  return (status){.run = wifi_wait};
}

static status wifi_wait(status s) {

  if(WiFi.status() == WL_CONNECTED) 
    s = (status){.run = wifi_idle};

  if(millis() - timeout_wifi > 10000)
    s = (status){.run = ap_mode_start};
  
  return s;
}

static status wifi_idle(status s) {

  setStatoWifi(WiFi.macAddress(), false, WiFi.localIP().toString(), true);
  if(WiFi.status() != WL_CONNECTED || catchResetNetwork()){
    sendResetWeb();
    sendResetMqtt();
    s = (status){.run = wifi_start};
  }

  return s;
}

static status ap_mode_start(status s) {
  setStatoWifi(WiFi.macAddress(), false, WiFi.softAPIP().toString(), false);
  int canaliPuliti[] = {1, 6, 11};
  int indiceRandom = esp_random() % 3;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  WiFi.softAP(MODEL_NAME "-" SERIAL_NR, "12345678", canaliPuliti[indiceRandom], 0, 4);
  return (status){.run = ap_mode_idle};
}

static status ap_mode_idle(status s){
  setStatoWifi(WiFi.macAddress(), true, WiFi.softAPIP().toString(), false);
  if(catchResetNetwork()){
    sendResetWeb();
    sendResetMqtt();
    s = (status){.run = wifi_start};
  } 
  return s;
}


