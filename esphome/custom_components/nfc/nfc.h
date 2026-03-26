#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
//#include <Adafruit_I2CDevice.h>
//#include <Adafruit_SPIDevice.h>
#include <Adafruit_PN532.h>

namespace esphome {
namespace nfc {

class NFCComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_reset_pin(GPIOPin *reset_pin) { reset_pin_ = reset_pin; }
  void set_tx_pin(GPIOPin *tx_pin) { tx_pin_ = tx_pin; }
  void set_rx_pin(GPIOPin *rx_pin) { rx_pin_ = rx_pin; }
  void set_readed_value_sensor(text_sensor::TextSensor *readed_value_sensor) {
    readed_value_sensor_ = readed_value_sensor;
  }
  void add_tag(const std::string &id, const std::string &desc);
  void list_tags();
  std::string has_tag(const std::string &id);

 protected:
  text_sensor::TextSensor *readed_value_sensor_{nullptr};
  std::string buffer_;
  Adafruit_PN532 *nfc_{nullptr};
  HardwareSerial *my_serial_{nullptr};
  GPIOPin *tx_pin_{nullptr};
  GPIOPin *rx_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  uint32_t last_update_{0};

};

}  // namespace nfc
}  // namespace esphome