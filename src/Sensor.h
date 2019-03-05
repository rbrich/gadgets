// Sensor.h - created by Radek Brich on 2019-03-05

#ifndef GADGETS_SENSOR_H
#define GADGETS_SENSOR_H

#include "Display.h"

#ifdef WITH_DALLAS_TEMP
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#ifdef WITH_SHT30
#include <WEMOS_SHT3X.h>
#endif

#ifdef WITH_BMP280
#include <Wire.h>
#include <BMP280.h>
#endif


class Sensor {
public:
    // PUBLIC INTERFACE

    // initial setup
    virtual void setup() = 0;

    // read the sensor value
    virtual void read() = 0;

    // print the value into stream
    // - usage: `print_value(Serial)`
    // - should print descriptive text: `[LDR] value: 956`
    // - this method should append one or more lines (do not forget newlines)
    virtual void output_to_stream(Stream& stream) = 0;

    // print the value into database query string
    // - the format is: "<field>,<tags> value=<value>"
    // - for example: "temperature," DEVICE_TAGS " value=21.3\n"
    // - this method should append one or more lines (do not forget newlines)
    // - do not replace existing content!
    virtual void output_to_database(String& query) = 0;

    // print the value to the display
    virtual void output_to_display(Display& display) {}

    // INSTANCE REGISTRY

    template <typename F>
    static void for_each(const F& fn) {
        Sensor* it = m_first;
        while (it != nullptr) {
            fn(*it);
            it = it->m_next;
        }
    }

protected:
    static void add(Sensor* sensor) {
        sensor->m_next = m_first;
        m_first = sensor;
    }

private:
    Sensor* m_next = nullptr;
    static Sensor* m_first;
};


#ifdef WITH_LDR
// LDR - Light-dependent resistor
class LDRSensor final: public Sensor {
public:
    LDRSensor() noexcept;
    void setup() override { pinMode(m_pin, INPUT); }
    void read() override { m_value = analogRead(m_pin); }
    void output_to_stream(Stream& stream) override;
    void output_to_database(String& query) override;

private:
    static constexpr int m_pin = A0;
    int m_value = 0;
};
#endif


#ifdef WITH_DALLAS_TEMP
// Dallas temperature sensor
class DallasTempSensor final: public Sensor {
public:
    DallasTempSensor() noexcept;
    void setup() override;
    void read() override;
    void output_to_stream(Stream& stream) override;
    void output_to_database(String& query) override;

private:
    static constexpr int m_pin = D2;  // GPIO4
    OneWire m_wire {m_pin};
    DeviceAddress m_addr;
    DallasTemperature m_sensor {&m_wire};
    float m_value = 0.f;
};
#endif


#ifdef WITH_SHT30
// SHT30 temperature + humidity sensor
class SHT30Sensor final: public Sensor {
public:
    SHT30Sensor() noexcept;
    void setup() override {}
    void read() override;
    void output_to_stream(Stream& stream) override;
    void output_to_database(String& query) override;
    void output_to_display(Display& display) override;

private:
    SHT3X m_sht30;
};
#endif


#ifdef WITH_BMP280
// BMP280 temperature + pressure sensor
class BMP280Sensor final: public Sensor {
public:
    BMP280Sensor() noexcept;
    void setup() override;
    void read() override;
    void output_to_stream(Stream& stream) override;
    void output_to_database(String& query) override;
    void output_to_display(Display& display) override;

private:
    BMP280 m_bmp;
    double m_temperature = 0;
    double m_pressure = 0;
};
#endif


#ifdef WITH_MOIST
// Generic soil moisture sensor
class MoistSensor final: public Sensor {
public:
    MoistSensor() noexcept;
    void setup() override;
    void read() override;
    void output_to_stream(Stream& stream) override;
    void output_to_database(String& query) override;
    void output_to_display(Display& display) override;

private:
    static constexpr int m_pin = A0;
    static constexpr int m_pin_digi = D3;
    float m_value = 0.f;
    int m_over_threshold = 0;
};
#endif

#endif // GADGETS_SENSOR_H
