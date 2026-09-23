# Grevir Encoder

**Public API:** [Grevir Encoder](https://github.com/owebeeone/grevir-wz/blob/main/docs/api/encoder.md).
See [installation](https://github.com/owebeeone/grevir-wz/blob/main/docs/install.md) and
[supported platforms](https://github.com/owebeeone/grevir-wz/blob/main/docs/supported.md).
The workspace `/docs` is the current user-facing contract; development
checkpoints below are historical.

Quadrature decoding and a module wrapper for injected pins.

## Development record (historical)

Quadrature decoding, optional interactive scaling and a Core module wrapper,
extracted from Ardoinus `ardOQuadEncoder`. Public types retain the `quad` names
and original scaler spellings. Pins and clocks are injected; the package has no
Arduino, FastLED or MCU-backend dependency.

`GrevirEncoder.h` includes `<grevir/encoder/encoder.hpp>`. Production use needs
Grevir Base, Time, Core and Peripherals. Callers supply pin types with static
`get()` and Core GPIO claims, typically `ardo::InputPin<Backend, N>`.

## Installed use

```cmake
find_package(grevir-encoder CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE grevir::encoder)
```

The target supplies C++23 and its Grevir dependencies. Normal production builds
and installed consumers do not require Catch2 or Test Support. Arduino library
layout is supplied; Arduino and target builds are not validated.

```cpp
#include <GrevirEncoder.h>
#include <GrevirPeripherals.h>

using PinA = ardo::InputPin<BoardGPIO, 5>;
using PinB = ardo::InputPin<BoardGPIO, 6>;
using Encoder = quad::QuadEncoder<PinA, PinB>;
using Module = quad::QuadEncoderModule<Encoder>;
using App = ardo::Application<Module>;
```

See [the installed consumer](tests/installed-consumer/main.cpp) for a host
application that steps one count. FastLED and stepper combination sketches remain
separate planned examples.

## Decoding contract

- Physical high reads as a zero bit and physical low as a one bit. That polarity
  is unchanged from Ardoinus.
- The first `iterate()` latches the current pair and returns zero. Unchanged
  levels also return zero. Adjacent Gray-code steps return ±1 and set the last
  direction. A two-bit jump is indeterminate: it returns `2 * lastDirection`, or
  zero when no direction has been observed.
- `getCurrentPosition()` accumulates `scaleValue(change)`. The default scaler is
  identity (`NUllScaler`). `setCurrentPosition` replaces the count.
- `QuadEncoderModule` claims both pins, sets them up through Core, and calls
  `iterate()` once per `runLoop()`. Distinct pin pairs are independent module
  instantiations. Reusing a pin fails Core's resource-conflict check.

## Scaling and virtual dispatch

`InteractiveScaler<Clock>` is an explicit floating-point algorithm. `Clock`
supplies `TimeType` and `now()`. Time's `RelativeInteractiveScaler` still ignores
the 100/500 constructor periods and uses its built-in 4/100 bounds; this extraction
does not change that scaler. Scaled unit steps can truncate to zero until the
retained remainder accumulates a whole count.

`QuadEncoderBase` retains virtual `getInput()`/`scaleValue()`. That is the
inherited dispatch model, not a new allocation. Host tests do not measure AVR
vtable cost.

## Extraction corrections and evidence

The Arduino `CoreIF` clock and implicit `InputPin<N>` backend are replaced by
injected GPIO and clock types. Decoding, polarity, scaler names and module
lifecycle are otherwise preserved. The old 16-line harness compiled against
Arduino pin numbers; it is adapted into host cases with a mock backend.

Native Catch2 cases cover startup latch, clockwise and counter-clockwise cycles,
indeterminate doubles, identity scaling, injected-clock interactive scaling,
the historical module setup/loop, and two independent instances. Two public
headers compile independently. Two valid and two rejected pin-claim probes pass.
Isolated production/install/consumer checks run with Catch2 and Test Support
discovery disabled. AVR compiler and hardware validation remain on hold.

Standalone host validation can use installed dependencies:

```sh
cmake -S . -B build/host -DCMAKE_PREFIX_PATH=/path/to/grevir/install \
  -DGREVIR_BUILD_HOST_TESTS=ON -DGREVIR_BUILD_COMPILE_CHECKS=ON \
  -DGREVIR_CATCH2_SOURCE_DIR=/path/to/Catch2-3.8.1
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

Stepper, FastLED and Arduino adapters remain separate planned packages.
