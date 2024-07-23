#include "DisplayManager.h"
#include <vector>

#define DEFAULT_LABEL_FONT_SIZE 18
#define DEFAULT_VALUE_FONT_SIZE 18
#define DEFAULT_CELL_PADDING 0

DisplayManager::DisplayManager() 
    : lcd(), 
      homeLabel(nullptr),
      CPUGridLabelFontSize(DEFAULT_LABEL_FONT_SIZE),
      CPUGridValueFontSize(DEFAULT_VALUE_FONT_SIZE),
      OtherGridLabelFontSize(DEFAULT_LABEL_FONT_SIZE),
      OtherGridValueFontSize(DEFAULT_VALUE_FONT_SIZE),
      CPUGridCellPadding(DEFAULT_CELL_PADDING),
      OtherGridCellPadding(DEFAULT_CELL_PADDING),
      CPUGridRows(3),
      CPUGridCols(4),
      OtherGridRows(3),
      OtherGridCols(3),
      currentLogLevel(LOG_LEVEL_INFO),  // Default log level
      screenCreated(false),
      textColor(lv_color_white()) { // Default text color
}

void DisplayManager::init() {
    lcd.begin();
    lcd.setColorDepth(16);
    lcd.setRotation(2); // Adjust the rotation as needed (0, 1, 2, 3)

    // Initialize LVGL
    lv_init();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[800 * 10]; // Updated to match the width of the screen
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 800 * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = this; // Pass the instance
    lv_disp_drv_register(&disp_drv);

    createHomeScreen();
}

void DisplayManager::my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    DisplayManager* instance = (DisplayManager*)disp->user_data;
    if (instance != nullptr) {
        instance->lcd.startWrite();
        instance->lcd.setAddrWindow(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
        instance->lcd.pushColors(&color_p->full, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1), true);
        instance->lcd.endWrite();
    }
    lv_disp_flush_ready(disp);
}

void DisplayManager::createHomeScreen() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT); // Set background to black

    homeLabel = lv_label_create(scr);
    if (homeLabel != nullptr) {
        lv_label_set_text(homeLabel, "Junction Relay");
        lv_obj_set_style_text_color(homeLabel, lv_color_white(), 0);
        lv_obj_set_style_text_font(homeLabel, &lv_font_montserrat_48, 0);
        lv_obj_align(homeLabel, LV_ALIGN_CENTER, 0, 0);
    } else {
        logMessage(LOG_LEVEL_ERROR, "Failed to create homeLabel");
    }
}

void DisplayManager::logMessage(LogLevel level, const char* message) {
    if (level <= currentLogLevel) {
        Serial.println(message);
    }
}

void DisplayManager::setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void DisplayManager::handleIncomingData(const String& json) {
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        logMessage(LOG_LEVEL_ERROR, "deserializeJson() failed");
        logMessage(LOG_LEVEL_ERROR, error.c_str());
        return;
    }

    if (doc["metadata"]["CustomMetadata"].containsKey("DebugLevel")) {
        int debugLevel = doc["metadata"]["CustomMetadata"]["DebugLevel"].as<int>();
        setLogLevel(static_cast<LogLevel>(debugLevel));
    }

    String layout = doc["metadata"]["CustomMetadata"]["Layout"].as<String>();
    logMessage(LOG_LEVEL_INFO, ("Layout: " + layout).c_str());

    // Initialize variables with values
    int cpuGridLabelFontSize = CPUGridLabelFontSize;
    int cpuGridValueFontSize = CPUGridValueFontSize;
    int otherGridLabelFontSize = OtherGridLabelFontSize;
    int otherGridValueFontSize = OtherGridValueFontSize;
    int cpuGridCellPadding = CPUGridCellPadding;
    int otherGridCellPadding = OtherGridCellPadding;
    int cpuGridRows = CPUGridRows;
    int cpuGridCols = CPUGridCols;
    int otherGridRows = OtherGridRows;
    int otherGridCols = OtherGridCols;

    // Update variables only if they are present in the JSON
    if (doc["metadata"]["CustomMetadata"].containsKey("CPUGridLabelFontSize")) {
        cpuGridLabelFontSize = doc["metadata"]["CustomMetadata"]["CPUGridLabelFontSize"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("CPUGridValueFontSize")) {
        cpuGridValueFontSize = doc["metadata"]["CustomMetadata"]["CPUGridValueFontSize"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("OtherGridLabelFontSize")) {
        otherGridLabelFontSize = doc["metadata"]["CustomMetadata"]["OtherGridLabelFontSize"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("OtherGridValueFontSize")) {
        otherGridValueFontSize = doc["metadata"]["CustomMetadata"]["OtherGridValueFontSize"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("CPUGridCellPadding")) {
        cpuGridCellPadding = doc["metadata"]["CustomMetadata"]["CPUGridCellPadding"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("OtherGridCellPadding")) {
        otherGridCellPadding = doc["metadata"]["CustomMetadata"]["OtherGridCellPadding"].as<int>();
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("CPUGridRows")) {
        cpuGridRows = doc["metadata"]["CustomMetadata"]["CPUGridRows"].as<int>();
        if (cpuGridRows == 0) {
            cpuGridRows = 1; // Avoid division by zero
        }
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("CPUGridCols")) {
        cpuGridCols = doc["metadata"]["CustomMetadata"]["CPUGridCols"].as<int>();
        if (cpuGridCols == 0) {
            cpuGridCols = 1; // Avoid division by zero
        }
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("OtherGridRows")) {
        otherGridRows = doc["metadata"]["CustomMetadata"]["OtherGridRows"].as<int>();
        if (otherGridRows == 0) {
            otherGridRows = 1; // Avoid division by zero
        }
    }
    if (doc["metadata"]["CustomMetadata"].containsKey("OtherGridCols")) {
        otherGridCols = doc["metadata"]["CustomMetadata"]["OtherGridCols"].as<int>();
        if (otherGridCols == 0) {
            otherGridCols = 1; // Avoid division by zero
        }
    }

    // Parse text color
    if (doc["metadata"]["CustomMetadata"].containsKey("TextColor")) {
        String textColorStr = doc["metadata"]["CustomMetadata"]["TextColor"].as<String>();
        if (textColorStr.startsWith("#")) {
            textColorStr.remove(0, 1); // Remove the '#' character if present
        }
        uint32_t colorValue = (uint32_t)strtol(textColorStr.c_str(), NULL, 16);
        textColor = lv_color_hex(colorValue);
    } else {
        textColor = lv_color_white(); // Default color
    }

    // Check if metadata has changed
    bool metadataChanged = cpuGridLabelFontSize != CPUGridLabelFontSize ||
                           cpuGridValueFontSize != CPUGridValueFontSize ||
                           otherGridLabelFontSize != OtherGridLabelFontSize ||
                           otherGridValueFontSize != OtherGridValueFontSize ||
                           cpuGridCellPadding != CPUGridCellPadding ||
                           otherGridCellPadding != OtherGridCellPadding ||
                           cpuGridRows != CPUGridRows ||
                           cpuGridCols != CPUGridCols ||
                           otherGridRows != OtherGridRows ||
                           otherGridCols != OtherGridCols;

    // Clear previous sensor data
    sensorCollection.clear();
    cpuCollection.clear();
    otherCollection.clear();

    // Parse sensor data
    for (JsonPair kv : doc["sensors"].as<JsonObject>()) {
        String sensorTag = kv.key().c_str();
        JsonArray sensorDataArray = kv.value().as<JsonArray>();
        String unit = sensorDataArray[0]["Unit"].as<String>();
        String value = sensorDataArray[0]["Value"].as<String>();
        int sensorOrder = sensorDataArray[0]["SensorOrder"].as<int>();
        String category = sensorDataArray[0]["Category"].as<String>();
        String componentName = sensorDataArray[0]["ComponentName"].as<String>();

        SensorData sensorData = {sensorTag, value + " " + unit, sensorOrder, category, componentName};

        if (layout == "DataGrid") {
            sensorCollection.push_back(sensorData);
        } else if (layout == "CPUDash" || layout == "CPUDials") {
            if (category == "Load" && componentName == "CPU") {
                cpuCollection.push_back(sensorData);
            } else {
                otherCollection.push_back(sensorData);
            }
        }
    }

    auto compare = [](const SensorData& a, const SensorData& b) {
        return a.order < b.order;
    };

    if (layout == "DataGrid") {
        logMessage(LOG_LEVEL_INFO, "Creating DataGrid Layout");
        if (!screenCreated || metadataChanged) {
            createDataGridScreen();
        }
        updateDataGridScreen();
    } else if (layout == "CPUDash") {
        std::sort(cpuCollection.begin(), cpuCollection.end(), compare);
        std::sort(otherCollection.begin(), otherCollection.end(), compare);
        logMessage(LOG_LEVEL_INFO, "Creating CPUDash Layout");
        if (!screenCreated || metadataChanged) {
            createCPUDashScreen();
        }
        updateCPUDashScreen();
    } else if (layout == "CPUDials") {
        std::sort(cpuCollection.begin(), cpuCollection.end(), compare);
        std::sort(otherCollection.begin(), otherCollection.end(), compare);
        logMessage(LOG_LEVEL_INFO, "Creating CPUDials Layout");
        if (!screenCreated || metadataChanged) {
            createCPUDialsScreen();
        }
        updateCPUDialsScreen();
    }

    // Update previous metadata values
    CPUGridLabelFontSize = cpuGridLabelFontSize;
    CPUGridValueFontSize = cpuGridValueFontSize;
    OtherGridLabelFontSize = otherGridLabelFontSize;
    OtherGridValueFontSize = otherGridValueFontSize;
    CPUGridCellPadding = cpuGridCellPadding;
    OtherGridCellPadding = otherGridCellPadding;
    CPUGridRows = cpuGridRows;
    CPUGridCols = cpuGridCols;
    OtherGridRows = otherGridRows;
    OtherGridCols = otherGridCols;
}


void DisplayManager::createDataGridScreen() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr); // Clear previous screen

    grid = lv_obj_create(scr);
    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(grid, OtherGridCellPadding, 0);
    lv_obj_set_style_bg_color(grid, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(grid, 0, 0);

    screenCreated = true;
}

void DisplayManager::updateDataGridScreen() {
    if (!screenCreated) {
        createDataGridScreen();
    }

    updateOtherGridLayout(grid, sensorCollection, OtherGridRows, OtherGridCols, OtherGridLabelFontSize, OtherGridValueFontSize);
}

void DisplayManager::createCPUDashScreen() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr); // Clear previous screen

    // Left half for CPU sensors
    lv_obj_t *leftHalf = lv_obj_create(scr);
    lv_obj_set_size(leftHalf, lv_pct(50), lv_pct(100));
    lv_obj_align(leftHalf, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(leftHalf, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(leftHalf, CPUGridCellPadding, 0);
    lv_obj_set_style_border_width(leftHalf, 0, 0); // No border for the container

    // Add padding to the leftHalf container
    lv_obj_set_style_pad_top(leftHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_bottom(leftHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_left(leftHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_right(leftHalf, 10, 0); // Adjust the value as needed

    // Bar chart for CPU usage
    barChart = lv_chart_create(leftHalf);
    lv_obj_set_size(barChart, lv_pct(100), lv_pct(48)); // Adjust height to leave space for padding
    lv_obj_align(barChart, LV_ALIGN_TOP_MID, 0, 0);
    lv_chart_set_type(barChart, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(barChart, 0, 0);
    lv_obj_set_style_bg_color(barChart, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(barChart, 0, 0); // Remove border from bar chart
    barSeries = lv_chart_add_series(barChart, lv_color_white(), LV_CHART_AXIS_PRIMARY_Y);

    // Grid for CPU sensor values
    cpuGrid = lv_obj_create(leftHalf);
    lv_obj_set_size(cpuGrid, lv_pct(100), lv_pct(48)); // Adjust height to leave space for padding
    lv_obj_align(cpuGrid, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(cpuGrid, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cpuGrid, CPUGridCellPadding, 0);
    lv_obj_set_style_border_width(cpuGrid, 0, 0); // No border for CPU grid

    updateCPUGridLayout(cpuGrid, cpuCollection, CPUGridRows, CPUGridCols, CPUGridLabelFontSize, CPUGridValueFontSize);

    // Right half for other sensors
    lv_obj_t *rightHalf = lv_obj_create(scr);
    lv_obj_set_size(rightHalf, lv_pct(50), lv_pct(100));
    lv_obj_align(rightHalf, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(rightHalf, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(rightHalf, OtherGridCellPadding, 0);
    lv_obj_set_style_border_width(rightHalf, 0, 0); // No border for the container

    // Add padding to the rightHalf container
    lv_obj_set_style_pad_top(rightHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_bottom(rightHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_left(rightHalf, 10, 0); // Adjust the value as needed
    lv_obj_set_style_pad_right(rightHalf, 10, 0); // Adjust the value as needed

    otherGrid = lv_obj_create(rightHalf);
    lv_obj_set_size(otherGrid, lv_pct(100), lv_pct(100)); // Use full height of rightHalf
    lv_obj_align(otherGrid, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(otherGrid, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(otherGrid, OtherGridCellPadding, 0);
    lv_obj_set_style_border_width(otherGrid, 0, 0); // No border for other grid

    createArcs(otherGrid, otherCollection, OtherGridRows, OtherGridCols);

    screenCreated = true;
}

void DisplayManager::updateCPUDashScreen() {
    if (!screenCreated) {
        createCPUDashScreen();
    }

    if (barChart != nullptr && barSeries != nullptr) {
        lv_chart_set_point_count(barChart, cpuCollection.size());
        for (size_t i = 0; i < cpuCollection.size(); ++i) {
            int cpuUsage = cpuCollection[i].value.toInt();
            lv_chart_set_value_by_id(barChart, barSeries, i, cpuUsage);
        }
    }

    updateCPUGridLayout(cpuGrid, cpuCollection, CPUGridRows, CPUGridCols, CPUGridLabelFontSize, CPUGridValueFontSize);
    updateOtherGridLayout(otherGrid, otherCollection, OtherGridRows, OtherGridCols, OtherGridLabelFontSize, OtherGridValueFontSize);
}

void DisplayManager::createCPUDialsScreen() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr); // Clear previous screen

    // Left half for CPU sensors
    lv_obj_t *leftHalf = lv_obj_create(scr);
    lv_obj_set_size(leftHalf, lv_pct(50), lv_pct(100));
    lv_obj_align(leftHalf, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(leftHalf, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(leftHalf, CPUGridCellPadding, 0);
    lv_obj_set_style_border_width(leftHalf, 0, 0); // No border for the container

    // Bar chart for CPU usage
    barChart = lv_chart_create(leftHalf);
    lv_obj_set_size(barChart, lv_pct(100), lv_pct(48));
    lv_obj_align(barChart, LV_ALIGN_TOP_MID, 0, 0);
    lv_chart_set_type(barChart, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(barChart, 0, 0);
    lv_obj_set_style_bg_color(barChart, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(barChart, 0, 0);
    barSeries = lv_chart_add_series(barChart, lv_color_white(), LV_CHART_AXIS_PRIMARY_Y);

    // Grid for CPU sensor values
    cpuGrid = lv_obj_create(leftHalf);
    lv_obj_set_size(cpuGrid, lv_pct(100), lv_pct(48));
    lv_obj_align(cpuGrid, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(cpuGrid, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cpuGrid, CPUGridCellPadding, 0);
    lv_obj_set_style_border_width(cpuGrid, 0, 0);

    updateCPUGridLayout(cpuGrid, cpuCollection, CPUGridRows, CPUGridCols, CPUGridLabelFontSize, CPUGridValueFontSize);

    // Right half for other sensors rendered as arcs
    lv_obj_t *rightHalf = lv_obj_create(scr);
    lv_obj_set_size(rightHalf, lv_pct(50), lv_pct(100));
    lv_obj_align(rightHalf, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(rightHalf, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(rightHalf, OtherGridCellPadding, 0);
    lv_obj_set_style_border_width(rightHalf, 0, 0);

    createArcs(rightHalf, otherCollection, OtherGridRows, OtherGridCols);

    screenCreated = true;
}


void DisplayManager::updateCPUDialsScreen() {
    if (!screenCreated) {
        createCPUDialsScreen();
    }

    if (barChart != nullptr && barSeries != nullptr) {
        lv_chart_set_point_count(barChart, cpuCollection.size());
        for (size_t i = 0; i < cpuCollection.size(); ++i) {
            int cpuUsage = cpuCollection[i].value.toInt();
            lv_chart_set_value_by_id(barChart, barSeries, i, cpuUsage);
        }
    }

    updateCPUGridLayout(cpuGrid, cpuCollection, CPUGridRows, CPUGridCols, CPUGridLabelFontSize, CPUGridValueFontSize);
    updateArcs(otherCollection, OtherGridRows, OtherGridCols);
}

void DisplayManager::createArcs(lv_obj_t* parent, const std::vector<SensorData>& collection, int rows, int cols) {
    logMessage(LOG_LEVEL_INFO, "Creating arcs for sensors...");

    // Calculate grid dimensions
    lv_coord_t cell_width = lv_pct(100 / cols);
    lv_coord_t cell_height = lv_pct(100 / rows);

    const lv_font_t* labelFont = getFontBySize(OtherGridLabelFontSize);
    const lv_font_t* valueFont = getFontBySize(OtherGridValueFontSize);

    for (size_t i = 0; i < collection.size(); ++i) {
        int row = i / cols;
        int col = i % cols;

        lv_obj_t* cell = lv_obj_create(parent);
        lv_obj_set_size(cell, cell_width, cell_height);
        lv_obj_align(cell, LV_ALIGN_TOP_LEFT, col * cell_width, row * cell_height);
        lv_obj_set_style_bg_color(cell, lv_color_black(), 0);
        lv_obj_set_style_pad_all(cell, OtherGridCellPadding, 0);
        lv_obj_set_style_border_width(cell, 0, 0);

        const auto& sensor = collection[i];

        lv_obj_t* arc = lv_arc_create(cell);
        lv_obj_set_size(arc, lv_pct(80), lv_pct(80)); // Adjust the arc size to fit within the cell
        lv_arc_set_rotation(arc, 135);
        lv_arc_set_bg_angles(arc, 0, 270);
        lv_arc_set_range(arc, 0, 100); // Set arc range to 0-100
        lv_arc_set_value(arc, sensor.value.toInt()); // Set the sensor value
        lv_obj_center(arc);

        lv_obj_t* label = lv_label_create(cell);
        lv_label_set_text(label, sensor.tag.c_str());
        lv_obj_set_style_text_color(label, textColor, 0);
        lv_obj_set_style_text_font(label, labelFont, 0);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);

        lv_obj_t* valueLabel = lv_label_create(cell);
        lv_label_set_text_fmt(valueLabel, "%s", sensor.value.c_str());
        lv_obj_set_style_text_color(valueLabel, textColor, 0);
        lv_obj_set_style_text_font(valueLabel, valueFont, 0);
        lv_obj_align(valueLabel, LV_ALIGN_CENTER, 0, 0);

        logMessage(LOG_LEVEL_INFO, ("Arc created for sensor: " + sensor.tag).c_str());
    }
}

void DisplayManager::updateArcs(const std::vector<SensorData>& collection, int rows, int cols) {
    int itemIndex = 0;
    logMessage(LOG_LEVEL_INFO, "Updating arcs for sensors...");
    lv_obj_t* parent = lv_obj_get_child(lv_scr_act(), 1); // Get the rightHalf container
    for (size_t i = 0; i < collection.size(); ++i) {
        int row = i / cols;
        int col = i % cols;

        lv_obj_t* cell = lv_obj_get_child(parent, itemIndex);
        if (cell) {
            lv_obj_t* arc = lv_obj_get_child(cell, 0);
            if (arc && lv_obj_check_type(arc, &lv_arc_class)) {
                lv_arc_set_value(arc, collection[i].value.toInt()); // Set the sensor value
            }

            lv_obj_t* label = lv_obj_get_child(cell, 1);
            if (label && lv_obj_check_type(label, &lv_label_class)) {
                lv_label_set_text(label, collection[i].tag.c_str());
                lv_obj_set_style_text_font(label, getFontBySize(OtherGridLabelFontSize), 0);
            }

            lv_obj_t* valueLabel = lv_obj_get_child(cell, 2);
            if (valueLabel && lv_obj_check_type(valueLabel, &lv_label_class)) {
                lv_label_set_text_fmt(valueLabel, "%s", collection[i].value.c_str());
                lv_obj_set_style_text_font(valueLabel, getFontBySize(OtherGridValueFontSize), 0);
            }

            logMessage(LOG_LEVEL_INFO, ("Arc updated for sensor: " + collection[i].tag).c_str());
        }
        ++itemIndex;
    }
}





void DisplayManager::updateCPUGridLayout(lv_obj_t* grid, const std::vector<SensorData>& collection, int rows, int cols, int labelFontSize, int valueFontSize) {
    lv_obj_clean(grid); // Clear previous grid items
    const lv_font_t* labelFont = getFontBySize(labelFontSize);
    const lv_font_t* valueFont = getFontBySize(valueFontSize);

    static lv_coord_t* col_dsc = new lv_coord_t[cols + 1];
    static lv_coord_t* row_dsc = new lv_coord_t[rows + 1];

    for (int i = 0; i < cols; i++) {
        col_dsc[i] = LV_GRID_FR(1);
    }
    col_dsc[cols] = LV_GRID_TEMPLATE_LAST;

    for (int i = 0; i < rows; i++) {
        row_dsc[i] = LV_GRID_FR(1);
    }
    row_dsc[rows] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    int itemIndex = 0;
    for (const auto& sensor : collection) {
        int row = itemIndex / cols;
        int col = itemIndex % cols;

        lv_obj_t* cell = lv_obj_create(grid);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_style_bg_color(cell, lv_color_black(), 0);
        lv_obj_set_style_pad_all(cell, CPUGridCellPadding, 0);
        lv_obj_set_style_border_color(cell, lv_color_black(), 0); // Set border color to black
        lv_obj_set_style_border_width(cell, 1, 0); // Set border width
        lv_obj_set_scrollbar_mode(cell, LV_SCROLLBAR_MODE_OFF); // Disable scrollbars

        lv_obj_t* label = lv_label_create(cell);
        lv_label_set_text_fmt(label, "%s\n%s", sensor.tag.c_str(), sensor.value.c_str());
        lv_obj_set_style_text_font(label, labelFont, 0);
        lv_obj_set_style_text_color(label, textColor, 0); // Apply text color
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); // Center text alignment
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Center the label within the cell

        ++itemIndex;
    }
}

void DisplayManager::updateOtherGridLayout(lv_obj_t* grid, const std::vector<SensorData>& collection, int rows, int cols, int labelFontSize, int valueFontSize) {
    lv_obj_clean(grid); // Clear previous grid items
    const lv_font_t* labelFont = getFontBySize(labelFontSize);
    const lv_font_t* valueFont = getFontBySize(valueFontSize);

    static lv_coord_t* col_dsc = new lv_coord_t[cols + 1];
    static lv_coord_t* row_dsc = new lv_coord_t[rows + 1];

    for (int i = 0; i < cols; i++) {
        col_dsc[i] = LV_GRID_FR(1);
    }
    col_dsc[cols] = LV_GRID_TEMPLATE_LAST;

    for (int i = 0; i < rows; i++) {
        row_dsc[i] = LV_GRID_FR(1);
    }
    row_dsc[rows] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    int itemIndex = 0;
    for (const auto& sensor : collection) {
        int row = itemIndex / cols;
        int col = itemIndex % cols;

        lv_obj_t* cell = lv_obj_create(grid);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_style_bg_color(cell, lv_color_black(), 0);
        lv_obj_set_style_pad_all(cell, OtherGridCellPadding, 0);
        lv_obj_set_style_border_color(cell, lv_color_black(), 0); // Set border color to black
        lv_obj_set_style_border_width(cell, 1, 0); // Set border width
        lv_obj_set_scrollbar_mode(cell, LV_SCROLLBAR_MODE_OFF); // Disable scrollbars

        lv_obj_t* label = lv_label_create(cell);
        lv_label_set_text_fmt(label, "%s\n%s", sensor.tag.c_str(), sensor.value.c_str());
        lv_obj_set_style_text_font(label, labelFont, 0);
        lv_obj_set_style_text_color(label, textColor, 0); // Apply text color
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0); // Center text alignment
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Center the label within the cell

        ++itemIndex;
    }
}

const lv_font_t* DisplayManager::getFontBySize(int fontSize) {
    switch (fontSize) {
        case 12: return &lv_font_montserrat_12;
        case 14: return &lv_font_montserrat_14;
        case 16: return &lv_font_montserrat_16;
        case 18: return &lv_font_montserrat_18;
        case 20: return &lv_font_montserrat_20;
        case 22: return &lv_font_montserrat_22;
        case 24: return &lv_font_montserrat_24;
        case 26: return &lv_font_montserrat_26;
        case 28: return &lv_font_montserrat_28;
        case 30: return &lv_font_montserrat_30;
        default: return &lv_font_montserrat_18;
    }
}
