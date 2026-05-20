#include <WiFi.h>
#include <PubSubClient.h>
#include "GlobalData.h"
#include "Tasks.h"     

static status current_status;

static WiFiClient esp32_client;
static PubSubClient client(esp32_client);
static unsigned long timeout_mqtt;
static unsigned long delay_ciclo = 200;

// Prototipi funzioni di stato
static status mqtt_start(status s);
static status mqtt_wait(status s);
static status mqtt_idle(status s);

void startTaskMqtt(){
  current_status = (status){.run = mqtt_start};
  xTaskCreatePinnedToCore(taskMqttLoop, "Mqtt", 8192, NULL, 1, NULL, 0);
}

void taskMqttLoop(void * pvParameters) {
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(delay_ciclo));
  }
}


static status mqtt_start(status s) {
  
  setStatoMqtt(false);
  if(!getStatoRete().isWifiConnected) return s;

  ParametriConfigurazione p = getParametriConfigurazione();
  if(p.mqtt_hostname != "") {
    client.setServer(p.mqtt_hostname.c_str(), p.mqtt_port);
    if(p.mqtt_username.length() > 0) {
      client.connect("ESP32_Termo", p.mqtt_username.c_str(), p.mqtt_password.c_str());
    }
    else{ 
      client.connect("ESP32_Termo");
    }
    s = (status){.run = mqtt_wait};
  }
  timeout_mqtt = millis();
  delay_ciclo = 200;
  return s;
}

static status mqtt_wait(status s) {
  
  if(client.connected())
    s = (status){.run = mqtt_idle};
  
  if(millis() - timeout_mqtt > 10000) 
    s = (status){.run = mqtt_start};
  
  delay_ciclo = 500;
  return s;
}

static status mqtt_idle(status s) {
  
  if(checkResetMqtt()) {
    client.disconnect(); 
  }
  if(client.connected()){
    setStatoMqtt(true);
    client.loop();
    ParametriConfigurazione p = getParametriConfigurazione();
    char message[100];
    LettureSensori l = getLettureSensori();
    sprintf(
      message, "{\"temperatura\": %.1f, \"tensioneBatteria\": %.2f, \"percentualeBatteria\": %.2f}", 
      l.temperatura, 
      l.tensioneBatteria, 
      l.percentualeBatteria
    );
    client.publish(p.mqtt_endpoint.c_str(),message);
  } else {
    catchResetWeb();
    s = (status){.run = mqtt_start};
  }

  delay_ciclo = 5000;
  return s;
}
