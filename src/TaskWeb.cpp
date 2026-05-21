#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "GlobalData.h"
#include "Tasks.h"

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static task_status current_status;
static task_status webserver_start(task_status s);
static task_status webserver_idle(task_status s);
static task_status webserver_stop(task_status s);


// --- Implementazione della funzione di avvio definita in Tasks.h ---
void startTaskWeb() {

    current_status = (task_status){.run = webserver_start};
    xTaskCreatePinnedToCore(taskWebLoop, "Web", 8192, NULL, 1, NULL, 0);
}


void taskWebLoop(void * pvParameters) {
    for(;;){
        current_status = current_status.run(current_status);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


static const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: sans-serif; margin: 20px; background: #f4f4f4; }
        .card { background: white; padding: 20px; border-radius: 8px; margin-bottom: 20px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .param { font-size: 1.2em; font-weight: bold; color: #007bff; }
    </style>
</head>
<body>

    <div class="card">
        <h2>Configurazione termometro WiFi</h2>
        <p>MAC ADDR: %MAC_ADDRESS%</p>
        <form action="/config" method="POST">
            WiFi SSID: <input type="text" name="wifi_ssid" value="%WIFI_SSID%"><br><br>
            WiFi Password: <input type="password" name="wifi_password" value="%WIFI_PASSWORD%"><br><br>
            MQTT Hostname: <input type="text" name="mqtt_hostname" value="%MQTT_HOSTNAME%"><br><br>
            MQTT Port: <input type="number" name="mqtt_port" value="%MQTT_PORT%"><br><br>
            MQTT Username: <input type="text" name="mqtt_username" value="%MQTT_USERNAME%"><br><br>
            MQTT Password: <input type="password" name="mqtt_password" value="%MQTT_PASSWORD%"><br><br>
            MQTT Endpoint: <input type="text" name="mqtt_endpoint" value="%MQTT_ENDPOINT%"><br><br>
            K Divider: <input type="number" step="0.01" name="k_divider" value="%K_DIVIDER%"><br><br>
            R Ref: <input type="number" step="1" name="r_ref" value="%R_REF%"><br><br>
            <input type="submit" value="Save Configuration">
        </form>
    </div>

    <div class="card">
        <h2>Letture in tempo reale</h2>
        <p>Temperatura: <span id="temperatura" class="param">0</span> °C</p> 
        <p>Tensione batteria: <span id="tensioneBatteria" class="param">0</span> V</p>
        <p>Percentuale batteria: <span id="percentualeBatteria" class="param">0</span> %</p>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket = new WebSocket(gateway);

        websocket.onmessage = function(event) {
            // Expecting data format: "A:123,B:456,C:789"
            var parts = event.data.split(',');
            parts.forEach(part => {
                var pair = part.split(':');
                if(pair[0] == 'A') document.getElementById('temperatura').innerHTML = pair[1];
                if(pair[0] == 'B') document.getElementById('tensioneBatteria').innerHTML = pair[1];
                if(pair[0] == 'C') document.getElementById('percentualeBatteria').innerHTML = pair[1];
            });
        };
    </script>
</body>
</html>
)rawliteral";

static task_status webserver_start(task_status s){

    StatoRete sr = getStatoRete();
    if(!sr.isWifiConnected and !sr.isStationMode) return s;

    // ROUTE: Serve Main Page with dynamic placeholders
    server.on("/", WebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html,
            [](const String& var){
                ParametriConfigurazione p = getParametriConfigurazione();
                StatoRete sr = getStatoRete();
                if(var == "MAC_ADDRESS") return sr.mac_address;
                if(var == "WIFI_SSID") return p.wifi_ssid;
                if(var == "WIFI_PASSWORD") return p.wifi_password;
                if(var == "MQTT_HOSTNAME") return p.mqtt_hostname;
                if(var == "MQTT_PORT") return String(p.mqtt_port);
                if(var == "MQTT_USERNAME") return p.mqtt_username;
                if(var == "MQTT_PASSWORD") return p.mqtt_password;
                if(var == "MQTT_ENDPOINT") return p.mqtt_endpoint;
                if(var == "K_DIVIDER") return String(p.k_divider, 2);
                if(var == "R_REF") return String(p.r_ref);
            
                return String();
            }
        );
    });

    server.on("/config", WebRequestMethod::HTTP_POST, [](AsyncWebServerRequest *request) {

        ParametriConfigurazione p;
        StatoRete s = getStatoRete();
        
        if (request->hasParam("wifi_ssid", true)) {
            p.wifi_ssid = request->getParam("wifi_ssid", true)->value();
        }
        
        if (request->hasParam("wifi_password", true)) {
            p.wifi_password = request->getParam("wifi_password", true)->value();
        }
        
        if (request->hasParam("mqtt_hostname", true)) {
            p.mqtt_hostname = request->getParam("mqtt_hostname", true)->value();
        }
        
        if (request->hasParam("mqtt_port", true)) {
            p.mqtt_port = request->getParam("mqtt_port", true)->value().toInt();
        }
        
        if (request->hasParam("mqtt_username", true)) {
            p.mqtt_username = request->getParam("mqtt_username", true)->value();
        }
        
        if (request->hasParam("mqtt_password", true)) {
            p.mqtt_password = request->getParam("mqtt_password", true)->value();
        }
    
        if (request->hasParam("mqtt_endpoint", true)) {
            p.mqtt_endpoint = request->getParam("mqtt_endpoint", true)->value();
        }

        if (request->hasParam("k_divider", true)) {
            p.k_divider = request->getParam("k_divider", true)->value().toDouble();
        }

        if (request->hasParam("r_ref", true)) {
            p.r_ref = request->getParam("r_ref", true)->value().toInt();
        }

        saveParametriConfigurazione(p);
        loadParametriConfigurazione();
        sendResetNetwork();
        return request->redirect("/");
    });

    // HANDLER: WebSocket
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

    LettureSensori l = getLettureSensori();
    String payload = "A:" + String(l.temperatura, 1) + 
    ",B:" + String(l.tensioneBatteria, 2) + 
    ",C:" + String(l.percentualeBatteria, 0);
    ws.textAll(payload);
    
    return s;
}

static task_status webserver_stop(task_status s){
    
    //Attende che tutti i client si disconnettano
    if (ws.count() > 0){
        ws.closeAll(1001, "Server Reboot/Switch");
        ws.enable(false);
    } else {
        server.removeHandler(&ws);
        server.end();
        s = (task_status){.run = webserver_start};
        vTaskDelay(pdMS_TO_TICKS(50));
        catchResetWeb(); // Pulisce il segnale di reset web
    }
    return s;
}

