// Display.cpp - created by Radek Brich on 2019-03-01

#include "Display.h"
#include <stdarg.h>

#ifdef WITH_OLED

// WIFI_icon.xbm
#define WIFI_icon_width 10
#define WIFI_icon_height 10
static unsigned char WIFI_icon_bits[] = {
        0x1c, 0x00, 0x60, 0x00, 0x8c, 0x00, 0x30, 0x01, 0x44, 0x01, 0x58, 0x02,
        0x92, 0x02, 0xa6, 0x02, 0x0f, 0x00, 0x03, 0x00 };


void Display::begin()
{
    m_oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    m_oled.dim(1);
    m_oled.setTextSize(1);
    m_oled.setTextColor(WHITE);
    m_oled.setTextWrap(0);
    m_oled.display();
}


void Display::clear()
{
    m_oled.clearDisplay();
}


void Display::display()
{
    m_oled.display();
}


void Display::drawWifiIcon()
{
    m_oled.drawXBitmap(0, 0, WIFI_icon_bits, WIFI_icon_width, WIFI_icon_height, WHITE);
}


void Display::drawTimer(int seconds)
{
    m_oled.setCursor(14, 2);
    m_oled.printf("T-%d:%02d\n", seconds / 60, seconds % 60);
}


void Display::drawStar()
{
    m_oled.setCursor(54, 2);
    m_oled.print("*");
}


void Display::drawText(int line, const char *text)
{
    m_oled.setCursor(0, line * 9 + 4);
    m_oled.print(text);
}


void Display::appendText(const char *text)
{
    m_oled.print(text);
}


void Display::drawValue(int line, const char *format, double value)
{
    m_oled.setCursor(0, line * 9 + 4);
    m_oled.printf(format, value);
}


void Display::appendValue(const char *format, double value)
{
    m_oled.printf(format, value);
}


#else

void Display::begin() {}
void Display::clear() {}
void Display::drawWifiIcon() {}
void Display::drawTimer(int seconds) {}
void Display::drawText(int line, const char *text) {}
void Display::appendText(const char *text) {}
void Display::drawValue(int line, const char *format, double value) {}
void Display::appendValue(const char *format, double value) {}

#endif
