// Sensor.cpp - created by Radek Brich on 2019-03-05

#include "config.h"
#include "Sensor.h"


Sensor* Sensor::m_first = nullptr;


#ifdef WITH_LDR

static LDRSensor s_ldr_sensor;

LDRSensor::LDRSensor()
{
    Sensor::add(&s_ldr_sensor);
}

void LDRSensor::output_to_stream(Stream& stream)
{
    stream.print("LDR: ");
    stream.println(m_value);
}


void LDRSensor::output_to_database(String& query)
{
    query.concat("ambient_light,sensor=LDR," DEVICE_TAGS " value=");
    query.concat(m_value);
    query.concat('\n');
}

#endif


#ifdef WITH_DALLAS_TEMP

static DallasTempSensor s_dallas_sensor;


DallasTempSensor::DallasTempSensor()
{
    Sensor::add(&s_dallas_sensor);
}

void DallasTempSensor::setup()
{
    m_sensor.begin();

    Serial.print("[dallas] Found ");
    Serial.print(m_sensor.getDeviceCount(), DEC);
    Serial.println(" temperature sensors.");

    // report parasite power requirements
    Serial.print("[dallas] Parasite power is: ");
    if (m_sensor.isParasitePowerMode())
        Serial.println("ON");
    else
        Serial.println("OFF");

    if (m_sensor.getAddress(m_addr, 0)) {
        Serial.print("[dallas] Device 0 Address: ");
        for (unsigned char ch : m_addr) {
            if (ch < 16) Serial.print("0");
            Serial.print(ch, HEX);
        }
        Serial.println();
    } else
        Serial.println("[dallas] Unable to find address for Device 0");

    Serial.print("[dallas] Device 0 Resolution: ");
    // sensors.setResolution(insideThermometer, 12);
    Serial.print(m_sensor.getResolution(m_addr), DEC);
    Serial.println();
}


void DallasTempSensor::read()
{
    m_sensor.requestTemperaturesByAddress(m_addr);
    m_value = m_sensor.getTempC(m_addr);
    if (m_value == DEVICE_DISCONNECTED_C)
        m_value = 0.f;
}


void DallasTempSensor::output_to_stream(Stream &stream)
{
    stream.print("[dallas] Temperature: ");
    stream.print(m_value);
    stream.println("°C");
}


void DallasTempSensor::output_to_database(String &query)
{
    if (m_value != 0.f) {
        query.concat("temperature,sensor=Dallas," DEVICE_TAGS " value=");
        query.concat(m_value);
        query.concat('\n');
    }
}

#endif


#ifdef WITH_SHT30

static SHT30Sensor s_sht30_sensor;


SHT30Sensor::SHT30Sensor()
{
    Sensor::add(&s_sht30_sensor);
}


void SHT30Sensor::read()
{
    if (m_sht30.get() != 0) {
        Serial.println("[SHT30] Error");
    }
}


void SHT30Sensor::output_to_stream(Stream &stream)
{
    stream.print("[SHT30] Temperature: ");
    stream.print(m_sht30.cTemp);
    stream.println("°C");
    stream.print("[SHT30] Humidity: ");
    stream.print(m_sht30.humidity);
    stream.println("%");
}


void SHT30Sensor::output_to_database(String &query)
{
#ifndef WITH_BMP280
    if (m_sht30.cTemp != 0.f) {
        query.concat("temperature,sensor=BMP280," DEVICE_TAGS " value=");
        query.concat(m_sht30.cTemp);
        query.concat('\n');
    }
#endif

    if (m_sht30.humidity != 0.f) {
        query.concat("humidity,sensor=BMP280," DEVICE_TAGS " value=");
        query.concat(m_sht30.humidity);
        query.concat('\n');
    }
}


void SHT30Sensor::output_to_display(Display &display)
{
#ifndef WITH_BMP280
    display.drawValue(1, "%.2f degC", m_sht30.cTemp);
#endif
    display.drawValue(2, "%.2f relH", m_sht30.humidity);
}

#endif


#ifdef WITH_BMP280

static BMP280Sensor s_bmp280_sensor;

BMP280Sensor::BMP280Sensor()
{
    Sensor::add(&s_bmp280_sensor);
}


void BMP280Sensor::setup()
{
    if (m_bmp.begin()) {
        Serial.println("[BMP280] Found.");
        m_bmp.setOversampling(4);
    } else {
        Serial.println("[BMP280] Error.");
    }
}


void BMP280Sensor::read()
{
    auto result = m_bmp.startMeasurment();
    if (result != 0) {
        delay(result);
        result = m_bmp.getTemperatureAndPressure(m_temperature, m_pressure);
#ifdef BMP280_TEMP_CORRECTION
        m_temperature += BMP280_TEMP_CORRECTION;
#endif
        if (result == 0) {
            m_temperature = 0;
            m_pressure = 0;
        }
    }
}


void BMP280Sensor::output_to_stream(Stream &stream)
{
    stream.print("[BMP280] Temperature: ");
    stream.print(m_temperature);
    stream.println("°C");
    stream.print("[BMP280] Pressure: ");
    stream.print(m_pressure);
    stream.println(" hPa");
}


void BMP280Sensor::output_to_database(String &query)
{
    if (m_temperature != 0) {
        query.concat("temperature,sensor=BMP280," DEVICE_TAGS " value=");
        query.concat(m_temperature);
        query.concat('\n');
    }

    if (m_pressure != 0) {
        query.concat("pressure,sensor=BMP280," DEVICE_TAGS " value=");
        query.concat(m_pressure);
        query.concat('\n');
    }
}


void BMP280Sensor::output_to_display(Display &display)
{
    display.drawValue(1, "%.2f degC", m_temperature);
    display.drawValue(3, "%.1f hPa", m_pressure);
}

#endif


#ifdef WITH_MOIST


static MoistSensor s_moist_sensor;

MoistSensor::MoistSensor()
{
    Sensor::add(&s_moist_sensor);
}


void MoistSensor::setup()
{
    pinMode(m_pin, INPUT);
    pinMode(m_pin_digi, INPUT);

#ifdef WITH_CUSTOM_LED
    pinMode(D0, OUTPUT);
#endif
}


void MoistSensor::read()
{
    // Tested values:
    // - emerged in water: 256 (100%)
    // - completely dry (capacitive sensor): 715? (40%)
    // - completely dry (resistive sensor): 1024 (0%)
    // Output range is 0.0 (dry) - 100.0 (emerged in water)
    m_value = (1024.f - analogRead(m_pin)) / 7.68f;
    m_over_threshold = digitalRead(m_pin_digi);
}


void MoistSensor::output_to_stream(Stream &stream)
{
    Serial.print("[soil] Moisture: ");
    Serial.print(m_value);
    Serial.print(" (threshold: ");
    Serial.print(m_over_threshold);
    Serial.println(")");
}


void MoistSensor::output_to_database(String &query)
{
    query.concat("moisture,sensor=Generic" DEVICE_TAGS " value=");
    query.concat(m_value);
    query.concat('\n');
}


void MoistSensor::output_to_display(Display &display)
{
    display.drawValue(4, "%.1f soilM", m_value);
    if (m_over_threshold) {
        display.drawStar();
    }
#ifdef WITH_CUSTOM_LED
    // Light up custom LED when moisture goes under custom threshold
    digitalWrite(D0, (m_value < 25) ? HIGH : LOW);
#endif
}

#endif
