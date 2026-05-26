#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h> 
#include "GlobalData.h"
#include "Tasks.h"
#include <ArduinoJson.h>

const char* HARDWARE_PASSWORD = "admin";
const char* SECRET_TOKEN = "esp32_secret_session_token_2026";

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static task_status current_status;
static task_status webserver_start(task_status s);
static task_status webserver_idle(task_status s);
static task_status webserver_stop(task_status s);
static bool isAuthenticated(AsyncWebServerRequest *request);

// --- Inizializzazione Task ---
void startTaskWeb() {
    SPIFFS.begin(true);
    current_status = (task_status){.run = webserver_start};
    xTaskCreatePinnedToCore(taskWebLoop, "Web", 8192, NULL, 1, NULL, 0);
}

void taskWebLoop(void * pvParameters) {
    for(;;){
        current_status = current_status.run(current_status);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// --- Controllo Autenticazione Bearer Token ---
static bool isAuthenticated(AsyncWebServerRequest *request) {
    if (!request->hasHeader("Authorization")) {
        return false;
    }
    const AsyncWebHeader* authHeader = request->getHeader("Authorization");
    String expectedValue = "Bearer " + String(SECRET_TOKEN);
    return authHeader->value() == expectedValue;
}

static task_status webserver_start(task_status s){
    StatoRete sr = getStatoRete();
    if(!sr.isWifiConnected and !sr.isStationMode) return s;

    // 1. File statici (Interfaccia Web)
    server.on("/", WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request){
        if(SPIFFS.exists("/index.html")){
            request->send(SPIFFS, "/index.html", "text/html"); 
        } else {
            request->send(404, "file not found");
        }
    });

    server.on("/vue.global.prod.js", WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request){
        if(SPIFFS.exists("/vue.global.prod.js")){
            request->send(SPIFFS, "/vue.global.prod.js", "application/javascript"); 
        } else {
            request->send(404, "file not found");
        }
    });

    // 2. Autenticazione / Login
    server.on("/api/login", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"JSON non valido\"}");
                return;
            }
            const char* inputPassword = doc["password"];
            if (inputPassword && strcmp(inputPassword, HARDWARE_PASSWORD) == 0) {
                String response;
                JsonDocument outputDoc;
                outputDoc["token"] = SECRET_TOKEN;
                serializeJson(outputDoc, response);
                request->send(200, "application/json", response);
            } else {
                request->send(401, "application/json", "{\"error\":\"Password errata\"}");
            }
        }
    );

    // 3. GET Impostazioni di Rete (Precompilazione Form)
    server.on("/api/settings/network", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!isAuthenticated(request)) {
            request->send(401, "application/json", "{\"error\":\"Non autorizzato\"}");
            return;
        }
        ParametriConfigurazione p = getParametriConfigurazione();
        JsonDocument doc;
        doc["ssid"] = p.wifi_ssid;
        doc["password"] = p.wifi_password;
        doc["mqttBroker"] = p.mqtt_hostname;
        doc["mqttPort"] = p.mqtt_port;
        doc["mqttEndpoint"] = p.mqtt_endpoint;
        doc["mqttUsername"] = p.mqtt_username;
        doc["mqttPassword"] = p.mqtt_password;
        

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 4. GET Calibrazione Sensori (Precompilazione Form)
    server.on("/api/settings/calibration", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!isAuthenticated(request)) {
            request->send(401, "application/json", "{\"error\":\"Non autorizzato\"}");
            return;
        }
        ParametriConfigurazione p = getParametriConfigurazione(); 
        JsonDocument doc;
        doc["k_divider"] = p.k_divider;
        doc["r_ref"] = p.r_ref; 

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 5. POST Salva Impostazioni di Rete (Aggiornato ad ArduinoJson v7)
    server.on("/api/settings/network", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (!isAuthenticated(request)) {
                request->send(401, "application/json", "{\"error\":\"Non autorizzato\"}");
                return;
            }
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"JSON non valido\"}");
                return;
            }

            ParametriConfigurazione p = getParametriConfigurazione(); 
            
            // Sostituito .containsKey con il controllo nativo v7 .is<T>()
            if (doc["ssid"].is<String>()) p.wifi_ssid = doc["ssid"].as<String>();
            if (doc["password"].is<String>()) p.wifi_password = doc["password"].as<String>();
            if (doc["mqttBroker"].is<String>()) p.mqtt_hostname = doc["mqttBroker"].as<String>();
            if (doc["mqttPort"].is<int>()) p.mqtt_port = doc["mqttPort"].as<int>();
            if (doc["mqttEndpoint"].is<String>()) p.mqtt_endpoint = doc["mqttEndpoint"].as<String>();
            if (doc["mqttUsername"].is<String>()) p.mqtt_username = doc["mqttUsername"].as<String>();
            if (doc["mqttPassword"].is<String>()) p.mqtt_password = doc["mqttPassword"].as<String>();

            saveParametriConfigurazione(p);
            loadParametriConfigurazione();
            sendResetNetwork();
            request->send(200, "application/json", "{\"status\":\"success\"}");
        }
    );

    // 6. POST Salva Calibrazione Sensori (Aggiornato ad ArduinoJson v7)
    server.on("/api/settings/calibration", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (!isAuthenticated(request)) {
                request->send(401, "application/json", "{\"error\":\"Non autorizzato\"}");
                return;
            }
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"JSON non valido\"}");
                return;
            }

            ParametriConfigurazione p = getParametriConfigurazione(); 
            
            // Sostituito .containsKey con il controllo nativo v7 .is<double>()
            if (doc["k_divider"].is<double>()) p.k_divider = doc["k_divider"].as<double>();
            if (doc["r_ref"].is<double>()) p.r_ref = doc["r_ref"].as<double>();

            saveParametriConfigurazione(p);
            loadParametriConfigurazione();
            request->send(200, "application/json", "{\"status\":\"success\"}");
        }
    );

    // Gestione Rotte Inesistenti
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "application/json", "{\"error\":\"Risorsa non trovata\"}");
    });

    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) Serial.println("WS Client Connected");
    });

    server.addHandler(&ws);
    server.begin();
    return (task_status){.run = webserver_idle};
}

static task_status webserver_idle(task_status s){
    if(checkResetWeb()){
        ws.closeAll(1001, "Server Reboot/Switch");
        ws.enable(false);
        server.removeHandler(&ws);
        server.end();
        return (task_status){.run = webserver_stop};
    }

    // Streaming dei dati in tempo reale alla sezione "Home" del frontend
    if (ws.count() > 0){
        StatoRete sr = getStatoRete();
        LettureSensori ls = getLettureSensori();
        
        // Sostituito StaticJsonDocument<300> con JsonDocument globale di v7
        JsonDocument doc; 
        doc["type"] = "telemetry";
        doc["temp"] = ls.temperatura;       
        doc["volt"] = ls.tensioneBatteria;  
        doc["batt_pct"] = ls.percentualeBatteria;
        doc["wifi"] = sr.isWifiConnected;
        doc["mqtt"] = sr.isMqttConnected;
        doc["sn"] = SERIAL_NR;
        doc["mac"] = WiFi.macAddress(); 

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        ws.textAll(jsonResponse);
    }
    return s;
}

static task_status webserver_stop(task_status s){
    if (ws.count() > 0){
        ws.closeAll(1001, "Server Reboot/Switch");
        ws.enable(false);
    } else {
        server.removeHandler(&ws);
        server.end();
        s = (task_status){.run = webserver_start};
        vTaskDelay(pdMS_TO_TICKS(50));
        catchResetWeb(); 
    }
    return s;
}