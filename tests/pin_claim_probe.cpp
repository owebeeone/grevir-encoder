#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>
#include <cstdint>

namespace {
struct Backend {
  static void pinMode(unsigned, ardo::gpio::InputPinMode);
  static bool digitalRead(unsigned);
};

#if CASE_ID == 0
using PinA = ardo::InputPin<Backend, 5>;
using PinB = ardo::InputPin<Backend, 6>;
using Encoder = quad::QuadEncoder<PinA, PinB>;
using Module = quad::QuadEncoderModule<Encoder>;
using App = ardo::Application<Module>;
#elif CASE_ID == 1
using First = quad::QuadEncoderModule<
  quad::QuadEncoder<ardo::InputPin<Backend, 1>, ardo::InputPin<Backend, 2>>>;
using Second = quad::QuadEncoderModule<
  quad::QuadEncoder<ardo::InputPin<Backend, 3>, ardo::InputPin<Backend, 4>>>;
using App = ardo::Application<First, Second>;
#elif CASE_ID == 2
using Encoder = quad::QuadEncoder<ardo::InputPin<Backend, 5>, ardo::InputPin<Backend, 5>>;
using App = ardo::Application<quad::QuadEncoderModule<Encoder>>;
#elif CASE_ID == 3
using First = quad::QuadEncoderModule<
  quad::QuadEncoder<ardo::InputPin<Backend, 1>, ardo::InputPin<Backend, 2>>>;
using Second = quad::QuadEncoderModule<
  quad::QuadEncoder<ardo::InputPin<Backend, 2>, ardo::InputPin<Backend, 3>>>;
using App = ardo::Application<First, Second>;
#endif

void instantiate_lifecycle() {
  App::runSetup();
  App::runLoop();
}
}
