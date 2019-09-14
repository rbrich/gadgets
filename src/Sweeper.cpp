// Sweeper.cpp created on 2019-09-13, part of XCI toolkit
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

void Sweeper::setup()

{
    pinMode(m_pin_button, INPUT);
    m_servo.attach(m_pin_servo);
}


bool Sweeper::check_button()
{
    // read the input pin:
    int buttonState = digitalRead(m_pin_button);
    // print out the state of the button:
    if (buttonState) {
        Serial.println("[sweeper btn pressed]");
    }
    return buttonState;
}


void Sweeper::sweep()
{
    Serial.println("* Sweep started");

    m_servo.write(0);
    delay(500);
    m_servo.write(160);

    Serial.println("* Sweep complete");
}
