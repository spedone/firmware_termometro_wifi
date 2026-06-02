/* TWIFI-ARDESP32 - Copyright 2026 Santino Pedone - see LICENSE for details */

#include <U8g2lib.h>
#include "GlobalData.h"
#include "Tasks.h"


U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

static task_status display_idle(task_status s);
static task_status display_deep_sleep(task_status s);
static task_status current_status;

void startTaskDisplay(){

    u8g2.begin();
    current_status = (task_status){.run = display_idle};
    xTaskCreatePinnedToCore(taskDisplayLoop, "Display", 8192, NULL, 1, NULL, 1);
}

void taskDisplayLoop(void * pvParameters){

    for(;;){
        current_status = current_status.run(current_status);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
   
}

static task_status display_idle(task_status s){

    if(catchDeepSleep()) return (task_status){.run = display_deep_sleep};
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_open_iconic_all_1x_t);

    StatoRete sr = getStatoRete();
    ParametriConfigurazione p = getParametriConfigurazione();
    if (sr.isStationMode) {
        u8g2.drawGlyph(0, 8, 0x54);
        u8g2.setFont(u8g2_font_helvR08_tr);
        u8g2.setCursor(15, 8);
        u8g2.print(sr.ip_address);
        
    } else  if(sr.isWifiConnected) {
        u8g2.drawGlyph(0, 8, 0xf8); 
        u8g2.setFont(u8g2_font_helvR08_tr);
        u8g2.setCursor(15, 8);
        u8g2.print(sr.ip_address);
    }

    if(sr.isMqttConnected){ 
        u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
        u8g2.drawGlyph(0, 64, 0x7c);
        u8g2.setFont(u8g2_font_helvR08_tr);
        u8g2.setCursor(15, 64);
        u8g2.print(p.mqtt_endpoint);
    } 

    char str_temperatura[10];
    char str_batteria[10];
    LettureSensori l = getLettureSensori();

    dtostrf(l.percentualeBatteria, 3, 0, str_batteria);
    strcat(str_batteria, "%");
    u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
    u8g2.drawGlyph(88, 8, 0x5A);
    u8g2.setFont(u8g2_font_helvR08_tr); 
    u8g2.drawStr(100, 8, str_batteria);

    if(l.erroreTemperatura){
        u8g2.setFont(u8g2_font_helvR24_tr);
        u8g2.drawStr(20, 46, "ERR");
    }else {
        dtostrf(l.temperatura, 4, 1, str_temperatura);
        u8g2.setFont(u8g2_font_helvR24_tr);
        u8g2.drawUTF8(25, 46, str_temperatura);
        u8g2.setFont(u8g2_font_helvR12_tf);
        u8g2.drawUTF8(103, 46, "°C");
    }
    u8g2.sendBuffer();
    return s;
}

static task_status display_deep_sleep(task_status s){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_battery24_tr);
    u8g2.drawGlyph(50, 45, 0x3d);
    u8g2.sendBuffer();
    return s;
}