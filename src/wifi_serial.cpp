#include "config.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>


static WiFiServer server(23);
static WiFiClient client;

constexpr size_t buffer_size = 1024;
static uint8_t buffer[buffer_size];


void forward_input(Stream& rstream, Stream& wstream)
{
    size_t i = 0;
    while (i < buffer_size) {
        int b = rstream.read();
        if (b == -1)
            break;
        buffer[i++] = (uint8_t) b;
    }
    if (i > 0) {
        wstream.write(buffer, i);
    }
}


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    digitalWrite(LED_BUILTIN, LOW);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        server.begin();
    }
    WiFiClient::setDefaultSync(true);
    digitalWrite(LED_BUILTIN, HIGH);
}


void loop()
{
    if (server.status() == CLOSED) {
        // Wi-Fi error, blink slowly
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(900);
        return;
    }

    if (client.connected()) {
        // pass data between client and serial
        digitalWrite(LED_BUILTIN, LOW);
        forward_input(client, Serial);
        forward_input(Serial, client);
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        // wait for a client to connect
        client = server.available();
    }
    delay(100);
}
