#include <WiFi.h>
#include <PubSubClient.h>
#include "GlobalData.h"
#include "Tasks.h"     

static status current_status;

static WiFiClient esp32_client;
static PubSubClient client(esp32_client);
static unsigned long timeout_mqtt;
static int timout_lettura;

// Prototipi funzioni di stato
static status mqtt_start(status s);
static status mqtt_wait(status s);
static status mqtt_idle(status s);

void startTaskMqtt(){
  pinMode(LED_BLUE, OUTPUT);
  current_status = (status){.run = mqtt_start};
  xTaskCreatePinnedToCore(taskMqttLoop, "Mqtt", 8192, NULL, 1, NULL, 0);
}

void taskMqttLoop(void * pvParameters) {
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}


static status mqtt_start(status s) {
  
  setStatoMqtt(false);
  catchResetMqtt();

  if(!getStatoRete().isWifiConnected) return s;

  ParametriConfigurazione p = getParametriConfigurazione();
  if(p.mqtt_hostname != "") {
    client.setServer(p.mqtt_hostname.c_str(), p.mqtt_port);
    if(p.mqtt_username.length() > 0) {
      client.connect(MODEL_NAME "-" SERIAL_NR, p.mqtt_username.c_str(), p.mqtt_password.c_str());
    }
    else{ 
      client.connect(MODEL_NAME "-" SERIAL_NR);
    }
    s = (status){.run = mqtt_wait};
  }
  timeout_mqtt = millis();
  return s;
}

static status mqtt_wait(status s) {
  
  if(client.connected())
    s = (status){.run = mqtt_idle};
  
  if(millis() - timeout_mqtt > 10000) 
    s = (status){.run = mqtt_start};
  
  return s;
}

static status mqtt_idle(status s) {
  
  setStatoMqtt(true);

  if(!client.connected() || checkResetMqtt()) {
    client.disconnect(); 
    s = (status){.run = mqtt_start};
  }else{
    client.loop();
    digitalWrite(LED_BLUE, HIGH);

    if (millis() - timout_lettura > 5000) {
      ParametriConfigurazione p = getParametriConfigurazione();
      char message[100];
      LettureSensori l = getLettureSensori();
      sprintf(
        message, "{\"T\": %.1f, \"V_BAT\": %.2f, \"PC_BAT\": %.2f}", 
        l.temperatura, 
        l.tensioneBatteria, 
        l.percentualeBatteria
      );
      client.publish(p.mqtt_endpoint.c_str(),message);
      timout_lettura = millis();
      digitalWrite(LED_BLUE, LOW);
    } 
  }
 
  return s;
}
