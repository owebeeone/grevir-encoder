#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>
#include <array>
#include <cstdint>

namespace {
struct Backend {
  inline static std::array<bool, 8> inputs{true, true, true, true, true, true, true, true};
  inline static bool pin_a_configured = false;
  inline static bool pin_b_configured = false;
  static void pinMode(unsigned pin, ardo::gpio::InputPinMode mode) {
    if (pin == 5 && mode == ardo::gpio::InputPinMode::PullUp) {
      pin_a_configured = true;
    }
    if (pin == 6 && mode == ardo::gpio::InputPinMode::PullUp) {
      pin_b_configured = true;
    }
  }
  static bool digitalRead(unsigned pin) {
    return inputs.at(pin);
  }
};
using Encoder = quad::QuadEncoder<ardo::InputPin<Backend, 5>, ardo::InputPin<Backend, 6>>;
using Module = quad::QuadEncoderModule<Encoder>;
using App = ardo::Application<Module>;
}

int main() {
  App::runSetup();
  if (!Backend::pin_a_configured || !Backend::pin_b_configured) {
    return 1;
  }
  App::runLoop();
  if (Module::quadEncoder.getCurrentPosition() != 0) {
    return 2;
  }
  Backend::inputs[5] = false;
  App::runLoop();
  if (Module::quadEncoder.getCurrentPosition() != 1) {
    return 3;
  }
  Module::quadEncoder.setCurrentPosition(10);
  if (Module::quadEncoder.getCurrentPosition() != 10) {
    return 4;
  }
  return 0;
}
