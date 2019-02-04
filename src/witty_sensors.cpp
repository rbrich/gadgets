// Read LDR and Temperature sensors every five minutes,
// send values to TCP server (InfluxDB) over Wi-Fi connection.

#include "config.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Note that the pin numbers on PCB are the real numbers, eg. GPIO4 is 4, not D4
static const int pin_led = 2;
static const int pin_ldr = A0;
static const int pin_rgb_red = 15;
static const int pin_rgb_green = 12;
static const int pin_rgb_blue = 13;

int timer = 0;

OneWire temp_wire(4);
DallasTemperature temp_sensor(&temp_wire);
DeviceAddress temp_addr;


void setup()
{
    // Connect with: pio device monitor
    Serial.begin(9600);
    Serial.println();
    Serial.println("=== Setup ===");

    // LED + LDR pins
    pinMode(pin_led, OUTPUT);
    pinMode(pin_ldr, INPUT);
    pinMode(pin_rgb_red, OUTPUT);
    pinMode(pin_rgb_green, OUTPUT);
    pinMode(pin_rgb_blue, OUTPUT);

    // temperature sensor
    Serial.println("--- Temperature ---");
    temp_sensor.begin();
    Serial.print("Found ");
    Serial.print(temp_sensor.getDeviceCount(), DEC);
    Serial.println(" temperature sensors.");

    // report parasite power requirements
    Serial.print("Parasite power is: ");
    if (temp_sensor.isParasitePowerMode())
        Serial.println("ON");
    else
        Serial.println("OFF");

    if (temp_sensor.getAddress(temp_addr, 0)) {
        Serial.print("Device 0 Address: ");
        for (unsigned char ch : temp_addr) {
            if (ch < 16) Serial.print("0");
            Serial.print(ch, HEX);
        }
        Serial.println();
    } else
        Serial.println("Unable to find address for Device 0");

    Serial.print("Device 0 Resolution: ");
    // sensors.setResolution(insideThermometer, 12);
    Serial.print(temp_sensor.getResolution(temp_addr), DEC);
    Serial.println();

    // Setup Wi-Fi
    Serial.println("--- Wi-Fi ---");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("=== Loop ===");
}


void loop()
{
    // Tick
    timer++;

    // Reset LEDs
    digitalWrite(pin_led, HIGH);
    analogWrite(pin_rgb_red, 0);
    analogWrite(pin_rgb_green, 0);
    analogWrite(pin_rgb_blue, 0);
    delay(500);

    // Present remaining time using RGB diode
    // - The value range is 0 .. 1024 (we use only 0..59)
    // - Each minute, a color is smoothly lighten up
    // - Color changes each minute: Blue, Red, Magenta, Green, Yellow
    int t_secs = timer % 60;
    int t_mins = timer / 60;
    if (t_secs == 0) {
        Serial.print(" ");
        Serial.println(t_mins);
    } else {
        Serial.print(".");
        int value = t_secs;
        if (t_mins == 1 || t_mins == 2 || t_mins == 4)
            analogWrite(pin_rgb_red, value);
        if (t_mins == 3 || t_mins == 4)
            analogWrite(pin_rgb_green, value);
        if (t_mins == 0 || t_mins == 2)
            analogWrite(pin_rgb_blue, value);
    }
    delay(500);

    // Wait five minutes
    if (t_mins == 5 && t_secs == 0) {
        // Trigger the action
        timer = 0;
        digitalWrite(pin_led, LOW);
    } else {
        // Not yet
        return;
    }

    // ------------------------------------------------------------------------

    // Read light sensor
    int ldr_value = analogRead(pin_ldr);
    Serial.print("LDR: ");
    Serial.println(ldr_value);

    // Read temperature sensor
    temp_sensor.requestTemperaturesByAddress(temp_addr);
    float temp_celsius = temp_sensor.getTempC(temp_addr);
    Serial.print("Temperature: ");
    Serial.print(temp_celsius);
    Serial.println("°C");

    // Send values to InfluxDB:
    WiFiClient client;
    Serial.println();
    Serial.println("* Connecting to " DB_HOST " ...");
    if (client.connect(DB_HOST, DB_PORT)) {
        Serial.printf("* Connected (%s)\n", client.remoteIP().toString().c_str());
        Serial.println("* Sending data...");
        String data("ambient_light," DEVICE_TAGS " value=");
        data.concat(ldr_value);
        data.concat('\n');
        if (temp_celsius != DEVICE_DISCONNECTED_C) {
            data.concat("temperature," DEVICE_TAGS " value=");
            data.concat(temp_celsius);
            data.concat('\n');
        }
        client.printf(
                "POST /write?db=" DB_NAME " HTTP/1.1\r\n"
                "Host: " DB_HOST ":%d\r\n"
                "Connection: close\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                DB_PORT, data.length());
        client.print(data);

        Serial.println("* Waiting for response...");
        while (client.connected() || client.available()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client.stop();
        Serial.println("* Connection closed");
    } else {
        Serial.println("* Connection failed.");
        client.stop();
    }
}
