// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// https://github.com/milesburton/Arduino-Temperature-Control-Library

#include "temp_DS18B20.h"

#include <Arduino.h>


static byte addr[8];
static bool ready = false;


bool setup_temperature(OneWire& ds)
{
  Serial.println("[temp] Looking for DS18B20 temperature sensor...");
  ds.reset_search();
  if (!ds.search(addr)) {
    Serial.println("[temp] No more addresses.");
    Serial.println();
    ds.reset_search();
    return false;
  }

  Serial.print("[temp] Found ROM =");
  for (byte i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("[temp] CRC is not valid!");
    return false;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  if (addr[0] != 0x28) {
    Serial.println("[temp] Device is not a DS18B20.");
    return false;
  }
  Serial.println("[temp]   Chip = DS18B20");

  ready = true;
  return true;
}


bool read_temperature(OneWire& ds, float& celsius)
{
    if (!ready) {
        Serial.print("[temp]   Not set up.");
        return false;
    }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  byte data[12];
  Serial.print("[temp]   Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for (byte i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  /*DS18B20*/ {
    byte cfg = (data[4] & 0x60);
    Serial.print("[temp]   Cfg = ");
    Serial.print(cfg, HEX);
    Serial.println();
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0f;
  Serial.print("[temp]   Temperature = ");
  Serial.print(celsius);
  Serial.println("°C");
  return true;
}
