
#include "esphome.h"
#include <BLEDevice.h>
#include "StaggKettle.h"
#include <FreeRTOS.h>

static StaggKettle kettle;

static bool xLifted;
static bool xPower;
static bool refresh_state;
static bool xIdle;
static byte xCurrentTemp = -1;
static byte xTargetTemp = -1;

unsigned long lastHeapDebug = -1;

class Kettle : public Component, public Climate
{
public:
    void setup() override
    {
        BLEDevice::init("");
        esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        kettle.scan();
        this->set_visual_min_temperature_override(40.0);
        this->set_visual_max_temperature_override(100.0);
        this->set_visual_temperature_step_override(1.0);
    }
    void loop() override
    {
        kettle.loop();

        refresh_state = false;

        if (xPower != kettle.isOn())
        {
            xPower = kettle.isOn();
            ESP_LOGD("Stagg", "Power: %s", xPower ? "on" : "off");
            this->mode = xPower ? climate::CLIMATE_MODE_HEAT : climate::CLIMATE_MODE_OFF;
            refresh_state = true;
        }

        if (xCurrentTemp != kettle.getCurrentTemp()) {
            xCurrentTemp = kettle.getCurrentTemp();
            ESP_LOGD("Stagg", "Current Temp: %d", xCurrentTemp);
            this->current_temperature = float(xCurrentTemp);
            refresh_state = true;

        }
        if (xTargetTemp != kettle.getTargetTemp()) {
            xTargetTemp = kettle.getTargetTemp();
            ESP_LOGD("Stagg", "Target Temp: %d", xTargetTemp);
            this->target_temperature = float(xTargetTemp);
            refresh_state = true;
        }

        if (xIdle != kettle.isIdle() || refresh_state)
        {
            xIdle = kettle.isIdle();
            ESP_LOGD("Stagg", "Idle: %s", xIdle ? "true" : "false");
            if (xPower && xIdle) {
                this->action = climate::CLIMATE_ACTION_IDLE;
            }
            if (xPower && !xIdle) {
                this->action = climate::CLIMATE_ACTION_HEATING;
            }
            refresh_state = true;
        }


        if(refresh_state) {
            this->publish_state();
        }
        unsigned long timeNow = millis();
        // Handle 64 bit wraparound
        if (timeNow < lastHeapDebug)
          lastHeapDebug = timeNow;  
        if (timeNow - lastHeapDebug > 500) {
            ESP_LOGD("Stagg", "Free Heap: %d", ESP.getFreeHeap());
            lastHeapDebug = timeNow;
        }
    }
    void control(const ClimateCall &call) override
    {
        ESP_LOGD("Stagg"
                 "custom",
                 "Control Called");
        if (call.get_mode().has_value())
        {
            // User requested mode change
            ClimateMode mode = *call.get_mode();
            if (mode == climate::CLIMATE_MODE_HEAT && !kettle.isOn()) {
                ESP_LOGD("Stagg", "turning on the kettle");
                kettle.on();
            }
            if (mode == climate::CLIMATE_MODE_OFF && kettle.isOn()) {
                ESP_LOGD("Stagg", "turning off kettle");
                kettle.off();
            }
            this->mode = mode;
            this->publish_state();
        }
        if (call.get_target_temperature().has_value())
        {
            byte temp = static_cast<int>(*call.get_target_temperature());
            kettle.setTemp(temp);
        }
    }
    ClimateTraits traits() override
    {
        // The capabilities of the climate device
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(true);
        traits.set_supports_heat_mode(true);
        traits.set_supports_action(true);
        return traits;
    }
};
