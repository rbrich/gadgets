// Read LDR and Temperature sensors every five minutes,
// send values to TCP server (InfluxDB) over Wi-Fi connection.

#include "config.h"
#include "Display.h"
#include "Sensor.h"
#include "HttpClient.h"

#ifdef WITH_SERVO
#include "Servo.h"
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>

// -----------------------------------------------------------------------------

#ifdef WITH_RGB
static const int pin_rgb_red = 15;
static const int pin_rgb_green = 12;
static const int pin_rgb_blue = 13;
#endif


static Display display;
static int timer = 0;

void setup()
{
    // Connect with: pio device monitor
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println();
    Serial.println("=== Setup ===");

    // LED pins
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef WITH_RGB
    pinMode(pin_rgb_red, OUTPUT);
    pinMode(pin_rgb_green, OUTPUT);
    pinMode(pin_rgb_blue, OUTPUT);
#endif

    Sensor::for_each([](Sensor& sensor) {
        sensor.setup();
    });

    display.begin();

    // Setup Wi-Fi
    Serial.println("--- Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    //wifi_set_sleep_type(LIGHT_SLEEP_T);

    Serial.println("=== Loop ===");
}


void loop()
{
    // Reset LEDs
    digitalWrite(LED_BUILTIN, HIGH);
#ifdef WITH_RGB
    analogWrite(pin_rgb_red, 0);
    analogWrite(pin_rgb_green, 0);
    analogWrite(pin_rgb_blue, 0);
#endif
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
#ifdef WITH_RGB
        int value = t_secs;
        if (t_mins == 1 || t_mins == 2 || t_mins == 4)
            analogWrite(pin_rgb_red, value);
        if (t_mins == 3 || t_mins == 4)
            analogWrite(pin_rgb_green, value);
        if (t_mins == 0 || t_mins == 2)
            analogWrite(pin_rgb_blue, value);
#endif
    }
    display.clear();
    if (WiFi.isConnected()) {
        display.drawWifiIcon();
    }
    display.drawTimer(SEND_INTERVAL - timer);

    Sensor::for_each([](Sensor& sensor) {
        sensor.read();
        sensor.output_to_display(display);
    });

    display.display();
    delay(500);

    // Wait N minutes
    if (timer == SEND_INTERVAL) {
        // Trigger the action
        timer = 0;
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        // Not yet
        timer++;
        return;
    }

    // ------------------------------------------------------------------------

    display.clear();
    display.drawText(1, "Conn ");
    display.display();

    // Need Wi-Fi
    if (!WiFi.isConnected())
        return;
    Serial.print("Wi-Fi connected, IP address: ");
    Serial.println(WiFi.localIP());

    display.appendText("OK");
    display.display();

    Sensor::for_each([](Sensor& sensor) {
        sensor.read();
        sensor.output_to_stream(Serial);
    });

    Serial.println();

    // Contact C&C server
    HttpClient client(display);
    if (client.connect(DB_HOST, DB_PORT)) {
        Serial.println("* Checking commands...");

        int seq = -1;
        bool device_checked = false;
        bool cmd_feed = false;
        auto status = client.query("GET", "/control/" DEVICE_NAME,
                [&](const String& name, const String& value) {
                    if (name == "X-Seq")
                        seq = (int) value.toInt();
                    else if (name == "X-Device" && value.equals(DEVICE_NAME))
                        device_checked = true;
                    else
                        Serial.printf("hdr: %s: %s\n", name.c_str(), value.c_str());
                },
                [&](const String& line) {
                    if (line == "feed")
                        cmd_feed = true;
                    else
                        Serial.println("line: " + line);
                });
        client.stop();
        Serial.println("* Status: " + String(status));
        Serial.flush();

        if (status == 200) {
            if (!device_checked || seq == -1) {
                Serial.printf("* Error: device_checked=%d seq=%d\n",
                              device_checked, seq);
                return;
            }

            if (cmd_feed) {
                Serial.println("Feed!");
            }

            if (!client.reconnect())
                return;

            Serial.println("* Sending ack...");
            client.query("DELETE", ("/control/" DEVICE_NAME "?seq=" + String(seq)).c_str(),
                     [&](const String& name, const String& value) {
                         Serial.printf("hdr: %s=%s", name.c_str(), value.c_str());
                     },
                     [](const String& line) {
                         Serial.println("cnt: " + line);
                     });
            client.stop();
        }
        Serial.flush();

#ifndef NO_SENSORS
        // Send values to InfluxDB:
        if (!client.reconnect())
            return;

        Serial.println("* Sending data...");
        String data;

        Sensor::for_each([&data](Sensor& sensor) {
            sensor.output_to_database(data);
        });

        Serial.println(data);

        client.post("/write?db=" DB_NAME, data);
        client.stop();
        display.appendText("OK");
        display.display();
#endif
    } else {
        client.stop();
        display.appendText("FAIL");
        display.display();
        delay(1000);
    }
}
