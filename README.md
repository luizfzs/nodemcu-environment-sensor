# NodeMCU Environment Sensor
NodeMCU code that collects data about temperature, humidity, heat index and posts to a MQTT topic.
This code uses NTP time, so the events can be even processed out-of-order and still reflect the data on a given instant. 

# Beware
This code uses deep sleep. Unless pin D0 is conneceted to RST pin, the application never wakes up. Also, when uploading the code to the board, *disconnect* pin D0 from RST or you'll not be able to upload it.

# Known bugs
* Sometimes the NTP client returns a very small number that clearly is not a timestamp (e.g. 3, 4, ...)

# To Do
* Externalize configuration to a file that can be added to `.gitignore` so I'm less prone to commit sensistive data.

# Caveats
From the PubSubClient library, the result of `max_header_size + the topic name size + the payload size` cannot be greater that 128 (MQTT_MAX_PACKET_SIZE), otherwise it fails the publish with a `false` status.

# Requirements
This code requires the following libraries
* https://github.com/knolleary/pubsubclient
* https://github.com/arduino-libraries/NTPClient
