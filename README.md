# Maxim 31856 Thermocouple IC Example

Initializes a Max31856 with Type-K Thermocouple.
Starts a FreeRTOS task to monitor Thermocouple Temperature.

The register read and write functions are a bit of hack, I couldn't get them working in a single transaction.

From what I can tell the Max31856 expects dummy bytes or "dead air" in between each byte sent. 
After two days with a Logic Analyzer hooked up, I decided sending each byte in its own transaction was good enough for now.

https://imgur.com/a/GD3RvAK

After I finished the library, I had to go back and add a read_fast_register() function because the Fault register works like it *should*
I am going to assume this is some weird hardware issue with this chip until further notice.