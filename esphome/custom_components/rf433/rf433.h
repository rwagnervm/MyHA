#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <RCSwitch.h>

namespace esphome {
namespace rf433 {

class RF433Component : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }
  void set_readed_value_sensor(text_sensor::TextSensor *readed_value_sensor) {
    readed_value_sensor_ = readed_value_sensor;
  }

 protected:
  InternalGPIOPin *pin_{nullptr};
  text_sensor::TextSensor *readed_value_sensor_{nullptr};
  RCSwitch my_switch_;
};

}  // namespace rf433
}  // namespace esphome