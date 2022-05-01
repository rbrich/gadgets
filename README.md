IoT Gadgets
===========

This is my single project for all IoT devices I develop.
I'm sharing it in hope to help others with their own IoT projects. 


Sensors
-------

Sensor devices measure environmental data like temperature or humidity
and send the values to centralized database.

I use InfluxDB + Grafana for collecting and presenting the sensor data.


How to build a device
---------------------

My development environment:

* [PlatformIO Core](https://platformio.org/install/cli)
* [crudini](https://www.pixelbeat.org/programs/crudini/)
* [CLion IDE](https://www.jetbrains.com/clion/)

Step by step:

1. Buy hardware. See `platformio.ini` for the hardware I use. Assemble the headers, sensors etc.

2. Select or add new env in `platformio.ini`, adjust `build_flags`.

3. Copy `include/config.h.templ` to `include/config.h` and fill the credentials for Wi-Fi etc.

4. Run `./pio_init` to apply the changes (and install optional libraries)

5. Connect the device and run `pio run --target upload` to program it.

6. Debug with `pio device monitor`.


TODO: Control Server
--------------------

Provides centralized control of all connected devices.
Each device has to actively query for new firmware and commands.

URLs:
- `/update` - ArduinoOTA firmware update (each device gets its own firmware)
- `/control` - Control commands for each device, headers contain "X-Seq" which indicates a change
- `/write` - sensor data, forward to InfluxDB

Implementation:
- Python + frontend web server (nginx)


TODO: Deployed devices
----------------------

Here I'll document the assembled devices, with links to the parts and also some photos.
