Home Sensors
============

This library contains source codes for ours home sensor network.

Network:
 - 2x ESP8266
 - DHT22 measuring temperature and humidity
 - MQ135 measuring air quality
 - SHARP gp2y1010 measuring dust particles

Everything is connected to our home WiFi and is periodically uploading measurements.

TODO
----

 - Connect to WiFi only when uploading data.
 - Make data collection daemon
 - Send timestamp to synchronize ESP's time with real time.
 - Make simple GUI to display measurements.
