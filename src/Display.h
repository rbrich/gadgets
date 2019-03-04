// Display.h - created by Radek Brich on 2019-03-01

#ifndef GADGETS_DISPLAY_H
#define GADGETS_DISPLAY_H

#ifdef WITH_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 0  // GPIO0
#endif


class Display {
public:
    void begin();

    void clear();
    void display();

    // Status bar (line 0):
    void drawWifiIcon();
    void drawTimer(int seconds);
    void drawStar();

    // Text/value lines (1 .. 4):
    void drawText(int line, const char *text);
    void appendText(const char *text);
    void drawValue(int line, const char *format, double value);
    void appendValue(const char *format, double value);

private:
#ifdef WITH_OLED
    Adafruit_SSD1306 m_oled {OLED_RESET};
#endif
};


#endif // GADGETS_DISPLAY_H
