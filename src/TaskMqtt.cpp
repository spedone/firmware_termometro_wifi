#include <WiFi.h>
#include <PubSubClient.h>
#include "GlobalData.h"
#include "Tasks.h"     

static task_status current_status;

static WiFiClient esp32_client;
static PubSubClient client(esp32_client);
static unsigned long timeout_mqtt;
static int timout_lettura;

// Prototipi funzioni di stato
static task_status mqtt_start(task_status s);
static task_status mqtt_wait(task_status s);
static task_status mqtt_idle(task_status s);

void startTaskMqtt(){
  pinMode(LED_BLUE, OUTPUT);
  current_status = (task_status){.run = mqtt_start};
  xTaskCreatePinnedToCore(taskMqttLoop, "Mqtt", 8192, NULL, 1, NULL, 0);
}

void taskMqttLoop(void * pvParameters) {
  for(;;) {
    current_status = current_status.run(current_status);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}


static task_status mqtt_start(task_status s) {
  
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
    s = (task_status){.run = mqtt_wait};
  }
  timeout_mqtt = millis();
  return s;
}

static task_status mqtt_wait(task_status s) {
  
  if(client.connected())
    s = (task_status){.run = mqtt_idle};
  
  if(millis() - timeout_mqtt > 10000) 
    s = (task_status){.run = mqtt_start};
  
  return s;
}

static task_status mqtt_idle(task_status s) {
  
  setStatoMqtt(true);

  if(!client.connected() || checkResetMqtt()) {
    client.disconnect(); 
    s = (task_status){.run = mqtt_start};
  }else{
    client.loop();
    digitalWrite(LED_BLUE, HIGH);

    if (millis() - timout_lettura > 5000) {
      ParametriConfigurazione p = getParametriConfigurazione();
      char message[100];
      LettureSensori l = getLettureSensori();
      sprintf(
        message, "{\"T_RATIO\": %lu, \"T\": %.1f, \"V_BAT\": %.2f, \"PC_BAT\": %.2f}", 
        l.t_ratio,
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
