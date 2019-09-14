// Sweeper.h created on 2019-09-13, part of XCI toolkit
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

#ifndef GADGETS_SWEEPER_H
#define GADGETS_SWEEPER_H

#include <Servo.h>

class Sweeper {
public:
    Sweeper(int pin_servo, int pin_button) noexcept
        : m_pin_servo(pin_servo), m_pin_button(pin_button) {}

    void setup();

    bool check_button();
    void sweep();

private:
    Servo m_servo;

    int m_pin_servo;
    int m_pin_button;

    int pos = 0;    // variable to store the servo position
    int maxpos = 160;
    int step = 10;
    int DELAY = 15;
};

#endif // include guard
