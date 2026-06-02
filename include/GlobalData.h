/* TWIFI-ARDESP32 - Copyright 2026 Santino Pedone - see LICENSE for details */
#ifndef GLOBALDATA_H
#define GLOBALDATA_H


#ifndef MODEL_NAME
    #define MODEL_NAME "TWIFI-ARDESP32" 
#endif

#ifndef SERIAL_NR
    #define SERIAL_NR "26000" 
#endif

#include <Arduino.h>

typedef struct s_letture_sensori {
    double temperatura;      
    unsigned long t_ratio;  
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
    bool resetMqtt;
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
    double k_divider;
    double a1;
    double a0;
    int r_ref;
} ParametriConfigurazione;

LettureSensori getLettureSensori();
StatoRete getStatoRete();
ParametriConfigurazione getParametriConfigurazione();

void loadParametriConfigurazione();
void saveParametriConfigurazione(ParametriConfigurazione p);

void setTemperatura(double t, unsigned long t_ratio);
void setValoriBatteria(double t, double p);
void setErroreTemperatura(bool r);

void setStatoWifi(String mac_address, bool isStationMode, String ip_address, bool isWifiConnected);
void setStatoMqtt(bool isMqttConnected);

void sendResetNetwork();
void sendResetWeb();
void sendResetMqtt();
void sendDeepSleep();

bool catchResetNetwork();
bool catchResetWeb();
bool catchResetMqtt();

bool checkResetWeb();
bool checkResetMqtt();

bool catchDeepSleep();


#endif