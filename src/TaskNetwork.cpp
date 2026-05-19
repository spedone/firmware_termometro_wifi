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
static status mqtt_client_mode(status s);

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
  ParametriConfigurazione p = getParametriConfigurazione();
  if(p.wifi_ssid == "") return (status){.run = ap_mode_start};
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(p.wifi_ssid.c_str(), p.wifi_password.c_str());
  
  timeout_wifi = millis();
  setStatoWifi(WiFi.macAddress(), false, WiFi.localIP().toString(), false);
  return (status){.run = wifi_wait};
}

static status wifi_wait(status s) {

  if(WiFi.status() == WL_CONNECTED) {
    sendResetWeb();
    return (status){.run = wifi_idle};
  }

  if(millis() - timeout_wifi > 10000) {
   return (status){.run = ap_mode_start};
  }

  return s;
}

static status wifi_idle(status s) {

  setStatoWifi(WiFi.macAddress(), false, WiFi.localIP().toString(), true);
  if(WiFi.status() != WL_CONNECTED || catchResetNetwork()) 
    return (status){.run = wifi_start};
  return s;
}

static status ap_mode_start(status s) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(MODEL_NAME "-" SERIAL_NR, "12345678");
  setStatoWifi(WiFi.macAddress(), false, WiFi.softAPIP().toString(), false);
  sendResetWeb();
  return (status){.run = ap_mode_idle};
}

static status ap_mode_idle(status s){
  if(catchResetNetwork()) return (status){.run = wifi_start};
  setStatoWifi(WiFi.macAddress(), true, WiFi.softAPIP().toString(), false);
  return s;
}


