#include "esphome.h"
using namespace esphome;

class FanFloatOutput : public Component, public FloatOutput {
 public:
  void setup() override {
    pinMode(D6, OUTPUT);
    pinMode(D8, OUTPUT);
    pinMode(D2, OUTPUT);
  }

  void write_state(float state) override {
    int spd = state*10;
    digitalWrite(D2, false);
    digitalWrite(D6, false);
    digitalWrite(D8, false);
    if (spd==3) digitalWrite(D2, true);
    if (spd==6) digitalWrite(D8, true);
    if (spd==10) digitalWrite(D6, true);
  }
};
