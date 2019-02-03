// Read LDR and Temperature sensors every five minutes,
// send values to TCP server (InfluxDB) over Wi-Fi connection.

#include "temp_DS18B20.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

// Note that the pin numbers on PCB are the real numbers, eg. GPIO4 is 4, not D4
static const int pin_led = 2;
static const int pin_ldr = A0;
static const int pin_rgb_red = 15;
static const int pin_rgb_green = 12;
static const int pin_rgb_blue = 13;

int timer = 0;

OneWire wire_temp(4);


void setup()
{
  pinMode(pin_led, OUTPUT);
  pinMode(pin_ldr, INPUT);
  pinMode(pin_rgb_red, OUTPUT);
  pinMode(pin_rgb_green, OUTPUT);
  pinMode(pin_rgb_blue, OUTPUT);

  setup_temperature(wire_temp);

  // pio device monitor
  Serial.begin(9600);
  Serial.println("Begin");

  WiFi.begin("WIFI", "PASSWORD");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
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
    float temperature;
    bool temp_ok = read_temperature(wire_temp, temperature);

    // Send values to server
    WiFiClient client;
    const char* host = "server.lan";
    uint16_t port = 8086;
    Serial.printf("\n* Connecting to %s ...\n", host);
    if (client.connect(host, port)) {
        Serial.printf("* Connected (%s)\n", client.remoteIP().toString().c_str());
        Serial.println("* Sending data...");
        String data("ambient_light,device=ufo1,location=kitchen value=");
        data.concat(ldr_value);
        data.concat('\n');
        if (temp_ok) {
            data.concat("temperature,device=ufo1,location=kitchen value=");
            data.concat(temperature);
            data.concat('\n');
        }
        client.printf(
                "POST /write?db=sensors HTTP/1.1\r\n"
                "Host: %s:%d\r\n"
                "Connection: close\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                host, port, data.length());
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
