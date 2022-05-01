// HttpClient.h - created by Radek Brich on 2019-09-13

#ifndef GADGETS_HTTPCLIENT_H
#define GADGETS_HTTPCLIENT_H

#include "Display.h"
#include <ESP8266WiFi.h>
#include <functional>

class HttpClient {
public:
    explicit HttpClient(Display& display) : m_display(display) {}

    bool connect(const String& host, uint16_t port);
    bool reconnect();

    using XHdrCallback = std::function<void(const String& name, const String& value)>;
    using ContentCallback = std::function<void(const String& line)>;
    int query(const char *method, const char* url,
              const XHdrCallback& x_hdr_cb, const ContentCallback& cnt_cb);
    void post(const char* url, const String& data);

    void stop();

private:
    bool _connect();

private:
    WiFiClient m_client;
    Display& m_display;
    String m_host;
    uint16_t m_port = 0;
};

#endif // include guard
