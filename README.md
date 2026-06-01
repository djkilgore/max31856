# Maxim 31856 Thermocouple IC Driver

* Initializes a MAX31856 with Type-K Thermocouple Settings.
* Starts a FreeRTOS task to monitor Thermocouple Temperature and Faults.

### Notes on Register Reads and Writes

The private functions which read and write the registers of the chip are a bit of a hack, they send each byte in a seperate transaction and manually control the SPI CS line.

From what I can tell the MAX31856 expects dummy bytes or "dead air" in between each byte sent and I wasn't able to replicate this with the ESP-IDF spi-master driver but I am probably doing something wrong. 

Pull requests are welcome...

After two days with a Logic Analyzer hooked up, the only difference I could see was this delay or clock pulse between each byte on a working implementation.

[Logic Analyzer Screenshot](https://imgur.com/a/GD3RvAK)

### Notes on Fault Register Reads

`read_fast_register()` does a single 16-bit SPI transaction. It writes the address byte, then a dummy `0xFF` to clock in the response. The two-transaction approach used by `read_register()` inserts inter-byte gaps that some chips need to latch a stable register value, but the fault register can be read in one transaction without any gap.

Earlier versions of this function returned `rx_data[0]`. That byte is junk on the MAX31856. The chip cannot drive MISO with the response until it has decoded the address byte, so the first 8 clock cycles read whatever MISO floated to. The actual register value arrives during the dummy byte and lives in `rx_data[1]`.

The function was fixed after a downstream consumer saw the MAX31856 type-K saturation value (1372 °C) on their display instead of the expected NO COIL fault. They traced it to junk coming out of `read_fast_register` and verified the fix on hardware. The earlier "weird hardware issue" framing was wrong. This is standard SPI register-read behaviour.
