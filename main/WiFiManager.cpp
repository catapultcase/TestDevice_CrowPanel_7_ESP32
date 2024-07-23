#include "WiFiManager.h"
#include <ArduinoJson.h>
#include <lvgl.h>

const char* WiFiManager::ssid = "ssid";
const char* WiFiManager::password = "password";

WiFiManager::WiFiManager() 
    : server(80), jsonBuffer(""), payloadLength(0), readingLength(true), bytesRead(0), dataCallback(nullptr) {}

void WiFiManager::init() {
    connectToWiFi();

    // Initialize server
    server.on("/data", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Data received");
    }, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleIncomingDataChunk(data, len);
    });
    server.begin();
}

void WiFiManager::connectToWiFi() {
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi.");
    } else {
        Serial.println("Connected to WiFi.");
    }
}

void WiFiManager::updateWiFiStatusLabel(lv_obj_t* label) {
    if (label == nullptr) {
        Serial.println("Error: WiFi status label is null.");
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        String ipAddress = WiFi.localIP().toString();
        String statusText = "Status: Connected to WiFi (" + ipAddress + ")";
        Serial.println("Connected to WiFi.");
        lv_label_set_text(label, statusText.c_str());
    } else {
        lv_label_set_text(label, "Status: Unable to connect to WiFi");
    }
}

void WiFiManager::handleIncomingDataChunk(uint8_t *data, size_t len) {
    if (data == nullptr || len == 0) {
        Serial.println("Error: Incoming data is null or empty.");
        return;
    }

    if (readingLength) {
        // Check the first byte of the incoming data
        char firstByte = data[0];
        if (firstByte == '{') {
            // If the data starts with '{', it indicates that there's no prefix length
            Serial.println("Data without prefix length detected. Responding 'ok'.");
            jsonBuffer = ""; // Clear buffer
            readingLength = true; // Stay in reading length mode
            bytesRead = 0; // Reset bytes read
            return;
        }

        // If the first byte is a number, assume the data includes a length prefix
        if (isdigit(firstByte)) {
            jsonBuffer += String((char*)data).substring(0, len);
            bytesRead += len;

            if (jsonBuffer.length() >= 8) {
                payloadLength = jsonBuffer.substring(0, 8).toInt(); // Convert the length prefix to an integer
                Serial.print("Length Prefix Detected: ");
                Serial.println(payloadLength);
                jsonBuffer = jsonBuffer.substring(8); // Remove the length prefix from the buffer
                readingLength = false; // Switch to reading the payload
            }
        }
    } else {
        // Append the data chunk to the buffer
        jsonBuffer += String((char*)data).substring(0, len);
        bytesRead += len;

        if (bytesRead >= payloadLength) {
            Serial.println("WiFiManager Processed Inbound Data");
            if (dataCallback) {
                dataCallback(jsonBuffer);
            }

            // Process the JSON data
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonBuffer);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            } else {
                // Successfully processed JSON data
                Serial.println("JSON data processed successfully.");
            }

            // Clear buffer and reset states for next message
            jsonBuffer = "";
            readingLength = true;
            bytesRead = 0;
        }
    }
}

void WiFiManager::handleSerialData() {
    while (Serial.available() > 0) {
        char incomingByte = Serial.read();
        handleIncomingDataChunk((uint8_t*)&incomingByte, 1);
    }
}

void WiFiManager::setDataCallback(std::function<void(const String&)> callback) {
    dataCallback = callback;
}
