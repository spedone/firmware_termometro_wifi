#include <Preferences.h>
#include "GlobalData.h"

Preferences pref;

static LettureSensori lettureSensori = (LettureSensori){
  .temperatura = 0.0,
  .t_ratio = 0,
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
    lett.t_ratio = lettureSensori.t_ratio;
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
    p.k_divider = parametriConfigurazione.k_divider;
    p.r_ref = parametriConfigurazione.r_ref;
    p.a1 = parametriConfigurazione.a1;
    p.a0 = parametriConfigurazione.a0;
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
    parametriConfigurazione.k_divider = pref.getDouble("k_divider", 2.13);
    parametriConfigurazione.r_ref = pref.getInt("r_ref", 430);
    parametriConfigurazione.a1 = pref.getDouble("a1", 0.0);
    parametriConfigurazione.a0 = pref.getDouble("a0", 0.0);
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
  pref.putDouble("k_divider", p.k_divider);
  pref.putInt("r_ref", p.r_ref);
  pref.putDouble("a1", p.a1);
  pref.putDouble("a0", p.a0);
  pref.end();
}


void setTemperatura(double t, unsigned long t_ratio){
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lettureSensori.temperatura = t;
    lettureSensori.t_ratio = t_ratio;
    xSemaphoreGive(xMutexLettureSensori);
  }
}

void setValoriBatteria(double t, double p){
  if (xSemaphoreTake(xMutexLettureSensori, pdMS_TO_TICKS(10)) == pdTRUE) {
    lettureSensori.tensioneBatteria = t;
    lettureSensori.percentualeBatteria = p;
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

void sendResetNetwork(){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.resetNetwork = true;
    xSemaphoreGive(xMutexSegnali);
  }
}
void sendResetWeb(){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.resetWeb = true;
    xSemaphoreGive(xMutexSegnali);
  }
}
void sendDeepSleep(){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.deepSleep = true;
    xSemaphoreGive(xMutexSegnali);
  }
}

void sendResetMqtt(){
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    segnali.resetMqtt = true;
    xSemaphoreGive(xMutexSegnali);
  }
}

bool catchResetNetwork(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.resetNetwork;
    segnali.resetNetwork = false;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}

bool catchResetWeb(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.resetWeb;
    segnali.resetWeb = false;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}

bool catchResetMqtt(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.resetMqtt;
    segnali.resetMqtt = false;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}

bool catchDeepSleep(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.deepSleep;
    segnali.deepSleep = false;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}

bool checkResetWeb(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.resetWeb;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}

bool checkResetMqtt(){
  bool r = false;
  if (xSemaphoreTake(xMutexSegnali, pdMS_TO_TICKS(10)) == pdTRUE) {
    r = segnali.resetMqtt;
    xSemaphoreGive(xMutexSegnali);
  }
  return r;
}