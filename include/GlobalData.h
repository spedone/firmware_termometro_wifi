#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <Arduino.h>

typedef struct s_letture_sensori {
    double temperatura;        
    double tensioneBatteria;  
    double percentualeBatteria;
    bool erroreTemperatura;
} LettureSensori;

typedef struct s_stato_rete {
    String mac_address;
    bool isStationMode;
    bool isWifiConnected;
    bool isMqttConnected;
    String ip_address;

} StatoRete;

typedef struct s_segnali {
    bool resetNetwork;           
    bool resetWeb;
    bool deepSleep;
} Segnali;

typedef struct s_parametri_configurazione {
    String wifi_ssid; 
    String wifi_password;
    String mqtt_hostname;
    String mqtt_endpoint;
    int mqtt_port;
    String mqtt_username;
    String mqtt_password;
} ParametriConfigurazione;

LettureSensori getLettureSensori();
StatoRete getStatoRete();
Segnali getSegnali();
ParametriConfigurazione getParametriConfigurazione();

void loadParametriConfigurazione();
void saveParametriConfigurazione(ParametriConfigurazione p);

void setTemperatura(double t);
void setTensioneBatteria(double t);
void setErroreTemperatura(bool r);

void setStatoWifi(String mac_address, bool isStationMode, String ip_address, bool isWifiConnected);
void setStatoMqtt(bool isMqttConnected);
void resetNetwork(bool r);
void resetWeb(bool r);
void deepSleep();



#endif