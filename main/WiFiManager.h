#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <lvgl.h>
#include <functional>

class WiFiManager {
public:
    WiFiManager();
    void init();
    void updateWiFiStatusLabel(lv_obj_t* label);
    void handleIncomingDataChunk(uint8_t *data, size_t len);
    void handleSerialData();
    void setDataCallback(std::function<void(const String&)> callback); // Setter for data callback

private:
    static const char* ssid;
    static const char* password;
    AsyncWebServer server;
    String jsonBuffer;
    int payloadLength; // Length of the payload
    bool readingLength; // Flag to check if reading the length
    int bytesRead; // Bytes read so far
    std::function<void(const String&)> dataCallback; // Callback for handling data

    void connectToWiFi();
};

#endif // WIFI_MANAGER_H
