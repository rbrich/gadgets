// HttpClient.h created on 2019-09-13, part of XCI toolkit
// Copyright 2019 Radek Brich
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
