#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>
#include <cstdint>

namespace {
struct Backend {
  static void pinMode(unsigned, ardo::gpio::InputPinMode);
  static bool digitalRead(unsigned);
};
struct Clock {
  using TimeType = setl::Time<std::uint32_t, setl::TimeUnit::MICROS>;
  static TimeType now();
};
using PinA = ardo::InputPin<Backend, 5>;
using PinB = ardo::InputPin<Backend, 6>;
using Encoder = quad::QuadEncoder<PinA, PinB>;
using Scaled = quad::QuadEncoder<PinA, PinB, quad::InteractiveScaler<Clock>>;
using Module = quad::QuadEncoderModule<Encoder>;
struct Access : quad::QuadEncoderBase {
  static constexpr bool unchanged(State index) {
    return decodeAction(index) == Action::unchanged;
  }
  static constexpr bool increment(State index) {
    return decodeAction(index) == Action::increment;
  }
  static constexpr bool decrement(State index) {
    return decodeAction(index) == Action::decrement;
  }
  static constexpr bool indeterminate(State index) {
    return decodeAction(index) == Action::indeterminate;
  }
};
}

static_assert(Access::unchanged(0));
static_assert(Access::increment(1));
static_assert(Access::decrement(2));
static_assert(Access::indeterminate(3));
static_assert(Access::increment(7));
static_assert(Access::increment(14));
static_assert(Access::increment(8));
static_assert(Access::decrement(4));

void instantiate_encoder() {
  ardo::Application<Module>::runSetup();
  ardo::Application<Module>::runLoop();
  Module::quadEncoder.getCurrentPosition();
  Module::quadEncoder.setCurrentPosition(0);
  Scaled scaled;
  scaled.scaleValue(1);
}
