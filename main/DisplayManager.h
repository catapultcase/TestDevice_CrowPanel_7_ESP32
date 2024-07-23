#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <lvgl.h>
#include <ArduinoJson.h>
#include <vector>
#include "LGFXSetup.h"

enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

struct SensorData {
    String tag;
    String value;
    int order;
    String category;
    String componentName;

    bool operator<(const SensorData& other) const {
        return order < other.order;
    }
};

class DisplayManager {
public:
    DisplayManager();
    virtual void init();
    void createHomeScreen();
    virtual void handleIncomingData(const String& json);
    void setLogLevel(LogLevel level);
    void logMessage(LogLevel level, const char* message);

protected:
    LGFX lcd;
    lv_obj_t* homeLabel;
    static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

    void createDataGridScreen();
    void updateDataGridScreen();
    void createCPUDashScreen();
    void updateCPUDashScreen();
    void createCPUDialsScreen();      // New method for creating CPUDials layout
    void updateCPUDialsScreen();      // New method for updating CPUDials layout
    void createArcs(lv_obj_t* parent, const std::vector<SensorData>& collection, int rows, int cols);  // New method for creating arcs
    void updateArcs(const std::vector<SensorData>& collection, int rows, int cols);                    // New method for updating arcs
    void updateCPUGridLayout(lv_obj_t* grid, const std::vector<SensorData>& collection, int rows, int cols, int labelFontSize, int valueFontSize);
    void updateOtherGridLayout(lv_obj_t* grid, const std::vector<SensorData>& collection, int rows, int cols, int labelFontSize, int valueFontSize);
    const lv_font_t* getFontBySize(int fontSize);

    int CPUGridLabelFontSize;
    int CPUGridValueFontSize;
    int OtherGridLabelFontSize;
    int OtherGridValueFontSize;
    int CPUGridCellPadding;
    int OtherGridCellPadding;
    int CPUGridRows;
    int CPUGridCols;
    int OtherGridRows;
    int OtherGridCols;

    bool screenCreated;

    LogLevel currentLogLevel;

    std::vector<SensorData> sensorCollection;
    std::vector<SensorData> cpuCollection;
    std::vector<SensorData> otherCollection;

    lv_obj_t* barChart;
    lv_chart_series_t* barSeries;
    lv_obj_t* otherGrid;
    lv_obj_t* cpuGrid;
    lv_obj_t* grid;
    lv_color_t textColor;
};

#endif // DISPLAY_MANAGER_H
