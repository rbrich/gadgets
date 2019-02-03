#ifndef TEMP_DS15B20_H_INCLUDED
#define TEMP_DS15B20_H_INCLUDED

#include <OneWire.h>

bool setup_temperature(OneWire& ds);
bool read_temperature(OneWire& ds, float& celsius);

#endif
