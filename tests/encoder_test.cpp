#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace {
struct GPIO {
  inline static std::array<bool, 32> inputs{};
  inline static std::vector<std::string> events;

  static void record(unsigned pin, const std::string& action) {
    events.push_back(std::to_string(pin) + ":" + action);
  }
  static void pinMode(unsigned pin, ardo::gpio::InputPinMode mode) {
    record(pin, mode == ardo::gpio::InputPinMode::PullUp ? "pullup" : "input");
  }
  static bool digitalRead(unsigned pin) {
    return inputs.at(pin);
  }
};

struct Clock {
  using TimeType = setl::Time<std::uint32_t, setl::TimeUnit::MICROS>;
  inline static TimeType current{};
  static TimeType now() {
    return current;
  }
  static void set(std::uint32_t ticks) {
    current = TimeType(ticks);
  }
};

struct Fixture {
  Fixture() {
    Clock::set(0);
    GPIO::inputs.fill(false);
    GPIO::events.clear();
  }
};

using PinA = ardo::InputPin<GPIO, 5>;
using PinB = ardo::InputPin<GPIO, 6>;
using Encoder = quad::QuadEncoder<PinA, PinB>;
using Module = quad::QuadEncoderModule<Encoder>;
using App = ardo::Application<Module>;

void drive(bool pin_a_high, bool pin_b_high) {
  GPIO::inputs[5] = pin_a_high;
  GPIO::inputs[6] = pin_b_high;
}

Encoder& module_encoder() {
  return Module::quadEncoder;
}
} // namespace

TEST_CASE_METHOD(Fixture, "first sample latches the level and does not move", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  REQUIRE(encoder.iterate() == 0);
  REQUIRE(encoder.getCurrentPosition() == 0);
  REQUIRE(encoder.iterate() == 0);
  REQUIRE(encoder.getCurrentPosition() == 0);
}

TEST_CASE_METHOD(Fixture, "clockwise quadrature cycle advances one count per edge", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  REQUIRE(encoder.iterate() == 0);
  drive(false, true);
  REQUIRE(encoder.iterate() == 1);
  drive(false, false);
  REQUIRE(encoder.iterate() == 1);
  drive(true, false);
  REQUIRE(encoder.iterate() == 1);
  drive(true, true);
  REQUIRE(encoder.iterate() == 1);
  REQUIRE(encoder.getCurrentPosition() == 4);
}

TEST_CASE_METHOD(Fixture, "counter-clockwise quadrature cycle decrements", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  REQUIRE(encoder.iterate() == 0);
  drive(true, false);
  REQUIRE(encoder.iterate() == -1);
  drive(false, false);
  REQUIRE(encoder.iterate() == -1);
  drive(false, true);
  REQUIRE(encoder.iterate() == -1);
  drive(true, true);
  REQUIRE(encoder.iterate() == -1);
  REQUIRE(encoder.getCurrentPosition() == -4);
}

TEST_CASE_METHOD(Fixture, "skipped state doubles the last direction", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  REQUIRE(encoder.iterate() == 0);
  drive(false, true);
  REQUIRE(encoder.iterate() == 1);
  drive(true, false);
  REQUIRE(encoder.iterate() == 2);
  REQUIRE(encoder.getCurrentPosition() == 3);
}

TEST_CASE_METHOD(Fixture, "skipped state without a last direction stays still", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  REQUIRE(encoder.iterate() == 0);
  drive(false, false);
  REQUIRE(encoder.iterate() == 0);
  REQUIRE(encoder.getCurrentPosition() == 0);
}

TEST_CASE_METHOD(Fixture, "setCurrentPosition replaces the accumulated count", "[encoder]") {
  Encoder encoder;
  drive(true, true);
  encoder.iterate();
  drive(false, true);
  encoder.iterate();
  encoder.setCurrentPosition(40);
  REQUIRE(encoder.getCurrentPosition() == 40);
  drive(false, false);
  REQUIRE(encoder.iterate() == 1);
  REQUIRE(encoder.getCurrentPosition() == 41);
}

TEST_CASE_METHOD(Fixture, "identity scaler leaves deltas unchanged", "[encoder]") {
  REQUIRE(quad::NUllScaler::scale(3L) == 3L);
  Encoder encoder;
  drive(true, true);
  encoder.iterate();
  drive(false, true);
  REQUIRE(encoder.scaleValue(1) == 1);
}

TEST_CASE_METHOD(Fixture, "interactive scaler reads the injected clock", "[encoder]") {
  quad::InteractiveScaler<Clock> scaler;
  Clock::set(0);
  const long first = scaler.scale(100);
  Clock::set(4);
  const long second = scaler.scale(100);
  REQUIRE(first != 0);
  REQUIRE(second != 0);
  Clock::set(200);
  const long slow = scaler.scale(100);
  REQUIRE(slow != first);
  quad::InteractiveScaler<Clock> unit;
  Clock::set(0);
  REQUIRE(unit.scale(1) == 0);
  bool accumulated = false;
  for (unsigned step = 1; step <= 8; ++step) {
    Clock::set(step * 4u);
    if (unit.scale(1) != 0) {
      accumulated = true;
    }
  }
  REQUIRE(accumulated);
  using Scaled = quad::QuadEncoder<PinA, PinB, quad::InteractiveScaler<Clock>>;
  Scaled encoder;
  drive(true, true);
  encoder.iterate();
  const bool cw[][2] = {{false, true}, {false, false}, {true, false}, {true, true}};
  for (int step = 0; step < 16; ++step) {
    Clock::set(static_cast<std::uint32_t>(step + 1) * 4u);
    const auto& levels = cw[static_cast<unsigned>(step) % 4u];
    drive(levels[0], levels[1]);
    encoder.iterate();
  }
  REQUIRE(encoder.getCurrentPosition() != 0);
}

TEST_CASE_METHOD(Fixture, "module setup claims pins and loop follows the historical harness", "[encoder]") {
  drive(true, true);
  App::runSetup();
  REQUIRE(GPIO::events == std::vector<std::string>{"6:pullup", "5:pullup"});
  App::runLoop();
  REQUIRE(module_encoder().getCurrentPosition() == 0);
  drive(false, true);
  App::runLoop();
  REQUIRE(module_encoder().getCurrentPosition() == 1);
  module_encoder().getCurrentPosition();
}

TEST_CASE_METHOD(Fixture, "two encoder modules keep independent positions", "[encoder]") {
  using First = quad::QuadEncoderModule<
    quad::QuadEncoder<ardo::InputPin<GPIO, 1>, ardo::InputPin<GPIO, 2>>>;
  using Second = quad::QuadEncoderModule<
    quad::QuadEncoder<ardo::InputPin<GPIO, 3>, ardo::InputPin<GPIO, 4>>>;
  using Dual = ardo::Application<First, Second>;
  GPIO::inputs[1] = true;
  GPIO::inputs[2] = true;
  GPIO::inputs[3] = true;
  GPIO::inputs[4] = true;
  Dual::runSetup();
  Dual::runLoop();
  GPIO::inputs[1] = false;
  Dual::runLoop();
  REQUIRE(First::quadEncoder.getCurrentPosition() == 1);
  REQUIRE(Second::quadEncoder.getCurrentPosition() == 0);
  GPIO::inputs[4] = false;
  Dual::runLoop();
  REQUIRE(First::quadEncoder.getCurrentPosition() == 1);
  REQUIRE(Second::quadEncoder.getCurrentPosition() == -1);
}
