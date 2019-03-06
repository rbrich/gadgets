#include "config.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>


static WiFiServer server(23);
static WiFiClient client;

constexpr size_t buffer_size = 1024;
static uint8_t buffer[buffer_size];

// The buffer type in read() methods differ...
char* buffer_for_read(HardwareSerial&) { return (char*) buffer; }
uint8_t* buffer_for_read(WiFiClient&) { return buffer; }

template <typename TRead, typename TWrite>
void forward_stream(TRead& rstream, TWrite& wstream)
{
    auto avail = (size_t) rstream.available();
    if (avail > 0 && wstream.availableForWrite() >= avail) {
        if (avail > buffer_size)
            avail = buffer_size;
        size_t bytes_read = rstream.read(buffer_for_read(rstream), avail);
        size_t bytes_written = 0;
        while (bytes_read > bytes_written) {
            bytes_written += wstream.write(buffer + bytes_written,
                                           bytes_read - bytes_written);
        }
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
    WiFiClient::setDefaultNoDelay(true);
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
        forward_stream(client, Serial);
        forward_stream(Serial, client);
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        // wait for a client to connect
        client = server.available();
    }
}
