// fish_feeder.cpp - created by Radek Brich on 2019-09-14

#include "Sweeper.h"
#include <Arduino.h>

static Sweeper sweeper(9, 2);


void setup()
{
    // Connect with: pio device monitor
    Serial.begin(57600);
    while (!Serial)
        ;
    Serial.println();
    Serial.println("=== Setup ===");

    // LED pins
    pinMode(LED_BUILTIN, OUTPUT);

    sweeper.setup();

    Serial.println("=== Loop ===");
}


void loop()
{
    // Reset LEDs
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);

    if (sweeper.check_button()) {
        digitalWrite(LED_BUILTIN, LOW);
        sweeper.sweep();
    }
}
