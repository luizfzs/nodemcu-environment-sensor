# NodeMCU Environment Sensor
NodeMCU code that collects data about temperature, humidity, heat index and posts to a MQTT topic

# Caveats
From the PubSubClient library, the result of `max_header_size + the topic name size + the payload size` cannot be greater that 128 (MQTT_MAX_PACKET_SIZE), otherwise it fails the publish with a `false` status.
