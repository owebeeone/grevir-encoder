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
  using quad::QuadEncoderBase::Action;
  using quad::QuadEncoderBase::decodeAction;
};
}

static_assert(Access::decodeAction(0) == Access::Action::unchanged);
static_assert(Access::decodeAction(1) == Access::Action::increment);
static_assert(Access::decodeAction(2) == Access::Action::decrement);
static_assert(Access::decodeAction(3) == Access::Action::indeterminate);
static_assert(Access::decodeAction(7) == Access::Action::increment);
static_assert(Access::decodeAction(14) == Access::Action::increment);
static_assert(Access::decodeAction(8) == Access::Action::increment);
static_assert(Access::decodeAction(4) == Access::Action::decrement);

void instantiate_encoder() {
  ardo::Application<Module>::runSetup();
  ardo::Application<Module>::runLoop();
  Module::quadEncoder.getCurrentPosition();
  Module::quadEncoder.setCurrentPosition(0);
  Scaled scaled;
  scaled.scaleValue(1);
}
