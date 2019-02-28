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

#ifdef WITH_BMP280
#include <Wire.h>
#include <BMP280.h>
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

#ifdef WITH_MOIST
static const int pin_moist = A0;
static const int pin_moist_digi = D3;
#endif

#ifdef WITH_BMP280
static BMP280 bmp;
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
#ifdef WITH_CUSTOM_LED
    pinMode(D0, OUTPUT);
#endif
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

#ifdef WITH_MOIST
    pinMode(pin_moist, INPUT);
    pinMode(pin_moist_digi, INPUT);
#endif

#ifdef WITH_BMP280
    Serial.println("--- BMP280 ---");
    if (bmp.begin()) {
        Serial.println("Found.");
        bmp.setOversampling(4);
    } else {
        Serial.println("Failed.");
    }
#endif

#ifdef WITH_OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.dim(1);
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
    display.clearDisplay();
    if (WiFi.isConnected()) {
        display.drawXBitmap(0, 0, WIFI_icon_bits, WIFI_icon_width, WIFI_icon_height, WHITE);
    }

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setTextWrap(0);

    {
        int r = SEND_INTERVAL - timer;
        display.setCursor(14, 2);
        display.printf("T-%d:%02d\n", r / 60, r % 60);
    }

#ifdef WITH_SHT30
    sht30.get();
#ifndef WITH_BMP280
    display.setCursor(0, 12);
    display.print("Temp ");
    display.print(sht30.cTemp);
#endif
    display.setCursor(0, 22);
    display.printf("%.2f relH", sht30.humidity);
#endif

#ifdef WITH_MOIST
    {
        float value = (1024 - analogRead(pin_moist)) / 7.68f;
        int over_threshold = digitalRead(pin_moist_digi);
        display.setCursor(0, 40);
        display.printf("%.1f soilM", value);
        if (over_threshold) {
            display.setCursor(54, 2);
            display.print("*");
        }
#ifdef WITH_CUSTOM_LED
        // Light up custom LED when moisture goes under custom threshold
        digitalWrite(D0, (value < 25) ? HIGH : LOW);
#endif
    }
#endif

#ifdef WITH_BMP280
    {
        double temperature = 0, pressure = 0;
        auto result = bmp.startMeasurment();
        if (result != 0) {
            delay(result);
            result = bmp.getTemperatureAndPressure(temperature, pressure);
            if (result != 0) {
                display.setCursor(0, 13);
                display.printf("%.2f", temperature);
#ifdef WITH_SHT30
                // Show also SHT30 temperature when available
                // (the SHT30 shield is in vicinity of ESP board and should be hotter)
                display.printf("/%.1f", sht30.cTemp);
#else
                display.print(" degC");
#endif
                display.setCursor(0, 31);
                display.printf("%.1f hPa", pressure);
            }
        }
    }
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

    double temp_celsius = 0;
#ifdef WITH_DALLAS_TEMP
    // Read temperature sensor
    temp_sensor.requestTemperaturesByAddress(temp_addr);
    temp_celsius = temp_sensor.getTempC(temp_addr);
    if (temp_celsius == DEVICE_DISCONNECTED_C)
        temp_celsius = 0;
    Serial.print("[dallas] Temperature: ");
    Serial.print(temp_celsius);
    Serial.println("°C");
#endif

#ifdef WITH_SHT30
    // Read temperature/humidity sensor
    if (sht30.get() != 0) {
        Serial.println("sht30: Error");
    }
    temp_celsius = sht30.cTemp;
    float humidity = sht30.humidity;
    Serial.print("[SHT30] Temperature: ");
    Serial.print(temp_celsius);
    Serial.println("°C");
    Serial.print("[SHT30] Relative Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
#endif

#ifdef WITH_BMP280
    double pressure = 0;
    auto result = bmp.startMeasurment();
    if (result != 0) {
        delay(result);
        result = bmp.getTemperatureAndPressure(temp_celsius, pressure);
        if (result != 0) {
            Serial.print("[BMP280] Temperature: ");
            Serial.print(temp_celsius);
            Serial.println("°C");
            Serial.print("[BMP280] Pressure: ");
            Serial.print(pressure);
            Serial.println(" hPa");
        }
    }
#endif

#ifdef WITH_MOIST
    // Tested values:
    // - emerged in water: 256
    // - completely dry: 1024
    // Output range is 0.0 (dry) - 100.0 (emerged in water)
    float moisture = (1024 - analogRead(pin_moist)) / 7.68f;
    Serial.print("[soil] Moisture: ");
    Serial.print(moisture);
    Serial.print(" (threshold: ");
    int moisture_threshold = digitalRead(pin_moist_digi);
    Serial.print(moisture_threshold);
    Serial.println(")");
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

#if defined(WITH_DALLAS_TEMP) || defined(WITH_SHT30) || defined(WITH_BMP280)
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

#ifdef WITH_BMP280
        if (pressure != 0) {
            data.concat("pressure," DEVICE_TAGS " value=");
            data.concat(pressure);
            data.concat('\n');
        }
#endif

#ifdef WITH_MOIST
        data.concat("moisture," DEVICE_TAGS " value=");
        data.concat(moisture);
        data.concat('\n');
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
        display.print("Recv ");
        display.display();
#endif

        Serial.println("* Waiting for response...");
        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client.stop();
        Serial.println("* Connection closed");
#ifdef WITH_OLED
        display.println("OK");
        display.display();
#endif
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