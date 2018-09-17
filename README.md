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

After I finished the library, I had to go back and add a read_fast_register() function because the Fault register works like it *should* in a single transaction and was returning bad data with the hack.

I am going to assume this is some weird hardware issue with this chip until further notice or someone else submits a patch.
