// fish_feeder.cpp created on 2019-09-14, part of XCI toolkit
// Copyright 2019 Radek Brich
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
