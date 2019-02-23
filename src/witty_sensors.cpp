// Read LDR and Temperature sensors every five minutes,
// send values to TCP server (InfluxDB) over Wi-Fi connection.

#include "config.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifdef WITH_DALLAS_TEMP
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#ifdef WITH_SHT30
#include <WEMOS_SHT3X.h>
#endif

#ifdef WITH_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

// -----------------------------------------------------------------------------

#ifdef WITH_RGB
static const int pin_rgb_red = 15;
static const int pin_rgb_green = 12;
static const int pin_rgb_blue = 13;
#endif

#ifdef WITH_LDR
static const int pin_ldr = A0;
#endif

#ifdef WITH_DALLAS_TEMP
static OneWire temp_wire(4);
static DallasTemperature temp_sensor(&temp_wire);
static DeviceAddress temp_addr;
#endif

#ifdef WITH_SHT30
static SHT3X sht30;
#endif

#ifdef WITH_OLED
#define OLED_RESET 0  // GPIO0
static Adafruit_SSD1306 display(OLED_RESET);
// WIFI_icon.xbm
#define WIFI_icon_width 10
#define WIFI_icon_height 10
static unsigned char WIFI_icon_bits[] = {
        0x1c, 0x00, 0x60, 0x00, 0x8c, 0x00, 0x30, 0x01, 0x44, 0x01, 0x58, 0x02,
        0x92, 0x02, 0xa6, 0x02, 0x0f, 0x00, 0x03, 0x00 };
#endif

static int timer = 0;

void setup()
{
    // Connect with: pio device monitor
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Setup ===");

    // LED pins
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef WITH_RGB
    pinMode(pin_rgb_red, OUTPUT);
    pinMode(pin_rgb_green, OUTPUT);
    pinMode(pin_rgb_blue, OUTPUT);
#endif

#ifdef WITH_LDR
    pinMode(pin_ldr, INPUT);
#endif

#ifdef WITH_DALLAS_TEMP
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
#endif

#ifdef WITH_OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
#endif

    // Setup Wi-Fi
    Serial.println("--- Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    wifi_set_sleep_type(LIGHT_SLEEP_T);

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
#ifdef WITH_OLED
    display.clearDisplay();
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
#ifdef WITH_OLED
    if (WiFi.isConnected()) {
        display.drawXBitmap(0, 0, WIFI_icon_bits, WIFI_icon_width, WIFI_icon_height, WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(WHITE);

    {
        int r = SEND_INTERVAL - timer;
        display.setCursor(14, 2);
        display.printf("T-%d:%02d\n", r / 60, r % 60);
    }

#ifdef WITH_SHT30
    sht30.get();
    display.setCursor(0, 12);
    display.print("Temp ");
    display.print(sht30.cTemp);
    display.setCursor(0, 22);
    display.print("Humi ");
    display.print(sht30.humidity);
#endif

    display.display();
#endif
    delay(500);

    // Wait five minutes
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

#ifdef WITH_OLED
    display.clearDisplay();
    display.setCursor(0, 12);
    display.print("Conn ");
    display.display();
#endif

    // Need Wi-Fi
    if (!WiFi.isConnected())
        return;
    Serial.print("Wi-Fi connected, IP address: ");
    Serial.println(WiFi.localIP());

#ifdef WITH_OLED
    display.println("OK");
    display.display();
#endif

#ifdef WITH_LDR
    // Read light sensor
    int ldr_value = analogRead(pin_ldr);
    Serial.print("LDR: ");
    Serial.println(ldr_value);
#endif

#ifdef WITH_DALLAS_TEMP
    // Read temperature sensor
    temp_sensor.requestTemperaturesByAddress(temp_addr);
    float temp_celsius = temp_sensor.getTempC(temp_addr);
    if (temp_celsius == DEVICE_DISCONNECTED_C)
        temp_celsius = 0;
    Serial.print("Temperature: ");
    Serial.print(temp_celsius);
    Serial.println("°C");
#endif

#ifdef WITH_SHT30
    // Read temperature/humidity sensor
    if (sht30.get() != 0) {
        Serial.println("sht30: Error");
    }
    float temp_celsius = sht30.cTemp;
    float humidity = sht30.humidity;
    Serial.print("Temperature: ");
    Serial.print(temp_celsius);
    Serial.println("°C");
    Serial.print("Relative Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
#endif

    // Send values to InfluxDB:
    WiFiClient client;
    Serial.println();
    Serial.println("* Connecting to " DB_HOST " ...");
#ifdef WITH_OLED
    display.setCursor(0, 22);
    display.print("Send ");
    display.display();
#endif
    if (client.connect(DB_HOST, DB_PORT)) {
        Serial.printf("* Connected (%s)\n", client.remoteIP().toString().c_str());
        Serial.println("* Sending data...");

        String data;

#ifdef WITH_LDR
        data.concat("ambient_light," DEVICE_TAGS " value=");
        data.concat(ldr_value);
        data.concat('\n');
#endif

#if defined(WITH_DALLAS_TEMP) || defined(WITH_SHT30)
        if (temp_celsius != 0) {
            data.concat("temperature," DEVICE_TAGS " value=");
            data.concat(temp_celsius);
            data.concat('\n');
        }
#endif

#ifdef WITH_SHT30
        if (humidity != 0) {
            data.concat("humidity," DEVICE_TAGS " value=");
            data.concat(humidity);
            data.concat('\n');
        }
#endif

        Serial.println(data);

        client.printf(
                "POST /write?db=" DB_NAME " HTTP/1.1\r\n"
                "Host: " DB_HOST ":%d\r\n"
                "Connection: close\r\n"
                "Content-Type: text/plain; charset=utf-8\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                DB_PORT, data.length());
        client.print(data);

#ifdef WITH_OLED
        display.println("OK");
        display.setCursor(0, 32);
        display.print("Recv");
        display.display();
#endif

        Serial.println("* Waiting for response...");
        int t = 0;
        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
#ifdef WITH_OLED
            if (t < display.width() - 30) {
                display.fillRect(30, 32, uint16_t(t + 1), 8, WHITE);
                display.display();
                delay(1);
                ++t;
            }
#endif
        }
        client.stop();
        Serial.println("* Connection closed");
    } else {
        Serial.println("* Connection failed.");
        client.stop();
#ifdef WITH_OLED
        display.println("FAIL");
        display.display();
        delay(1000);
#endif
    }
}
