#include "DisplayManager.h"
#include "WiFiManager.h"

WiFiManager wifiManager;
DisplayManager displayManager;

lv_obj_t* wifiStatusLabel;

void setup() {
    Serial.begin(115200); // Initialize serial communication for debugging

    // Initialize display manager
    displayManager.init();

    // WiFi status label
    wifiStatusLabel = lv_label_create(lv_scr_act());
    if (wifiStatusLabel) {
        lv_obj_set_style_text_color(wifiStatusLabel, lv_color_make(0xFF, 0xFF, 0x00), 0); // Yellow color
        lv_obj_set_style_text_font(wifiStatusLabel, &lv_font_montserrat_24, 0);
        lv_obj_align(wifiStatusLabel, LV_ALIGN_CENTER, 0, 50); // Adjust position as needed
    } else {
        Serial.println("Error: Failed to create wifiStatusLabel");
    }

    wifiManager.init();
    wifiManager.updateWiFiStatusLabel(wifiStatusLabel);

    // Set the data callback to pass the JSON data to the display manager
    wifiManager.setDataCallback([&](const String& jsonBuffer) {
        // Process the received JSON
        DynamicJsonDocument doc(8192);
        DeserializationError error = deserializeJson(doc, jsonBuffer);

        if (error) {
            displayManager.logMessage(LOG_LEVEL_ERROR, "deserializeJson() failed");
            displayManager.logMessage(LOG_LEVEL_ERROR, error.c_str());
        } else {
            // Log the parsed JSON for debugging
            displayManager.logMessage(LOG_LEVEL_DEBUG, "JSON parsed successfully");

            // Handle the incoming JSON data
            displayManager.handleIncomingData(jsonBuffer);
        }
    });

    // Create home screen
    displayManager.createHomeScreen(); // Ensure home screen is created on startup
}

void loop() {
    lv_task_handler(); // Handle LVGL tasks
    wifiManager.handleSerialData(); // Use the method from WiFiManager to handle serial data
}