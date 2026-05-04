#include <Preferences.h>
#include "GlobalData.h"

Preferences pref;

static LettureSensori lettureSensori = (LettureSensori){
  .tensioneBatteria = 0.0,
  .percentualeBatteria = 0.0,
  .erroreTemperatura = false,
};

static StatoRete statoRete = (StatoRete){
  
  .isStationMode = false,
  .isWifiConnected = false,
  .isMqttConnected = false,

};

static Segnali segnali = (Segnali){
  .resetNetwork = false,
  .resetWeb = false,
  .deepSleep = false,
};

static ParametriConfigurazione parametriConfigurazione;

SemaphoreHandle_t xMutexLettureSensori = xSemaphoreCreateMutex();
SemaphoreHandle_t xMutexStatoRete = xSemaphoreCreateMutex();
SemaphoreHandle_t xMutexSegnali = xSemaphoreCreateMutex();
SemaphoreHandle_t xMutexParametriConfigurazione = xSemaphoreCreateMutex();


LettureSensori getLettureSensori(){
  LettureSensori lett;
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lett.temperatura = lettureSensori.temperatura;
    lett.tensioneBatteria = lettureSensori.tensioneBatteria;
    lett.percentualeBatteria = lettureSensori.percentualeBatteria;
    lett.erroreTemperatura = lettureSensori.erroreTemperatura;
    xSemaphoreGive(xMutexLettureSensori);
  }
  return lett;
}

StatoRete getStatoRete(){
  StatoRete s;
  if (xSemaphoreTake(xMutexStatoRete, pdMS_TO_TICKS(10)) == pdTRUE) {
    s.isStationMode = statoRete.isStationMode;
    s.isWifiConnected = statoRete.isWifiConnected;
    s.isMqttConnected = statoRete.isMqttConnected;
    s.ip_address = statoRete.ip_address;
    s.mac_address = statoRete.mac_address;
    xSemaphoreGive(xMutexStatoRete);
  }
  return s;
}

Segnali getSegnali(){
  Segnali s;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    s.resetNetwork = segnali.resetNetwork;
    s.resetWeb = segnali.resetWeb;
    s.deepSleep = segnali.deepSleep;
    xSemaphoreGive(xMutexSegnali);
  }
  return s;
}

ParametriConfigurazione getParametriConfigurazione(){
  ParametriConfigurazione p;
  
  if (xSemaphoreTake(xMutexParametriConfigurazione, pdMS_TO_TICKS(10)) == pdTRUE) {
    p.wifi_ssid = parametriConfigurazione.wifi_ssid;
    p.wifi_password = parametriConfigurazione.wifi_password;
    p.mqtt_endpoint = parametriConfigurazione.mqtt_endpoint;
    p.mqtt_hostname = parametriConfigurazione.mqtt_hostname;
    p.mqtt_port = parametriConfigurazione.mqtt_port;
    p.mqtt_username = parametriConfigurazione.mqtt_username;
    p.mqtt_password = parametriConfigurazione.mqtt_password;
    xSemaphoreGive(xMutexParametriConfigurazione);
  }

  return p;
}

void loadParametriConfigurazione(){

  if (xSemaphoreTake(xMutexParametriConfigurazione, pdMS_TO_TICKS(10)) == pdTRUE) {
    pref.begin("config", true);
    parametriConfigurazione.wifi_ssid = pref.getString("wifi_ssid", "");
    parametriConfigurazione.wifi_password = pref.getString("wifi_password", "");
    parametriConfigurazione.mqtt_hostname = pref.getString("mqtt_hostname", "");
    parametriConfigurazione.mqtt_endpoint = pref.getString("mqtt_endpoint", "");
    parametriConfigurazione.mqtt_port = pref.getInt("mqtt_port", 1884);
    parametriConfigurazione.mqtt_username = pref.getString("mqtt_username", "");
    parametriConfigurazione.mqtt_password = pref.getString("mqtt_password", "");
    pref.end();
    xSemaphoreGive(xMutexParametriConfigurazione);
  }
  

}

void saveParametriConfigurazione(ParametriConfigurazione p){
  pref.begin("config", false);
  pref.putString("wifi_ssid", p.wifi_ssid);
  pref.putString("wifi_password", p.wifi_password);
  pref.putString("mqtt_hostname", p.mqtt_hostname);
  pref.putString("mqtt_endpoint", p.mqtt_endpoint);
  pref.putInt("mqtt_port", p.mqtt_port);
  pref.putString("mqtt_username", p.mqtt_username);
  pref.putString("mqtt_password", p.mqtt_password);
  pref.end();
}


void setTemperatura(double t){
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lettureSensori.temperatura = t;
    xSemaphoreGive(xMutexLettureSensori);
  }
}

void setTensioneBatteria(double t){
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lettureSensori.tensioneBatteria = t;
    lettureSensori.percentualeBatteria = ceil(constrain((t- 3.5) / (4.2 - 3.5) *100, 0 , 100));;
    xSemaphoreGive(xMutexLettureSensori);
  }
}
void setErroreTemperatura(bool r){
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lettureSensori.erroreTemperatura = r;
    xSemaphoreGive(xMutexLettureSensori);
  }
}

void setStatoWifi(String mac_address, bool isStationMode, String ip_address, bool isWifiConnected){
    
  if (xSemaphoreTake(xMutexStatoRete, pdMS_TO_TICKS(10)) == pdTRUE) {
    statoRete.mac_address = mac_address;
    statoRete.isStationMode = isStationMode;
    statoRete.ip_address = ip_address;
    statoRete.isWifiConnected = isWifiConnected;
    xSemaphoreGive(xMutexStatoRete);
  }

}

void setStatoMqtt(bool isMqttConnected){
  if (xSemaphoreTake(xMutexStatoRete, pdMS_TO_TICKS(10)) == pdTRUE) {
    statoRete.isMqttConnected = isMqttConnected;
    xSemaphoreGive(xMutexStatoRete);
  }
}

void resetNetwork(bool r){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.resetNetwork = r;
    xSemaphoreGive(xMutexSegnali);
  }
}


void resetWeb(bool r){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.resetWeb = r;
    xSemaphoreGive(xMutexSegnali);
  }
}

void deepSleep(){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.deepSleep = true;
    xSemaphoreGive(xMutexSegnali);
  }
}