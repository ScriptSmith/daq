/*
This code is adapted from example code made available by AirGradient under the MIT License:
https://github.com/airgradienthq/arduino/blob/0a9f184946b09835eb327ca24f89c8db52a4aa66/examples/C02_PM_SHT_OLED_WIFI/C02_PM_SHT_OLED_WIFI.ino
*/

#include <AirGradient.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include "SSD1306Wire.h"

#include "config.h"

void showTextRectangle(const String& line1, const String& line2, boolean small);
void connectToWifi();
void connectToAWS();
void configNetworkClient();

String DEVICE_ID = String(EspClass::getChipId(),HEX);

AirGradient ag = AirGradient();

WiFiClientSecure networkClient = WiFiClientSecure();
MQTTClient mqttClient = MQTTClient(256);

X509List CA_CERT = X509List(AWS_CERT_CA);
X509List CLIENT_CERT  = X509List(AWS_CERT_DEVICE);
PrivateKey CLIENT_KEY = PrivateKey(AWS_CERT_PRIVATE);

SSD1306Wire display(0x3c, SDA, SCL);

boolean hasPM=true;
boolean hasCO2=true;
boolean hasSHT=true;

/**
 * Setup sensors and network connections
 */
void setup(){
    Serial.begin(9600);

    display.init();
    display.flipScreenVertically();
    showTextRectangle("Init", DEVICE_ID,true);

    if (hasPM) ag.PMS_Init();
    if (hasCO2) ag.CO2_Init();
    if (hasSHT) ag.TMP_RH_Init(0x44);

    if (ENABLE_WIFI) {
        connectToWifi();
        configNetworkClient();
        configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
        connectToAWS();
    }

    delay(2000);
}

void loop(){
    StaticJsonDocument<256> message;
    message["device_id"] = DEVICE_ID;
    message["room"] = ROOM;
    message["wifi"] = WiFi.RSSI();

    if (hasPM) {
        int PM2 = ag.getPM2_Raw();
        message["pm2"] = PM2;
        showTextRectangle("PM2",String(PM2),false);
        delay(3000);
    }

    if (hasCO2) {
        int CO2 = ag.getCO2_Raw();
        message["co2"] = CO2;
        showTextRectangle("CO2",String(CO2),false);
        delay(3000);
    }

    if (hasSHT) {
        TMP_RH result = ag.periodicFetchData();
        message["tmp"] = result.t;
        message["hmd"] = result.rh;
        showTextRectangle(String(result.t),String(result.rh)+"%",false);
        delay(3000);
    }

    char messageBuffer[512];
    serializeJson(message, messageBuffer);

    if (ENABLE_WIFI){
        mqttClient.loop();

        if (!mqttClient.connected()) {
            Serial.println("Server connection lost");
            connectToAWS();
        }

        Serial.println(messageBuffer);
        mqttClient.publish(AWS_TOPIC, messageBuffer);
    }
}

/**
 * Display text on OLED screen
 * @param line1 Top line
 * @param line2 Bottom line
 * @param small 24pt or 16pt font
 */
void showTextRectangle(const String& line1, const String& line2, boolean small) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if (small) {
        display.setFont(ArialMT_Plain_16);
    } else {
        display.setFont(ArialMT_Plain_24);
    }
    display.drawString(32, 16, line1);
    display.drawString(32, 36, line2);
    display.display();
}

/**
 * Connect to Wi-Fi network if already configured. Otherwise, setup configuration hotspot
 */
void connectToWifi(){
    WiFiManager wifiManager;
    // WiFi.disconnect(); // delete previous saved hotspot
    String HOTSPOT = "DAQ-"+String(EspClass::getChipId(),HEX);
    wifiManager.setTimeout(120);
    if(!wifiManager.autoConnect((const char*)HOTSPOT.c_str())) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        EspClass::restart();
        delay(5000);
    }

}

/**
 * Setup AWS certificates
 */
void configNetworkClient() {
    networkClient.setTrustAnchors(&CA_CERT);
    networkClient.setClientRSACert(&CLIENT_CERT, &CLIENT_KEY);
}

/**
 * Connect to AWS MQTT server
 */
void connectToAWS() {
    mqttClient.begin(AWS_IOT_ENDPOINT, 8883, networkClient);

    Serial.println("Connecting to server");

    while (!mqttClient.connect(THING_NAME)) {
        Serial.print("SSL Error: ");
        Serial.println(networkClient.getLastSSLError());
        delay(5000);
    }

    if(mqttClient.connected()){
        Serial.println("Connected!");
    } else {
        Serial.println("Timeout!");
    }
}
