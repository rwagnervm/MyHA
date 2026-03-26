#include "rf433.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rf433 {

static const char *const TAG = "rf433";

void RF433Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RF433...");
  if (this->pin_ != nullptr) {
    this->pin_->setup();
    this->my_switch_.enableReceive(this->pin_->get_pin());
  }
}

void RF433Component::loop() {
  if (this->my_switch_.available()) {
    if (this->my_switch_.getReceivedBitlength() >= 24) {
      long value = this->my_switch_.getReceivedValue();
      if (this->readed_value_sensor_ != nullptr) {
        this->readed_value_sensor_->publish_state(std::to_string(value));
      }
    }
    this->my_switch_.resetAvailable();
  }
}

void RF433Component::dump_config() {
  ESP_LOGCONFIG(TAG, "RF433:");
  LOG_PIN("  Pin: ", this->pin_);
  if (this->readed_value_sensor_ != nullptr) {
    LOG_TEXT_SENSOR("  ", "Readed Value", this->readed_value_sensor_);
  }
}

}  // namespace rf433
}  // namespace esphome