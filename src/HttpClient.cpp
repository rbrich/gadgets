// HttpClient.cpp created on 2019-09-13, part of XCI toolkit
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

#include "HttpClient.h"


bool HttpClient::connect(const String& host, uint16_t port)
{
    m_host = host;
    m_port = port;
    return _connect();
}


bool HttpClient::reconnect()
{
    return _connect();
}


bool HttpClient::_connect()
{
    Serial.println("* Connecting to " + m_host + ":" + String(m_port) + " ...");
    m_display.drawText(2, "Send ");
    m_display.display();
    if (m_client.connect(m_host, m_port)) {
        Serial.printf("* Connected (%s)\n", m_client.remoteIP().toString().c_str());
        return true;
    } else {
        Serial.println("* Connection failed.");
        return false;
    }
}


int HttpClient:: query(const char *method, const char* url,
        const XHdrCallback& x_hdr_cb, const ContentCallback& cnt_cb)
{
    Serial.printf("* %s %s\n", method, url);
    m_client.printf(
            "%s %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Connection: close\r\n"
            "\r\n",
            method, url, m_host.c_str(), m_port);
    Serial.println("* Waiting for response...");

    int content_length = 0;
    bool awaiting_headers = true;
    int status = -1;
    while (m_client.connected() || m_client.available()) {
        if (m_client.available()) {
            String line = m_client.readStringUntil('\n');

            if (awaiting_headers) {
                line.trim();
                if (line.length() == 0) {
                    Serial.println("* End of headers");
                    awaiting_headers = false;
                } else

                // Process headers
                if (line.startsWith("HTTP/")) {
                    // HTTP/1.0 404 Not Found
                    auto space = line.indexOf(' ');
                    status = (int) line.substring(space, space + 4).toInt();
                } else

                if (line.startsWith("Content-Length: ")) {
                    content_length = (int) line.substring(strlen("Content-Length: ")).toInt();
                } else

                if (line.startsWith("X-")) {
                    auto colon = line.indexOf(':');
                    auto name = line.substring(0, colon);
                    auto value = line.substring(colon + 1);
                    name.trim();
                    value.trim();
                    x_hdr_cb(name, value);
                    continue;
                } else {
                    // Unknown header
                    //Serial.print("Ignored header: ");
                    //Serial.println(line);
                }
            } else {
                // Process body
                content_length -= int(line.length() + 1);
                line.trim();
                cnt_cb(line);
                if (content_length <= 0)
                    break;
            }
        }

        yield();
    }

    return status;
}


void HttpClient:: post(const char* url, const String& data)
{
    Serial.printf("* POST %s\n", url);
    m_client.printf(
            "POST %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Connection: close\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "\r\n",
            url, m_host.c_str(), m_port, data.length());
    m_client.print(data);

    m_display.appendText("OK");
    m_display.drawText(3, "Recv ");
    m_display.display();

    Serial.println("* Waiting for response...");

    while (m_client.connected() || m_client.available()) {
        if (m_client.available()) {
            String line = m_client.readStringUntil('\n');
            Serial.println(line);
        }

        yield();
    }
}


void HttpClient::stop()
{
    m_client.stop();
    Serial.println("* Connection closed");
}

