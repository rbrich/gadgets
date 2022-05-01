// Sweeper.cpp - created by Radek Brich on 2019-09-13

#include "Sweeper.h"
#include <Arduino.h>

void Sweeper::setup()

{
    pinMode(m_pin_button, INPUT);
    m_servo.attach(m_pin_servo);
    m_servo.write(m_home_pos);
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

    m_servo.write(m_sweep_pos);
    delay(m_sweep_delay);
    m_servo.write(m_home_pos);
    delay(m_sweep_delay);

    Serial.println("* Sweep complete");
}
