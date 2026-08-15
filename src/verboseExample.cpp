// // Pulls in the Arduino core for ESP32: Serial, millis(), delay(),
// // neopixelWrite(), the ESP object, and the uint8_t/size_t typedefs used below.
// // PlatformIO adds this automatically for .ino files but not for .cpp, so it
// // must be explicit here.
// #include <Arduino.h>

// // Pin map per the v1.0 spec.
// // The on-board LED is a WS2812 on GPIO48 and needs neopixelWrite(), 
// // not digitalWrite().

// // GPIO48 drives the on-board addressable RGB LED. constexpr means the value is
// // fixed at compile time, so it costs no RAM. uint8_t because GPIO numbers fit
// // in 0-255 and that is the type every Arduino pin function expects.
// constexpr uint8_t PIN_LED_ONBOARD_RGB = 48;
// // GPIO4 drives the external LED through its 220 ohm series resistor. Declared
// // now for v1.0; nothing in this sketch uses it yet.
// constexpr uint8_t PIN_LED_EXTERNAL = 4;
// // GPIO13 reads the push button. Declared now for v2.0; unused in this sketch.
// constexpr uint8_t PIN_BUTTON = 13;

// // Full scale is painfully bright and draws more current for no added
// // visibility. The core's own RGB_BRIGHTNESS default is 64; stay under it.

// // The per-channel value used for "on" below: 32 out of a possible 255, i.e.
// // about 12% duty. Raising this makes the LED brighter and hungrier.
// constexpr uint8_t ONBOARD_LEVEL = 32;

// // Groups one colour's three channel values with a human-readable label, so the
// // serial log can name the colour instead of printing three bare numbers.
// struct Color {
//   // Pointer to a string literal such as "red". const char* (not String) keeps
//   // the literal in flash rather than copying it into RAM.
//   const char *name;
//   // The three WS2812 channel values, 0-255 each: red, green, blue.
//   uint8_t r, g, b;
// };

// // Cycled once per tick so a glance at the board tells you loop() is alive.

// // The colour order the LED steps through. The empty [] lets the compiler count
// // the entries, so adding a colour here needs no other change.
// constexpr Color ONBOARD_SEQUENCE[] = {
//     // Red on, green and blue off.
//     {"red", ONBOARD_LEVEL, 0, 0},
//     // Green only.
//     {"green", 0, ONBOARD_LEVEL, 0},
//     // Blue only.
//     {"blue", 0, 0, ONBOARD_LEVEL},
//     // All channels zero, which switches the LED off.
//     {"off", 0, 0, 0},
// };
// // Number of entries in the array above: total byte size divided by the size of
// // one entry. Computed by the compiler, and it re-counts itself if the array
// // changes, which a hard-coded 4 would not.
// constexpr size_t ONBOARD_SEQUENCE_LEN =
//     sizeof(ONBOARD_SEQUENCE) / sizeof(ONBOARD_SEQUENCE[0]);

// // neopixelWrite() bit-bangs the WS2812 protocol over RMT. Passing GPIO48
// // directly is equivalent to passing the core's RGB_BUILTIN, which is a virtual
// // pin (SOC_GPIO_PIN_COUNT + 48) that the driver maps back to 48.

// // Sends one colour to the on-board LED. Takes the Color by const reference:
// // "const" so it cannot modify the caller's copy, "&" so the struct is not
// // duplicated on every call.
// void setOnboardColor(const Color &c) {
//   // neopixelWrite(pin, red, green, blue) - four arguments, all uint8_t:
//   //   pin   GPIO the WS2812's data line is wired to (48 here)
//   //   red   red channel intensity, 0-255
//   //   green green channel intensity, 0-255
//   //   blue  blue channel intensity, 0-255
//   // It clocks all 24 bits out through the RMT peripheral and returns once the
//   // LED has latched them. Note the WS2812 wants the bits in green-red-blue
//   // order on the wire; the driver reorders them, so pass plain R, G, B.
//   neopixelWrite(PIN_LED_ONBOARD_RGB, c.r, c.g, c.b);
// }

// // Runs once after every reset or power-up, before loop(). The Arduino core
// // calls it for you; it takes no arguments and returns nothing.
// void setup() {
//   // Serial.begin(baud) - one argument: bits per second. 115200 must match the
//   // monitor_speed in platformio.ini or the output arrives as garbage. This
//   // sketch's Serial is UART0, wired to the CH343 bridge, because
//   // ARDUINO_USB_CDC_ON_BOOT is 0.
//   Serial.begin(115200);
//   // No-op on UART0, but keeps this correct if Serial ever moves to native USB,
//   // where the first prints are lost until the host enumerates the port.
//   // "!Serial" asks whether the port is not ready yet; millis() < 3000 caps the
//   // wait at three seconds so the board never hangs here forever.
//   while (!Serial && millis() < 3000) {
//     // delay(ms) - one argument: milliseconds to block. Sleeping 10 ms between
//     // checks avoids spinning the CPU flat out while waiting.
//     delay(10);
//   }

//   // Serial.println() with no argument sends just a newline, pushing the banner
//   // clear of any boot messages the ROM already printed on this line.
//   Serial.println();
//   // Serial.println(text) - one argument: the string to send, followed by a
//   // newline.
//   Serial.println("Hello, world!");
//   // Serial.printf(format, ...) - a format string plus one argument per
//   // placeholder, same rules as C's printf:
//   //   %s  ESP.getChipModel()    returns a const char* such as "ESP32-S3"
//   //   %d  ESP.getChipRevision() returns the silicon revision as an integer
//   //   %u  getCpuFrequencyMhz()  returns the current CPU clock in MHz
//   // The \n at the end starts a new line.
//   Serial.printf("chip:  %s rev %d, %u MHz\n", ESP.getChipModel(),
//                 ESP.getChipRevision(), getCpuFrequencyMhz());
//   // Both should match the N16R8 module: 16 MB flash, 8 MB PSRAM. A zero here
//   // means the memory_type / flash_size settings did not take.

//   // ESP.getFlashChipSize() takes no arguments and returns the flash size in
//   // bytes, so dividing twice by 1024 converts bytes -> KB -> MB.
//   Serial.printf("flash: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
//   // ESP.getPsramSize() takes no arguments and returns usable PSRAM in bytes.
//   // Zero here means PSRAM was never initialised.
//   Serial.printf("psram: %u bytes\n", ESP.getPsramSize());

//   // A WS2812 holds its last colour through a reset, so clear it rather than
//   // inheriting whatever the previous sketch left behind.
//   // The braces build a temporary Color in place; passing all-zero channels
//   // switches the LED off.
//   setOnboardColor({"off", 0, 0, 0});
//   // %u prints PIN_LED_ONBOARD_RGB, so the log states which GPIO is being
//   // driven rather than leaving the reader to guess.
//   Serial.printf("onboard RGB on GPIO%u, cycling red/green/blue\n",
//                 PIN_LED_ONBOARD_RGB);
// }

// // Called over and over by the core for as long as the board is powered, each
// // time setup() has finished. Takes no arguments and returns nothing.
// void loop() {
//   // "static" makes this keep its value between calls instead of resetting to 0
//   // each time round, which is what lets it count. uint32_t holds up to about
//   // 4.29 billion ticks before wrapping to zero.
//   static uint32_t ticks = 0;

//   // Picks this tick's colour. The % (modulo) operator gives the remainder of
//   // ticks divided by the array length, producing 0,1,2,3,0,1,2,3... so the
//   // index always stays inside the array. Bound as a const reference to read
//   // the entry in place rather than copying it.
//   const Color &c = ONBOARD_SEQUENCE[ticks % ONBOARD_SEQUENCE_LEN];
//   // Pushes that colour to the LED. One argument: the Color chosen above.
//   setOnboardColor(c);

//   // Logs the tick. Three placeholders, three arguments:
//   //   %lu  ++ticks   pre-increment, so it adds 1 first and prints the new
//   //                  value: the first line reads "tick 1", not "tick 0"
//   //   %lu  millis()  no arguments; returns milliseconds since boot, wrapping
//   //                  after roughly 49.7 days
//   //   %s   c.name    the colour label from the struct
//   // %lu is the specifier for an unsigned long.
//   Serial.printf("tick %lu  uptime %lu ms  onboard %s\n", ++ticks, millis(),
//                 c.name);
//   // Blocks for 1000 ms, setting the one-second pace of the whole loop. Nothing
//   // else can run on this core meanwhile, which is fine now but is exactly what
//   // the v2.0 button work will have to replace with a millis() comparison.
//   delay(1000);
// }
