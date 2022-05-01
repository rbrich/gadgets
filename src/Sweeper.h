// Sweeper.h - created by Radek Brich on 2019-09-13

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

    int m_home_pos = 160;    // start in this pos and return here after sweep
    int m_sweep_pos = 0;
    int m_sweep_delay = 500;
};

#endif // include guard
